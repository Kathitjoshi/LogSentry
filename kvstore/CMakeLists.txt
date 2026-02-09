cmake_minimum_required(VERSION 3.15)
project(LogKVStore VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(KVSTORE_SOURCES
    src/memtable.cpp
    src/sstable.cpp
    src/wal.cpp
    src/compaction.cpp
    src/kvstore.cpp
    src/bloom_filter.cpp
    src/lru_cache.cpp
)

# Library
add_library(kvstore SHARED ${KVSTORE_SOURCES})
target_link_libraries(kvstore PRIVATE Threads::Threads)

# Server executable
add_executable(kvstore_server src/server.cpp)
target_link_libraries(kvstore_server PRIVATE kvstore Threads::Threads)

# Client library
add_library(kvstore_client SHARED src/client.cpp)
target_link_libraries(kvstore_client PRIVATE Threads::Threads)

# Installation
install(TARGETS kvstore kvstore_server kvstore_client
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include/kvstore)

# Testing
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    
    # Download and build Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    
    # Test executables
    add_executable(kvstore_test
        tests/test_memtable.cpp
        tests/test_sstable.cpp
        tests/test_wal.cpp
        tests/test_kvstore.cpp
        tests/test_bloom_filter.cpp
    )
    
    target_link_libraries(kvstore_test
        PRIVATE
        kvstore
        gtest_main
        gmock_main
    )
    
    include(GoogleTest)
    gtest_discover_tests(kvstore_test)
endif()
