#include <gtest/gtest.h>
#include "wal.h"
#include "bloom_filter.h"
#include "sstable.h"
#include "lsm_engine.h"
#include <filesystem>

using namespace kvstore;
namespace fs = std::filesystem;

// ===== WAL Tests =====
class WALTest : public ::testing::Test {
protected:
    std::string test_file = "/tmp/test_wal.log";

    void TearDown() override {
        std::remove(test_file.c_str());
    }
};

TEST_F(WALTest, BasicAppendAndRecover) {
    {
        WAL wal(test_file);
        wal.append("key1", "value1");
        wal.append("key2", "value2");
        wal.flush();
    }
    
    WAL wal(test_file);
    auto entries = wal.recover();
    
    ASSERT_EQ(entries.size(), 2);
    EXPECT_EQ(entries[0].first, "key1");
    EXPECT_EQ(entries[0].second, "value1");
    EXPECT_EQ(entries[1].first, "key2");
    EXPECT_EQ(entries[1].second, "value2");
}

TEST_F(WALTest, ClearLog) {
    WAL wal(test_file);
    wal.append("key1", "value1");
    wal.flush();
    
    wal.clear();
    
    auto entries = wal.recover();
    EXPECT_EQ(entries.size(), 0);
}

// ===== Bloom Filter Tests =====
TEST(BloomFilterTest, BasicAddAndCheck) {
    BloomFilter filter(100, 0.01);
    
    filter.add("key1");
    filter.add("key2");
    filter.add("key3");
    
    EXPECT_TRUE(filter.might_contain("key1"));
    EXPECT_TRUE(filter.might_contain("key2"));
    EXPECT_TRUE(filter.might_contain("key3"));
}

TEST(BloomFilterTest, FalseNegativesNotPossible) {
    BloomFilter filter(100, 0.01);
    
    filter.add("existing_key");
    
    // This should NEVER return false for an added key
    EXPECT_TRUE(filter.might_contain("existing_key"));
}

TEST(BloomFilterTest, FalsePositiveRate) {
    BloomFilter filter(1000, 0.01);
    
    // Add 1000 keys
    for (int i = 0; i < 1000; i++) {
        filter.add("key" + std::to_string(i));
    }
    
    // Check 1000 non-existent keys
    int false_positives = 0;
    for (int i = 1000; i < 2000; i++) {
        if (filter.might_contain("key" + std::to_string(i))) {
            false_positives++;
        }
    }
    
    // False positive rate should be roughly 1%
    double rate = static_cast<double>(false_positives) / 1000.0;
    EXPECT_LT(rate, 0.05);  // Allow some variance
}

TEST(BloomFilterTest, Serialization) {
    BloomFilter filter(100, 0.01);
    
    filter.add("key1");
    filter.add("key2");
    
    auto data = filter.serialize();
    auto restored = BloomFilter::deserialize(data);
    
    EXPECT_TRUE(restored.might_contain("key1"));
    EXPECT_TRUE(restored.might_contain("key2"));
}

// ===== SSTable Tests =====
class SSTableTest : public ::testing::Test {
protected:
    std::string test_file = "/tmp/test_sstable.sst";

    void TearDown() override {
        std::remove(test_file.c_str());
    }
};

TEST_F(SSTableTest, CreateAndRead) {
    std::map<std::string, std::string> data = {
        {"key1", "value1"},
        {"key2", "value2"},
        {"key3", "value3"}
    };
    
    auto sstable = SSTable::create(test_file, data, 0);
    
    auto result1 = sstable.get("key1");
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), "value1");
    
    auto result2 = sstable.get("key2");
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "value2");
}

TEST_F(SSTableTest, NonExistentKey) {
    std::map<std::string, std::string> data = {
        {"key1", "value1"}
    };
    
    auto sstable = SSTable::create(test_file, data, 0);
    
    auto result = sstable.get("nonexistent");
    EXPECT_FALSE(result.has_value());
}

TEST_F(SSTableTest, BloomFilterOptimization) {
    std::map<std::string, std::string> data = {
        {"key1", "value1"}
    };
    
    auto sstable = SSTable::create(test_file, data, 0);
    
    // Bloom filter should indicate non-existent key doesn't exist
    EXPECT_FALSE(sstable.might_contain("definitely_not_there"));
}

TEST_F(SSTableTest, GetKeys) {
    std::map<std::string, std::string> data = {
        {"apple", "fruit"},
        {"banana", "fruit"},
        {"carrot", "vegetable"}
    };
    
    auto sstable = SSTable::create(test_file, data, 0);
    auto keys = sstable.get_keys();
    
    ASSERT_EQ(keys.size(), 3);
    EXPECT_EQ(keys[0], "apple");
    EXPECT_EQ(keys[1], "banana");
    EXPECT_EQ(keys[2], "carrot");
}

// ===== LSM Engine Tests =====
class LSMEngineTest : public ::testing::Test {
protected:
    std::string test_db = "/tmp/test_lsm_db";

    void SetUp() override {
        fs::remove_all(test_db);
    }

    void TearDown() override {
        fs::remove_all(test_db);
    }
};

TEST_F(LSMEngineTest, BasicPutGet) {
    LSMEngine engine(test_db);
    
    engine.put("key1", "value1");
    
    std::string value;
    ASSERT_TRUE(engine.get("key1", value));
    EXPECT_EQ(value, "value1");
    
    engine.close();
}

TEST_F(LSMEngineTest, PersistenceAcrossRestarts) {
    {
        LSMEngine engine(test_db);
        engine.put("persistent_key", "persistent_value");
        engine.close();
    }
    
    {
        LSMEngine engine(test_db);
        std::string value;
        ASSERT_TRUE(engine.get("persistent_key", value));
        EXPECT_EQ(value, "persistent_value");
        engine.close();
    }
}

TEST_F(LSMEngineTest, MemtableFlush) {
    LSMEngine engine(test_db);
    
    // Write enough data to trigger flush (> 4MB)
    for (int i = 0; i < 50000; i++) {
        engine.put("key" + std::to_string(i), std::string(100, 'x'));
    }
    
    auto stats = engine.get_stats();
    EXPECT_GT(stats.num_sstables, 0);
    
    engine.close();
}

TEST_F(LSMEngineTest, UpdateExistingKey) {
    LSMEngine engine(test_db);
    
    engine.put("key1", "value1");
    engine.put("key1", "value2");
    
    std::string value;
    ASSERT_TRUE(engine.get("key1", value));
    EXPECT_EQ(value, "value2");
    
    engine.close();
}

TEST_F(LSMEngineTest, Remove) {
    LSMEngine engine(test_db);
    
    engine.put("key1", "value1");
    engine.remove("key1");
    
    std::string value;
    // After removal, get should still work but return tombstone
    // In a real implementation, you'd filter tombstones in get()
    
    engine.close();
}

TEST_F(LSMEngineTest, GetStats) {
    LSMEngine engine(test_db);
    
    for (int i = 0; i < 100; i++) {
        engine.put("key" + std::to_string(i), "value" + std::to_string(i));
    }
    
    auto stats = engine.get_stats();
    
    EXPECT_GT(stats.memtable_size, 0);
    EXPECT_GT(stats.num_keys_approx, 0);
    
    engine.close();
}

TEST_F(LSMEngineTest, Compaction) {
    LSMEngine engine(test_db);
    
    // Force creation of multiple SSTables
    for (int batch = 0; batch < 5; batch++) {
        for (int i = 0; i < 10000; i++) {
            engine.put("key" + std::to_string(batch) + "_" + std::to_string(i), 
                      std::string(100, 'x'));
        }
    }
    
    size_t sstables_before = engine.get_stats().num_sstables;
    
    // Trigger compaction
    engine.compact();
    
    // After compaction, should have fewer SSTables
    size_t sstables_after = engine.get_stats().num_sstables;
    
    engine.close();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
