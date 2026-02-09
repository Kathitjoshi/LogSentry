#ifndef LSM_ENGINE_H
#define LSM_ENGINE_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include "memtable.h"
#include "wal.h"
#include "sstable.h"

namespace kvstore {

/**
 * Main LSM-tree based storage engine
 * Coordinates memtable, WAL, SSTables, and compaction
 */
class LSMEngine {
private:
    std::string db_path_;
    std::unique_ptr<MemTable> active_memtable_;
    std::unique_ptr<MemTable> immutable_memtable_;
    std::unique_ptr<WAL> wal_;
    std::vector<std::unique_ptr<SSTable>> sstables_;
    
    mutable std::mutex mu_;
    std::atomic<bool> running_;
    std::thread compaction_thread_;
    
    uint32_t next_sstable_id_;
    static constexpr size_t MAX_LEVEL = 7;
    static constexpr size_t LEVEL_SIZE_MULTIPLIER = 10;

    void flush_memtable();
    void compact_background();
    void compact_level(uint32_t level);
    std::string generate_sstable_filename();
    void load_existing_sstables();
    void recover_from_wal();

public:
    /**
     * Constructor
     * @param db_path Directory path for database files
     */
    explicit LSMEngine(const std::string& db_path);
    ~LSMEngine();

    // Disable copy
    LSMEngine(const LSMEngine&) = delete;
    LSMEngine& operator=(const LSMEngine&) = delete;

    /**
     * Put a key-value pair
     * @param key The key
     * @param value The value
     */
    void put(const std::string& key, const std::string& value);

    /**
     * Get a value by key
     * @param key The key to search for
     * @param value Output parameter for the value
     * @return true if found, false otherwise
     */
    bool get(const std::string& key, std::string& value);

    /**
     * Delete a key (implemented as a tombstone)
     * @param key The key to delete
     */
    void remove(const std::string& key);

    /**
     * Manually trigger compaction
     */
    void compact();

    /**
     * Get statistics about the storage engine
     */
    struct Stats {
        size_t memtable_size;
        size_t num_sstables;
        size_t total_disk_usage;
        size_t num_keys_approx;
    };
    
    Stats get_stats() const;

    /**
     * Close the engine and flush all data
     */
    void close();
};

} // namespace kvstore

#endif // LSM_ENGINE_H
