#include "lru_cache.h"

namespace kvstore {

LRUCache::LRUCache(size_t capacity_bytes)
    : capacity_(capacity_bytes), current_size_(0), hits_(0), misses_(0) {}

bool LRUCache::Get(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cache_map_.find(key);
    if (it == cache_map_.end()) {
        ++misses_;
        return false;
    }
    
    ++hits_;
    
    // Move to front
    lru_list_.splice(lru_list_.begin(), lru_list_, it->second);
    value = it->second->value;
    return true;
}

void LRUCache::Put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t entry_size = EstimateSize(key, value);
    
    // Remove if exists
    auto it = cache_map_.find(key);
    if (it != cache_map_.end()) {
        current_size_ -= it->second->size;
        lru_list_.erase(it->second);
        cache_map_.erase(it);
    }
    
    // Evict if necessary
    while (current_size_ + entry_size > capacity_ && !lru_list_.empty()) {
        Evict();
    }
    
    // Add new entry
    CacheEntry entry{key, value, entry_size};
    lru_list_.push_front(entry);
    cache_map_[key] = lru_list_.begin();
    current_size_ += entry_size;
}

void LRUCache::Invalidate(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cache_map_.find(key);
    if (it != cache_map_.end()) {
        current_size_ -= it->second->size;
        lru_list_.erase(it->second);
        cache_map_.erase(it);
    }
}

void LRUCache::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    lru_list_.clear();
    cache_map_.clear();
    current_size_ = 0;
}

double LRUCache::HitRate() const {
    size_t total = hits_ + misses_;
    return total > 0 ? static_cast<double>(hits_) / total : 0.0;
}

void LRUCache::Evict() {
    if (lru_list_.empty()) return;
    
    auto& entry = lru_list_.back();
    current_size_ -= entry.size;
    cache_map_.erase(entry.key);
    lru_list_.pop_back();
}

size_t LRUCache::EstimateSize(const std::string& key, const std::string& value) const {
    return key.size() + value.size() + sizeof(CacheEntry);
}

} // namespace kvstore
