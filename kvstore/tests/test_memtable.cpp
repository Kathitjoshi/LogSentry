#include <gtest/gtest.h>
#include "memtable.h"

using namespace kvstore;

TEST(MemTableTest, PutAndGet) {
    MemTable table;
    table.Put("test", "value");
    
    std::string value;
    ASSERT_TRUE(table.Get("test", value));
    ASSERT_EQ(value, "value");
}

TEST(MemTableTest, Delete) {
    MemTable table;
    table.Put("test", "value");
    table.Delete("test");
    
    std::string value;
    ASSERT_FALSE(table.Get("test", value));
}
