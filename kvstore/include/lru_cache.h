#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <string>
#include <unordered_map>
#include <list>
#include <mutex>

namespace kvstore {

class LRUCache {
public:
    explicit LRUCache(size_t capacity_bytes);
    
    bool Get(const std::string& key, std::string& value);
    void Put(const std::string& key, const std::string& value);
    void Invalidate(const std::string& key);
    void Clear();
    
    size_t Size() const { return current_size_; }
    size_t Capacity() const { return capacity_; }
    size_t HitCount() const { return hits_; }
    size_t MissCount() const { return misses_; }
    double HitRate() const;
    
private:
    struct CacheEntry {
        std::string key;
        std::string value;
        size_t size;
    };
    
    size_t capacity_;
    size_t current_size_;
    size_t hits_;
    size_t misses_;
    
    std::list<CacheEntry> lru_list_;
    std::unordered_map<std::string, std::list<CacheEntry>::iterator> cache_map_;
    
    mutable std::mutex mutex_;
    
    void Evict();
    size_t EstimateSize(const std::string& key, const std::string& value) const;
};

} // namespace kvstore

#endif // LRU_CACHE_H
