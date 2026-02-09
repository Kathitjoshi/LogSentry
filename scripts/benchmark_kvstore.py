#!/usr/bin/env python3
"""
Benchmark script for KV store performance
"""
import time
import random
import string
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

try:
    import kvstore
except ImportError:
    print("Error: kvstore module not found. Please build the C++ extension first.")
    sys.exit(1)


def generate_random_string(length=20):
    """Generate random string"""
    return ''.join(random.choices(string.ascii_letters + string.digits, k=length))


def benchmark_writes(engine, num_operations=10000):
    """Benchmark write performance"""
    print(f"\nBenchmarking {num_operations} writes...")
    
    start_time = time.time()
    
    for i in range(num_operations):
        key = f"key_{i:010d}"
        value = generate_random_string(100)
        engine.put(key, value)
    
    elapsed = time.time() - start_time
    ops_per_sec = num_operations / elapsed
    
    print(f"  Time: {elapsed:.2f}s")
    print(f"  Throughput: {ops_per_sec:.2f} writes/sec")
    print(f"  Latency: {(elapsed / num_operations) * 1000:.3f} ms/write")
    
    return ops_per_sec


def benchmark_reads(engine, num_operations=10000):
    """Benchmark read performance"""
    print(f"\nBenchmarking {num_operations} reads...")
    
    # First, write some data
    keys = []
    for i in range(num_operations):
        key = f"read_key_{i:010d}"
        value = generate_random_string(100)
        engine.put(key, value)
        keys.append(key)
    
    # Now benchmark reads
    start_time = time.time()
    hits = 0
    
    for key in keys:
        result = engine.get(key)
        if result is not None:
            hits += 1
    
    elapsed = time.time() - start_time
    ops_per_sec = num_operations / elapsed
    
    print(f"  Time: {elapsed:.2f}s")
    print(f"  Throughput: {ops_per_sec:.2f} reads/sec")
    print(f"  Latency: {(elapsed / num_operations) * 1000:.3f} ms/read")
    print(f"  Hit rate: {(hits / num_operations) * 100:.1f}%")
    
    return ops_per_sec


def benchmark_mixed(engine, num_operations=10000, read_ratio=0.8):
    """Benchmark mixed read/write workload"""
    print(f"\nBenchmarking {num_operations} mixed operations (read ratio: {read_ratio})...")
    
    # Pre-populate some keys
    keys = []
    for i in range(1000):
        key = f"mixed_key_{i:010d}"
        value = generate_random_string(100)
        engine.put(key, value)
        keys.append(key)
    
    start_time = time.time()
    reads = 0
    writes = 0
    
    for i in range(num_operations):
        if random.random() < read_ratio:
            # Read
            key = random.choice(keys)
            engine.get(key)
            reads += 1
        else:
            # Write
            key = f"mixed_key_{random.randint(0, 2000):010d}"
            value = generate_random_string(100)
            engine.put(key, value)
            if key not in keys:
                keys.append(key)
            writes += 1
    
    elapsed = time.time() - start_time
    ops_per_sec = num_operations / elapsed
    
    print(f"  Time: {elapsed:.2f}s")
    print(f"  Total ops: {num_operations} ({reads} reads, {writes} writes)")
    print(f"  Throughput: {ops_per_sec:.2f} ops/sec")
    print(f"  Latency: {(elapsed / num_operations) * 1000:.3f} ms/op")
    
    return ops_per_sec


def main():
    """Run benchmarks"""
    import argparse
    
    parser = argparse.ArgumentParser(description='Benchmark KV Store')
    parser.add_argument('--db-path', default='/tmp/benchmark_db',
                       help='Database path')
    parser.add_argument('--num-ops', type=int, default=10000,
                       help='Number of operations per test')
    
    args = parser.parse_args()
    
    # Clean up old database
    import shutil
    if os.path.exists(args.db_path):
        shutil.rmtree(args.db_path)
    
    print("=" * 60)
    print("KV Store Performance Benchmark")
    print("=" * 60)
    
    # Initialize engine
    print(f"\nInitializing KV store at {args.db_path}...")
    engine = kvstore.LSMEngine(args.db_path)
    
    # Run benchmarks
    write_throughput = benchmark_writes(engine, args.num_ops)
    read_throughput = benchmark_reads(engine, args.num_ops)
    mixed_throughput = benchmark_mixed(engine, args.num_ops)
    
    # Get statistics
    stats = engine.get_stats()
    
    print("\n" + "=" * 60)
    print("Summary")
    print("=" * 60)
    print(f"Write throughput: {write_throughput:.2f} ops/sec")
    print(f"Read throughput: {read_throughput:.2f} ops/sec")
    print(f"Mixed throughput: {mixed_throughput:.2f} ops/sec")
    print(f"\nStorage Statistics:")
    print(f"  Total keys: {stats.num_keys_approx}")
    print(f"  Disk usage: {stats.total_disk_usage / (1024*1024):.2f} MB")
    print(f"  Memtable size: {stats.memtable_size / 1024:.2f} KB")
    print(f"  Number of SSTables: {stats.num_sstables}")
    
    # Cleanup
    engine.close()
    
    print("\nBenchmark complete!")


if __name__ == '__main__':
    main()
