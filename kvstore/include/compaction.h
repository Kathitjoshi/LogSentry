#ifndef COMPACTION_H
#define COMPACTION_H

#include <vector>
#include <string>
#include <memory>
#include "sstable.h"

namespace kvstore {

class Compaction {
public:
    struct Strategy {
        size_t level;
        size_t max_files_per_level;
        size_t size_ratio;
    };
    
    static bool CompactSSTables(
        const std::vector<std::string>& input_files,
        const std::string& output_file,
        bool compression = true
    );
    
    static std::vector<std::string> SelectFilesForCompaction(
        const std::vector<std::unique_ptr<SSTable>>& sstables,
        size_t threshold
    );
    
private:
    struct MergeEntry {
        SSTableEntry entry;
        size_t source_index;
        
        bool operator>(const MergeEntry& other) const {
            if (entry.key != other.entry.key) {
                return entry.key > other.entry.key;
            }
            return entry.timestamp < other.entry.timestamp;
        }
    };
};

} // namespace kvstore

#endif // COMPACTION_H
