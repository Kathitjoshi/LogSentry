#include "wal.h"
#include <cstring>

namespace kvstore {

WAL::WAL(const std::string& filename)
    : filename_(filename), current_size_(0) {
    file_.open(filename, std::ios::app | std::ios::binary);
}

WAL::~WAL() {
    if (file_.is_open()) {
        Sync();
        file_.close();
    }
}

bool WAL::Append(WALRecordType type, const std::string& key,
                const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    WALRecord record;
    record.type = type;
    record.key = key;
    record.value = value;
    record.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    record.checksum = ComputeChecksum(record);
    
    return WriteRecord(record);
}

bool WAL::Sync() {
    if (file_.is_open()) {
        file_.flush();
        return true;
    }
    return false;
}

std::vector<WALRecord> WAL::ReadAll() {
    std::vector<WALRecord> records;
    std::ifstream in(filename_, std::ios::binary);
    
    if (!in.is_open()) {
        return records;
    }
    
    WALRecord record;
    while (ReadRecord(in, record)) {
        if (VerifyChecksum(record)) {
            records.push_back(record);
        }
    }
    
    return records;
}

void WAL::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (file_.is_open()) {
        file_.close();
    }
    
    file_.open(filename_, std::ios::trunc | std::ios::binary);
    current_size_ = 0;
}

uint32_t WAL::ComputeChecksum(const WALRecord& record) const {
    uint32_t checksum = 0;
    for (char c : record.key) checksum += c;
    for (char c : record.value) checksum += c;
    checksum += static_cast<uint32_t>(record.type);
    checksum += record.timestamp;
    return checksum;
}

bool WAL::VerifyChecksum(const WALRecord& record) const {
    return record.checksum == ComputeChecksum(record);
}

bool WAL::WriteRecord(const WALRecord& record) {
    uint8_t type = static_cast<uint8_t>(record.type);
    file_.write(reinterpret_cast<const char*>(&type), sizeof(type));
    
    uint32_t key_len = record.key.size();
    file_.write(reinterpret_cast<const char*>(&key_len), sizeof(key_len));
    file_.write(record.key.c_str(), key_len);
    
    uint32_t value_len = record.value.size();
    file_.write(reinterpret_cast<const char*>(&value_len), sizeof(value_len));
    file_.write(record.value.c_str(), value_len);
    
    file_.write(reinterpret_cast<const char*>(&record.timestamp), 
                sizeof(record.timestamp));
    file_.write(reinterpret_cast<const char*>(&record.checksum), 
                sizeof(record.checksum));
    
    current_size_ += sizeof(type) + sizeof(key_len) + key_len +
                     sizeof(value_len) + value_len + sizeof(record.timestamp) +
                     sizeof(record.checksum);
    
    return file_.good();
}

bool WAL::ReadRecord(std::ifstream& in, WALRecord& record) {
    uint8_t type;
    if (!in.read(reinterpret_cast<char*>(&type), sizeof(type))) {
        return false;
    }
    record.type = static_cast<WALRecordType>(type);
    
    uint32_t key_len;
    in.read(reinterpret_cast<char*>(&key_len), sizeof(key_len));
    record.key.resize(key_len);
    in.read(&record.key[0], key_len);
    
    uint32_t value_len;
    in.read(reinterpret_cast<char*>(&value_len), sizeof(value_len));
    record.value.resize(value_len);
    in.read(&record.value[0], value_len);
    
    in.read(reinterpret_cast<char*>(&record.timestamp), sizeof(record.timestamp));
    in.read(reinterpret_cast<char*>(&record.checksum), sizeof(record.checksum));
    
    return in.good();
}

} // namespace kvstore
