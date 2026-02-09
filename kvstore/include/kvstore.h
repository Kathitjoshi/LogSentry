#ifndef KVSTORE_H
#define KVSTORE_H

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <map>
#include "memtable.h"
#include "sstable.h"
#include "wal.h"
#include "bloom_filter.h"
#include "lru_cache.h"

namespace kvstore {

struct Config {
    std::string data_dir = "./data";
    size_t memtable_size_mb = 64;
    size_t compaction_threshold = 4;
    size_t cache_size_mb = 128;
    bool enable_compression = true;
    bool enable_bloom_filter = true;
};

class KVStore {
public:
    explicit KVStore(const Config& config);
    ~KVStore();

    // Basic operations
    bool Put(const std::string& key, const std::string& value);
    bool Get(const std::string& key, std::string& value);
    bool Delete(const std::string& key);
    
    // Batch operations
    bool PutBatch(const std::vector<std::pair<std::string, std::string>>& entries);
    
    // Range scan
    std::vector<std::pair<std::string, std::string>> Scan(
        const std::string& start_key,
        const std::string& end_key,
        size_t limit = 1000
    );
    
    // Statistics
    struct Stats {
        size_t total_keys;
        size_t total_size_bytes;
        size_t memtable_size;
        size_t num_sstables;
        size_t cache_hits;
        size_t cache_misses;
    };
    Stats GetStats() const;
    
    // Maintenance
    void Compact();
    void Flush();
    
private:
    Config config_;
    std::unique_ptr<MemTable> memtable_;
    std::unique_ptr<MemTable> immutable_memtable_;
    std::vector<std::unique_ptr<SSTable>> sstables_;
    std::unique_ptr<WAL> wal_;
    std::unique_ptr<LRUCache> cache_;
    
    mutable std::mutex mutex_;
    mutable std::mutex compaction_mutex_;
    
    bool should_flush_;
    size_t next_sstable_id_;
    
    // Private methods
    void FlushMemTable();
    void LoadSSTables();
    void MaybeCompact();
    void RecoverFromWAL();
    std::string GetSSTablePath(size_t id) const;
};

} // namespace kvstore

#endif // KVSTORE_H
