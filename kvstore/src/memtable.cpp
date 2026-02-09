#include "memtable.h"

namespace kvstore {

void MemTable::Put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = table_.find(key);
    if (it != table_.end()) {
        size_bytes_ -= EstimateSize(key, it->second);
    }
    
    Entry entry(value);
    size_bytes_ += EstimateSize(key, entry);
    table_[key] = entry;
}

bool MemTable::Get(const std::string& key, std::string& value) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = table_.find(key);
    if (it != table_.end() && !it->second.is_deleted) {
        value = it->second.value;
        return true;
    }
    return false;
}

void MemTable::Delete(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = table_.find(key);
    if (it != table_.end()) {
        size_bytes_ -= EstimateSize(key, it->second);
    }
    
    Entry entry("", true);
    size_bytes_ += EstimateSize(key, entry);
    table_[key] = entry;
}

void MemTable::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    table_.clear();
    size_bytes_ = 0;
}

size_t MemTable::EstimateSize(const std::string& key, const Entry& entry) const {
    return key.size() + entry.value.size() + sizeof(Entry);
}

} // namespace kvstore
