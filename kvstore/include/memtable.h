#ifndef MEMTABLE_H
#define MEMTABLE_H

#include <map>
#include <string>
#include <mutex>
#include <chrono>

namespace kvstore {

struct Entry {
    std::string value;
    bool is_deleted;
    std::chrono::system_clock::time_point timestamp;
    
    Entry() : is_deleted(false), timestamp(std::chrono::system_clock::now()) {}
    Entry(const std::string& v) : value(v), is_deleted(false), 
        timestamp(std::chrono::system_clock::now()) {}
    Entry(const std::string& v, bool deleted) : value(v), is_deleted(deleted),
        timestamp(std::chrono::system_clock::now()) {}
};

class MemTable {
public:
    MemTable() : size_bytes_(0) {}
    
    void Put(const std::string& key, const std::string& value);
    bool Get(const std::string& key, std::string& value) const;
    void Delete(const std::string& key);
    
    size_t Size() const { return table_.size(); }
    size_t SizeBytes() const { return size_bytes_; }
    bool IsEmpty() const { return table_.empty(); }
    
    // Iterator support for flushing to SSTable
    using Iterator = std::map<std::string, Entry>::const_iterator;
    Iterator Begin() const { return table_.begin(); }
    Iterator End() const { return table_.end(); }
    
    void Clear();
    
private:
    std::map<std::string, Entry> table_;
    size_t size_bytes_;
    mutable std::mutex mutex_;
    
    size_t EstimateSize(const std::string& key, const Entry& entry) const;
};

} // namespace kvstore

#endif // MEMTABLE_H
