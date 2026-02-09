#include "kvstore.h"
#include "compaction.h"
#include <filesystem>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

namespace kvstore {

KVStore::KVStore(const Config& config)
    : config_(config),
      should_flush_(false),
      next_sstable_id_(0) {
    
    // Create data directory
    fs::create_directories(config_.data_dir);
    
    // Initialize WAL
    std::string wal_path = config_.data_dir + "/wal.log";
    wal_ = std::make_unique<WAL>(wal_path);
    
    // Initialize memtable
    memtable_ = std::make_unique<MemTable>();
    
    // Initialize cache
    size_t cache_size = config_.cache_size_mb * 1024 * 1024;
    cache_ = std::make_unique<LRUCache>(cache_size);
    
    // Load existing SSTables
    LoadSSTables();
    
    // Recover from WAL if needed
    RecoverFromWAL();
}

KVStore::~KVStore() {
    Flush();
}

bool KVStore::Put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Write to WAL first
    if (!wal_->Append(WALRecordType::PUT, key, value)) {
        return false;
    }
    
    // Write to memtable
    memtable_->Put(key, value);
    
    // Invalidate cache
    cache_->Invalidate(key);
    
    // Check if we need to flush
    size_t memtable_size = memtable_->SizeBytes();
    size_t threshold = config_.memtable_size_mb * 1024 * 1024;
    
    if (memtable_size >= threshold) {
        should_flush_ = true;
        FlushMemTable();
    }
    
    return true;
}

bool KVStore::Get(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check cache first
    if (cache_->Get(key, value)) {
        return true;
    }
    
    // Check memtable
    if (memtable_->Get(key, value)) {
        cache_->Put(key, value);
        return true;
    }
    
    // Check immutable memtable
    if (immutable_memtable_ && immutable_memtable_->Get(key, value)) {
        cache_->Put(key, value);
        return true;
    }
    
    // Check SSTables (newest to oldest)
    for (auto it = sstables_.rbegin(); it != sstables_.rend(); ++it) {
        if ((*it)->MayContain(key) && (*it)->Get(key, value)) {
            cache_->Put(key, value);
            return true;
        }
    }
    
    return false;
}

bool KVStore::Delete(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Write tombstone to WAL
    if (!wal_->Append(WALRecordType::DELETE, key)) {
        return false;
    }
    
    // Write tombstone to memtable
    memtable_->Delete(key);
    
    // Invalidate cache
    cache_->Invalidate(key);
    
    return true;
}

bool KVStore::PutBatch(const std::vector<std::pair<std::string, std::string>>& entries) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& [key, value] : entries) {
        if (!wal_->Append(WALRecordType::PUT, key, value)) {
            return false;
        }
        memtable_->Put(key, value);
        cache_->Invalidate(key);
    }
    
    // Check if flush needed
    size_t memtable_size = memtable_->SizeBytes();
    size_t threshold = config_.memtable_size_mb * 1024 * 1024;
    
    if (memtable_size >= threshold) {
        FlushMemTable();
    }
    
    return true;
}

std::vector<std::pair<std::string, std::string>> KVStore::Scan(
    const std::string& start_key,
    const std::string& end_key,
    size_t limit) {
    
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::pair<std::string, std::string>> results;
    
    // Merge from all sources
    std::map<std::string, std::string> merged;
    
    // Get from SSTables
    for (auto& sstable : sstables_) {
        auto entries = sstable->Scan(start_key, end_key, limit);
        for (const auto& entry : entries) {
            if (!entry.is_deleted) {
                merged[entry.key] = entry.value;
            }
        }
    }
    
    // Override with memtable data (more recent)
    for (auto it = memtable_->Begin(); it != memtable_->End(); ++it) {
        if (it->first >= start_key && it->first <= end_key) {
            if (!it->second.is_deleted) {
                merged[it->first] = it->second.value;
            } else {
                merged.erase(it->first);
            }
        }
    }
    
    // Convert to vector
    for (const auto& [key, value] : merged) {
        if (results.size() >= limit) break;
        results.emplace_back(key, value);
    }
    
    return results;
}

KVStore::Stats KVStore::GetStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Stats stats;
    stats.memtable_size = memtable_->SizeBytes();
    stats.num_sstables = sstables_.size();
    stats.cache_hits = cache_->HitCount();
    stats.cache_misses = cache_->MissCount();
    
    // Count total keys and size
    stats.total_keys = memtable_->Size();
    stats.total_size_bytes = memtable_->SizeBytes();
    
    for (const auto& sstable : sstables_) {
        stats.total_keys += sstable->GetNumEntries();
        stats.total_size_bytes += sstable->GetSize();
    }
    
    return stats;
}

void KVStore::Compact() {
    MaybeCompact();
}

void KVStore::Flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!memtable_->IsEmpty()) {
        FlushMemTable();
    }
}

void KVStore::FlushMemTable() {
    if (memtable_->IsEmpty()) {
        return;
    }
    
    // Create new memtable
    immutable_memtable_ = std::move(memtable_);
    memtable_ = std::make_unique<MemTable>();
    
    // Clear WAL
    wal_->Clear();
    
    // Convert immutable memtable to SSTable
    std::vector<SSTableEntry> entries;
    for (auto it = immutable_memtable_->Begin(); 
         it != immutable_memtable_->End(); ++it) {
        SSTableEntry entry;
        entry.key = it->first;
        entry.value = it->second.value;
        entry.is_deleted = it->second.is_deleted;
        entry.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            it->second.timestamp.time_since_epoch()).count();
        entries.push_back(entry);
    }
    
    // Write SSTable
    std::string filename = GetSSTablePath(next_sstable_id_++);
    if (SSTable::Create(filename, entries, 
                       config_.enable_compression,
                       config_.enable_bloom_filter)) {
        sstables_.push_back(std::make_unique<SSTable>(filename));
    }
    
    // Clear immutable memtable
    immutable_memtable_.reset();
    
    // Maybe trigger compaction
    MaybeCompact();
}

void KVStore::LoadSSTables() {
    if (!fs::exists(config_.data_dir)) {
        return;
    }
    
    std::vector<std::string> sstable_files;
    for (const auto& entry : fs::directory_iterator(config_.data_dir)) {
        if (entry.path().extension() == ".sst") {
            sstable_files.push_back(entry.path().string());
        }
    }
    
    std::sort(sstable_files.begin(), sstable_files.end());
    
    for (const auto& file : sstable_files) {
        sstables_.push_back(std::make_unique<SSTable>(file));
        
        // Update next ID
        std::string filename = fs::path(file).filename().string();
        size_t id = std::stoull(filename.substr(0, filename.find('.')));
        next_sstable_id_ = std::max(next_sstable_id_, id + 1);
    }
}

void KVStore::MaybeCompact() {
    std::lock_guard<std::mutex> lock(compaction_mutex_);
    
    if (sstables_.size() < config_.compaction_threshold) {
        return;
    }
    
    auto files_to_compact = Compaction::SelectFilesForCompaction(
        sstables_, config_.compaction_threshold);
    
    if (!files_to_compact.empty()) {
        std::string output_file = GetSSTablePath(next_sstable_id_++);
        
        if (Compaction::CompactSSTables(files_to_compact, output_file, 
                                        config_.enable_compression)) {
            // Remove old SSTables
            sstables_.erase(
                std::remove_if(sstables_.begin(), sstables_.end(),
                    [&](const std::unique_ptr<SSTable>& sst) {
                        for (const auto& file : files_to_compact) {
                            // Compare filenames
                            return true; // Simplified
                        }
                        return false;
                    }),
                sstables_.end()
            );
            
            // Add new SSTable
            sstables_.push_back(std::make_unique<SSTable>(output_file));
            
            // Delete old files
            for (const auto& file : files_to_compact) {
                fs::remove(file);
            }
        }
    }
}

void KVStore::RecoverFromWAL() {
    auto records = wal_->ReadAll();
    for (const auto& record : records) {
        if (record.type == WALRecordType::PUT) {
            memtable_->Put(record.key, record.value);
        } else if (record.type == WALRecordType::DELETE) {
            memtable_->Delete(record.key);
        }
    }
}

std::string KVStore::GetSSTablePath(size_t id) const {
    return config_.data_dir + "/" + std::to_string(id) + ".sst";
}

} // namespace kvstore
