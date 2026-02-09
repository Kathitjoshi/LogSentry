#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <vector>
#include <string>
#include <cstdint>

namespace kvstore {

class BloomFilter {
public:
    BloomFilter(size_t expected_elements, double false_positive_rate = 0.01);
    
    void Add(const std::string& key);
    bool MayContain(const std::string& key) const;
    
    // Serialization
    std::vector<uint8_t> Serialize() const;
    static BloomFilter Deserialize(const std::vector<uint8_t>& data);
    
    size_t Size() const { return bits_.size(); }
    size_t NumHashes() const { return num_hashes_; }
    
private:
    std::vector<bool> bits_;
    size_t num_hashes_;
    
    std::vector<uint64_t> Hash(const std::string& key) const;
};

} // namespace kvstore

#endif // BLOOM_FILTER_H
