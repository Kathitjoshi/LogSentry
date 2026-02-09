#include "sstable.h"
#include <algorithm>
#include <cstring>

namespace kvstore {

SSTable::SSTable(const std::string& filename)
    : filename_(filename), file_size_(0), num_entries_(0),
      creation_time_(0), compression_enabled_(false) {
    
    file_.open(filename, std::ios::binary);
    if (file_.is_open()) {
        LoadIndex();
        LoadBloomFilter();
    }
}

SSTable::~SSTable() {
    if (file_.is_open()) {
        file_.close();
    }
}

bool SSTable::Get(const std::string& key, std::string& value) {
    if (!MayContain(key)) {
        return false;
    }
    
    SSTableEntry entry;
    if (BinarySearch(key, entry) && !entry.is_deleted) {
        value = entry.value;
        return true;
    }
    return false;
}

std::vector<SSTableEntry> SSTable::Scan(const std::string& start_key,
                                         const std::string& end_key,
                                         size_t limit) {
    std::vector<SSTableEntry> results;
    
    for (const auto& idx : index_) {
        if (idx.key < start_key) continue;
        if (idx.key > end_key) break;
        if (results.size() >= limit) break;
        
        file_.seekg(idx.offset);
        
        SSTableEntry entry;
        // Read entry (simplified)
        uint32_t key_len, value_len;
        file_.read(reinterpret_cast<char*>(&key_len), sizeof(key_len));
        
        entry.key.resize(key_len);
        file_.read(&entry.key[0], key_len);
        
        file_.read(reinterpret_cast<char*>(&value_len), sizeof(value_len));
        entry.value.resize(value_len);
        file_.read(&entry.value[0], value_len);
        
        file_.read(reinterpret_cast<char*>(&entry.is_deleted), sizeof(entry.is_deleted));
        file_.read(reinterpret_cast<char*>(&entry.timestamp), sizeof(entry.timestamp));
        
        results.push_back(entry);
    }
    
    return results;
}

bool SSTable::Create(const std::string& filename,
                     const std::vector<SSTableEntry>& entries,
                     bool use_compression,
                     bool use_bloom_filter) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        return false;
    }
    
    // Write header
    WriteHeader(out, entries.size(), use_compression, use_bloom_filter);
    
    // Build index and write data
    std::vector<SSTableIndex> index;
    uint64_t current_offset = 0;
    
    for (const auto& entry : entries) {
        SSTableIndex idx;
        idx.key = entry.key;
        idx.offset = current_offset;
        
        // Write entry
        uint32_t key_len = entry.key.size();
        uint32_t value_len = entry.value.size();
        
        out.write(reinterpret_cast<const char*>(&key_len), sizeof(key_len));
        out.write(entry.key.c_str(), key_len);
        out.write(reinterpret_cast<const char*>(&value_len), sizeof(value_len));
        out.write(entry.value.c_str(), value_len);
        out.write(reinterpret_cast<const char*>(&entry.is_deleted), sizeof(entry.is_deleted));
        out.write(reinterpret_cast<const char*>(&entry.timestamp), sizeof(entry.timestamp));
        
        idx.size = sizeof(key_len) + key_len + sizeof(value_len) + value_len +
                   sizeof(entry.is_deleted) + sizeof(entry.timestamp);
        
        current_offset += idx.size;
        index.push_back(idx);
    }
    
    // Write index
    WriteIndex(out, index);
    
    // Write bloom filter
    if (use_bloom_filter) {
        BloomFilter bf(entries.size());
        for (const auto& entry : entries) {
            bf.Add(entry.key);
        }
        auto bf_data = bf.Serialize();
        uint32_t bf_size = bf_data.size();
        out.write(reinterpret_cast<const char*>(&bf_size), sizeof(bf_size));
        out.write(reinterpret_cast<const char*>(bf_data.data()), bf_size);
    }
    
    out.close();
    return true;
}

bool SSTable::MayContain(const std::string& key) const {
    if (key < first_key_ || key > last_key_) {
        return false;
    }
    
    if (bloom_filter_) {
        return bloom_filter_->MayContain(key);
    }
    
    return true;
}

bool SSTable::LoadIndex() {
    // Simplified index loading
    return true;
}

bool SSTable::LoadBloomFilter() {
    // Simplified bloom filter loading
    return true;
}

bool SSTable::BinarySearch(const std::string& key, SSTableEntry& entry) {
    int left = 0, right = index_.size() - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (index_[mid].key == key) {
            file_.seekg(index_[mid].offset);
            // Read entry
            return true;
        }
        
        if (index_[mid].key < key) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return false;
}

bool SSTable::WriteHeader(std::ofstream& out, size_t num_entries,
                         bool compression, bool bloom_filter) {
    uint32_t magic = 0x53535401; // SST1
    out.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    out.write(reinterpret_cast<const char*>(&num_entries), sizeof(num_entries));
    
    uint8_t flags = 0;
    if (compression) flags |= 0x01;
    if (bloom_filter) flags |= 0x02;
    out.write(reinterpret_cast<const char*>(&flags), sizeof(flags));
    
    return true;
}

bool SSTable::WriteIndex(std::ofstream& out,
                        const std::vector<SSTableIndex>& index) {
    uint32_t index_size = index.size();
    out.write(reinterpret_cast<const char*>(&index_size), sizeof(index_size));
    
    for (const auto& idx : index) {
        uint32_t key_len = idx.key.size();
        out.write(reinterpret_cast<const char*>(&key_len), sizeof(key_len));
        out.write(idx.key.c_str(), key_len);
        out.write(reinterpret_cast<const char*>(&idx.offset), sizeof(idx.offset));
        out.write(reinterpret_cast<const char*>(&idx.size), sizeof(idx.size));
    }
    
    return true;
}

bool SSTable::WriteData(std::ofstream& out,
                       const std::vector<SSTableEntry>& entries,
                       bool compression) {
    // Already written in Create method
    return true;
}

} // namespace kvstore
