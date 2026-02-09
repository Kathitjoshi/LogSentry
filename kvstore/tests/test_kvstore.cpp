#include <gtest/gtest.h>
#include "kvstore.h"

using namespace kvstore;

TEST(KVStoreTest, PutAndGet) {
    Config config;
    config.data_dir = "/tmp/kvstore_test";
    KVStore store(config);
    
    ASSERT_TRUE(store.Put("key1", "value1"));
    
    std::string value;
    ASSERT_TRUE(store.Get("key1", value));
    ASSERT_EQ(value, "value1");
}

TEST(KVStoreTest, DeleteKey) {
    Config config;
    config.data_dir = "/tmp/kvstore_test";
    KVStore store(config);
    
    store.Put("key2", "value2");
    ASSERT_TRUE(store.Delete("key2"));
    
    std::string value;
    ASSERT_FALSE(store.Get("key2", value));
}

TEST(KVStoreTest, Scan) {
    Config config;
    config.data_dir = "/tmp/kvstore_test";
    KVStore store(config);
    
    store.Put("key_a", "value_a");
    store.Put("key_b", "value_b");
    store.Put("key_c", "value_c");
    
    auto results = store.Scan("key_a", "key_c", 10);
    ASSERT_GE(results.size(), 2);
}
