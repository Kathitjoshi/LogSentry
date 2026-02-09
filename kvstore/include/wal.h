#ifndef WAL_H
#define WAL_H

#include <string>
#include <fstream>
#include <vector>
#include <mutex>

namespace kvstore {

enum class WALRecordType {
    PUT = 1,
    DELETE = 2
};

struct WALRecord {
    WALRecordType type;
    std::string key;
    std::string value;
    uint64_t timestamp;
    uint32_t checksum;
};

class WAL {
public:
    explicit WAL(const std::string& filename);
    ~WAL();
    
    bool Append(WALRecordType type, const std::string& key, 
                const std::string& value = "");
    bool Sync();
    
    // Recovery
    std::vector<WALRecord> ReadAll();
    void Clear();
    
    size_t Size() const { return current_size_; }
    
private:
    std::string filename_;
    std::ofstream file_;
    size_t current_size_;
    std::mutex mutex_;
    
    uint32_t ComputeChecksum(const WALRecord& record) const;
    bool VerifyChecksum(const WALRecord& record) const;
    
    bool WriteRecord(const WALRecord& record);
    bool ReadRecord(std::ifstream& in, WALRecord& record);
};

} // namespace kvstore

#endif // WAL_H
