#include "lsm_engine.h"
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

namespace kvstore {

LSMEngine::LSMEngine(const std::string& db_path) 
    : db_path_(db_path), next_sstable_id_(0), running_(true) {
    
    // Create database directory if it doesn't exist
    fs::create_directories(db_path_);
    
    // Initialize WAL
    std::string wal_path = db_path_ + "/wal.log";
    wal_ = std::make_unique<WAL>(wal_path);
    
    // Initialize active memtable
    active_memtable_ = std::make_unique<MemTable>();
    
    // Recover from WAL if exists
    recover_from_wal();
    
    // Load existing SSTables
    load_existing_sstables();
    
    // Start background compaction thread
    compaction_thread_ = std::thread(&LSMEngine::compact_background, this);
}

LSMEngine::~LSMEngine() {
    close();
}

void LSMEngine::put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mu_);
    
    // Write to WAL first for durability
    wal_->append(key, value);
    
    // Write to active memtable
    active_memtable_->put(key, value);
    
    // Check if memtable should be flushed
    if (active_memtable_->should_flush()) {
        flush_memtable();
    }
}

bool LSMEngine::get(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(mu_);
    
    // Check active memtable first
    if (active_memtable_->get(key, value)) {
        return true;
    }
    
    // Check immutable memtable if exists
    if (immutable_memtable_ && immutable_memtable_->get(key, value)) {
        return true;
    }
    
    // Check SSTables from newest to oldest
    for (auto it = sstables_.rbegin(); it != sstables_.rend(); ++it) {
        if ((*it)->might_contain(key)) {
            auto result = (*it)->get(key);
            if (result.has_value()) {
                value = result.value();
                return true;
            }
        }
    }
    
    return false;
}

void LSMEngine::remove(const std::string& key) {
    // Deletion is implemented as a special tombstone value
    put(key, "__TOMBSTONE__");
}

void LSMEngine::flush_memtable() {
    // Make current memtable immutable
    immutable_memtable_ = std::move(active_memtable_);
    active_memtable_ = std::make_unique<MemTable>();
    
    // Get all entries from immutable memtable
    auto entries = immutable_memtable_->get_all_sorted();
    
    // Create new SSTable
    std::string filename = generate_sstable_filename();
    auto sstable = SSTable::create(filename, entries, 0);
    sstables_.push_back(std::make_unique<SSTable>(filename));
    
    // Clear immutable memtable and WAL
    immutable_memtable_.reset();
    wal_->clear();
    
    std::cout << "Flushed memtable to SSTable: " << filename << std::endl;
}

void LSMEngine::compact_background() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(30));
        
        if (!running_) break;
        
        try {
            compact();
        } catch (const std::exception& e) {
            std::cerr << "Compaction error: " << e.what() << std::endl;
        }
    }
}

void LSMEngine::compact() {
    std::lock_guard<std::mutex> lock(mu_);
    
    // Simple compaction: merge all level 0 SSTables if there are more than 4
    std::vector<SSTable*> level0_tables;
    for (const auto& table : sstables_) {
        if (table->level() == 0) {
            level0_tables.push_back(table.get());
        }
    }
    
    if (level0_tables.size() <= 4) {
        return;  // No need to compact yet
    }
    
    std::cout << "Starting compaction of " << level0_tables.size() << " level 0 SSTables" << std::endl;
    
    // Merge all level 0 tables
    std::map<std::string, std::string> merged_data;
    for (auto* table : level0_tables) {
        for (const auto& key : table->get_keys()) {
            auto value = table->get(key);
            if (value.has_value()) {
                // Skip tombstones
                if (value.value() != "__TOMBSTONE__") {
                    merged_data[key] = value.value();
                }
            }
        }
    }
    
    // Create new level 1 SSTable
    std::string filename = generate_sstable_filename();
    auto new_table = SSTable::create(filename, merged_data, 1);
    
    // Remove old SSTables
    sstables_.erase(
        std::remove_if(sstables_.begin(), sstables_.end(),
            [&level0_tables](const std::unique_ptr<SSTable>& table) {
                for (auto* old_table : level0_tables) {
                    if (table.get() == old_table) {
                        table->remove();
                        return true;
                    }
                }
                return false;
            }),
        sstables_.end()
    );
    
    // Add new SSTable
    sstables_.push_back(std::make_unique<SSTable>(filename));
    
    std::cout << "Compaction complete. Created: " << filename << std::endl;
}

std::string LSMEngine::generate_sstable_filename() {
    return db_path_ + "/sstable_" + std::to_string(next_sstable_id_++) + ".sst";
}

void LSMEngine::load_existing_sstables() {
    if (!fs::exists(db_path_)) {
        return;
    }
    
    std::vector<std::string> sstable_files;
    for (const auto& entry : fs::directory_iterator(db_path_)) {
        if (entry.path().extension() == ".sst") {
            sstable_files.push_back(entry.path().string());
        }
    }
    
    // Sort by filename (which includes ID)
    std::sort(sstable_files.begin(), sstable_files.end());
    
    for (const auto& filename : sstable_files) {
        try {
            sstables_.push_back(std::make_unique<SSTable>(filename));
            
            // Update next_sstable_id_
            std::string basename = fs::path(filename).stem().string();
            size_t pos = basename.find('_');
            if (pos != std::string::npos) {
                uint32_t id = std::stoul(basename.substr(pos + 1));
                if (id >= next_sstable_id_) {
                    next_sstable_id_ = id + 1;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to load SSTable " << filename << ": " << e.what() << std::endl;
        }
    }
    
    std::cout << "Loaded " << sstables_.size() << " existing SSTables" << std::endl;
}

void LSMEngine::recover_from_wal() {
    auto entries = wal_->recover();
    
    if (!entries.empty()) {
        std::cout << "Recovering " << entries.size() << " entries from WAL" << std::endl;
        
        for (const auto& [key, value] : entries) {
            active_memtable_->put(key, value);
        }
    }
}

LSMEngine::Stats LSMEngine::get_stats() const {
    std::lock_guard<std::mutex> lock(mu_);
    
    Stats stats;
    stats.memtable_size = active_memtable_->size_bytes();
    if (immutable_memtable_) {
        stats.memtable_size += immutable_memtable_->size_bytes();
    }
    
    stats.num_sstables = sstables_.size();
    stats.total_disk_usage = 0;
    stats.num_keys_approx = active_memtable_->entry_count();
    
    for (const auto& table : sstables_) {
        stats.total_disk_usage += table->size();
        stats.num_keys_approx += table->get_keys().size();
    }
    
    return stats;
}

void LSMEngine::close() {
    running_ = false;
    
    if (compaction_thread_.joinable()) {
        compaction_thread_.join();
    }
    
    std::lock_guard<std::mutex> lock(mu_);
    
    // Flush any remaining data
    if (active_memtable_->entry_count() > 0) {
        flush_memtable();
    }
    
    wal_->flush();
}

} // namespace kvstore
