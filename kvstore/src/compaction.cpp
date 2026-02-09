#include "compaction.h"
#include <queue>
#include <fstream>

namespace kvstore {

bool Compaction::CompactSSTables(
    const std::vector<std::string>& input_files,
    const std::string& output_file,
    bool compression) {
    
    // Priority queue for merging
    std::priority_queue<MergeEntry, std::vector<MergeEntry>, 
                       std::greater<MergeEntry>> pq;
    
    // Open all input SSTables
    std::vector<std::unique_ptr<SSTable>> tables;
    for (const auto& file : input_files) {
        tables.push_back(std::make_unique<SSTable>(file));
    }
    
    // Merge all entries
    std::map<std::string, SSTableEntry> merged;
    
    for (size_t i = 0; i < tables.size(); ++i) {
        auto entries = tables[i]->Scan("", "~", SIZE_MAX);
        for (const auto& entry : entries) {
            auto it = merged.find(entry.key);
            if (it == merged.end() || entry.timestamp > it->second.timestamp) {
                merged[entry.key] = entry;
            }
        }
    }
    
    // Write merged entries
    std::vector<SSTableEntry> output_entries;
    for (const auto& [key, entry] : merged) {
        if (!entry.is_deleted) {
            output_entries.push_back(entry);
        }
    }
    
    return SSTable::Create(output_file, output_entries, compression, true);
}

std::vector<std::string> Compaction::SelectFilesForCompaction(
    const std::vector<std::unique_ptr<SSTable>>& sstables,
    size_t threshold) {
    
    std::vector<std::string> files;
    
    if (sstables.size() < threshold) {
        return files;
    }
    
    // Select oldest files for compaction
    size_t count = std::min(threshold, sstables.size());
    for (size_t i = 0; i < count; ++i) {
        // files.push_back(sstables[i]->GetFilename());
    }
    
    return files;
}

} // namespace kvstore
