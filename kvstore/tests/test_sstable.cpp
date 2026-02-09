#include <gtest/gtest.h>
#include "sstable.h"

using namespace kvstore;

TEST(SSTableTest, CreateAndRead) {
    std::vector<SSTableEntry> entries;
    SSTableEntry entry;
    entry.key = "test_key";
    entry.value = "test_value";
    entry.is_deleted = false;
    entry.timestamp = 12345;
    entries.push_back(entry);
    
    ASSERT_TRUE(SSTable::Create("/tmp/test.sst", entries));
    
    SSTable table("/tmp/test.sst");
    std::string value;
    ASSERT_TRUE(table.Get("test_key", value));
    ASSERT_EQ(value, "test_value");
}
