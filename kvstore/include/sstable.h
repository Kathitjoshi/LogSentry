#ifndef SSTABLE_H
#define SSTABLE_H

#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include "bloom_filter.h"

namespace kvstore {

struct SSTableEntry {
    std::string key;
    std::string value;
    bool is_deleted;
    uint64_t timestamp;
};

struct SSTableIndex {
    std::string key;
    uint64_t offset;
    uint32_t size;
};

class SSTable {
public:
    explicit SSTable(const std::string& filename);
    ~SSTable();
    
    // Read operations
    bool Get(const std::string& key, std::string& value);
    std::vector<SSTableEntry> Scan(const std::string& start_key, 
                                     const std::string& end_key,
                                     size_t limit = 1000);
    
    // Write operations (create new SSTable)
    static bool Create(const std::string& filename,
                      const std::vector<SSTableEntry>& entries,
                      bool use_compression = true,
                      bool use_bloom_filter = true);
    
    // Metadata
    std::string GetFirstKey() const { return first_key_; }
    std::string GetLastKey() const { return last_key_; }
    size_t GetSize() const { return file_size_; }
    size_t GetNumEntries() const { return num_entries_; }
    uint64_t GetCreationTime() const { return creation_time_; }
    
    // Check if key might exist (using bloom filter)
    bool MayContain(const std::string& key) const;
    
private:
    std::string filename_;
    std::ifstream file_;
    std::vector<SSTableIndex> index_;
    std::unique_ptr<BloomFilter> bloom_filter_;
    
    std::string first_key_;
    std::string last_key_;
    size_t file_size_;
    size_t num_entries_;
    uint64_t creation_time_;
    bool compression_enabled_;
    
    bool LoadIndex();
    bool LoadBloomFilter();
    bool BinarySearch(const std::string& key, SSTableEntry& entry);
    
    // Serialization helpers
    static bool WriteHeader(std::ofstream& out, size_t num_entries,
                           bool compression, bool bloom_filter);
    static bool WriteIndex(std::ofstream& out, 
                          const std::vector<SSTableIndex>& index);
    static bool WriteData(std::ofstream& out,
                         const std::vector<SSTableEntry>& entries,
                         bool compression);
};

} // namespace kvstore

#endif // SSTABLE_H
