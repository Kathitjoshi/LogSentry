#include "bloom_filter.h"
#include <cmath>
#include <functional>

namespace kvstore {

BloomFilter::BloomFilter(size_t expected_elements, double false_positive_rate) {
    size_t m = -1 * expected_elements * std::log(false_positive_rate) / 
               (std::log(2) * std::log(2));
    num_hashes_ = std::ceil((m / expected_elements) * std::log(2));
    bits_.resize(m, false);
}

void BloomFilter::Add(const std::string& key) {
    auto hashes = Hash(key);
    for (auto h : hashes) {
        bits_[h % bits_.size()] = true;
    }
}

bool BloomFilter::MayContain(const std::string& key) const {
    auto hashes = Hash(key);
    for (auto h : hashes) {
        if (!bits_[h % bits_.size()]) {
            return false;
        }
    }
    return true;
}

std::vector<uint8_t> BloomFilter::Serialize() const {
    std::vector<uint8_t> data;
    size_t size = bits_.size();
    data.resize(sizeof(size) + sizeof(num_hashes_) + (size + 7) / 8);
    
    size_t offset = 0;
    std::memcpy(&data[offset], &size, sizeof(size));
    offset += sizeof(size);
    std::memcpy(&data[offset], &num_hashes_, sizeof(num_hashes_));
    offset += sizeof(num_hashes_);
    
    for (size_t i = 0; i < bits_.size(); ++i) {
        if (bits_[i]) {
            data[offset + i / 8] |= (1 << (i % 8));
        }
    }
    
    return data;
}

BloomFilter BloomFilter::Deserialize(const std::vector<uint8_t>& data) {
    BloomFilter bf(1); // Placeholder
    // Deserialization logic
    return bf;
}

std::vector<uint64_t> BloomFilter::Hash(const std::string& key) const {
    std::vector<uint64_t> hashes;
    std::hash<std::string> hasher;
    
    uint64_t h1 = hasher(key);
    uint64_t h2 = hasher(key + "salt");
    
    for (size_t i = 0; i < num_hashes_; ++i) {
        hashes.push_back(h1 + i * h2);
    }
    
    return hashes;
}

} // namespace kvstore
