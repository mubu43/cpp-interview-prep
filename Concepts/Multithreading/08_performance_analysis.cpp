#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <atomic>
#include <queue>
#include <future>
#include <memory>
#include <algorithm>
#include <random>

// Performance measurement utilities
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::string name;
    
public:
    Timer(const std::string& operation_name) : name(operation_name) {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    ~Timer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << name << " took: " << duration.count() << " microseconds\n";
    }
};

// ===== CACHE LINE EFFECTS AND FALSE SHARING =====

struct PaddedCounter {
    alignas(64) std::atomic<int> counter{0};  // 64-byte aligned (typical cache line size)
    char padding[64 - sizeof(std::atomic<int>)];  // Prevent false sharing
};

struct UnpaddedCounter {
    std::atomic<int> counter{0};
};

void demonstrateFalseSharing() {
    std::cout << "=== FALSE SHARING DEMONSTRATION ===\n\n";
    
    const int num_threads = 4;
    const int iterations = 1000000;
    
    // Test with false sharing (adjacent counters)
    std::vector<UnpaddedCounter> unpadded_counters(num_threads);
    
    {
        Timer timer("Unpadded counters (false sharing)");
        std::vector<std::thread> threads;
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&unpadded_counters, i, iterations]() {
                for (int j = 0; j < iterations; ++j) {
                    unpadded_counters[i].counter.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    // Test without false sharing (padded counters)
    std::vector<PaddedCounter> padded_counters(num_threads);
    
    {
        Timer timer("Padded counters (no false sharing)");
        std::vector<std::thread> threads;
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&padded_counters, i, iterations]() {
                for (int j = 0; j < iterations; ++j) {
                    padded_counters[i].counter.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    std::cout << "Padded version should be significantly faster due to avoided false sharing\n\n";
}

// ===== MEMORY ACCESS PATTERNS =====

void demonstrateMemoryAccessPatterns() {
    std::cout << "=== MEMORY ACCESS PATTERNS ===\n\n";
    
    const size_t array_size = 10000000;
    const size_t num_threads = 4;
    
    std::vector<int> data(array_size);
    std::iota(data.begin(), data.end(), 1);  // Fill with 1, 2, 3, ...
    
    // Sequential access pattern
    {
        Timer timer("Sequential memory access");
        std::vector<std::thread> threads;
        
        size_t chunk_size = array_size / num_threads;
        
        for (size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back([&data, i, chunk_size]() {
                size_t start = i * chunk_size;
                size_t end = (i == 3) ? data.size() : (i + 1) * chunk_size;
                
                long long sum = 0;
                for (size_t j = start; j < end; ++j) {
                    sum += data[j];
                }
                
                // Prevent optimization
                volatile long long result = sum;
                (void)result;
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    // Random access pattern (cache-unfriendly)
    {
        Timer timer("Random memory access");
        std::vector<std::thread> threads;
        
        for (size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back([&data, i, array_size]() {
                std::random_device rd;
                std::mt19937 gen(rd() + i);  // Different seed per thread
                std::uniform_int_distribution<size_t> dist(0, array_size - 1);
                
                long long sum = 0;
                for (int j = 0; j < 100000; ++j) {
                    size_t index = dist(gen);
                    sum += data[index];
                }
                
                // Prevent optimization
                volatile long long result = sum;
                (void)result;
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    std::cout << "Sequential access should be much faster due to cache locality\n\n";
}

// ===== LOCK CONTENTION ANALYSIS =====

class ContentionDemo {
private:
    std::mutex hot_mutex;
    std::vector<std::mutex> distributed_mutexes;
    std::atomic<long long> shared_counter{0};
    std::vector<std::atomic<long long>> distributed_counters;
    
public:
    ContentionDemo(size_t num_distributed) : 
        distributed_mutexes(num_distributed),
        distributed_counters(num_distributed) {}
    
    void hotMutexTest(int iterations) {
        for (int i = 0; i < iterations; ++i) {
            std::lock_guard<std::mutex> lock(hot_mutex);
            shared_counter.fetch_add(1, std::memory_order_relaxed);
        }
    }
    
    void distributedMutexTest(int thread_id, int iterations) {
        size_t mutex_index = thread_id % distributed_mutexes.size();
        
        for (int i = 0; i < iterations; ++i) {
            std::lock_guard<std::mutex> lock(distributed_mutexes[mutex_index]);
            distributed_counters[mutex_index].fetch_add(1, std::memory_order_relaxed);
        }
    }
    
    long long getTotalDistributedCount() const {
        long long total = 0;
        for (const auto& counter : distributed_counters) {
            total += counter.load();
        }
        return total;
    }
};

void demonstrateLockContention() {
    std::cout << "=== LOCK CONTENTION ANALYSIS ===\n\n";
    
    const int num_threads = 8;
    const int iterations = 100000;
    
    ContentionDemo demo(num_threads);
    
    // High contention scenario
    {
        Timer timer("High contention (single mutex)");
        std::vector<std::thread> threads;
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&demo, iterations]() {
                demo.hotMutexTest(iterations);
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    // Distributed contention scenario
    {
        Timer timer("Distributed contention (multiple mutexes)");
        std::vector<std::thread> threads;
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&demo, i, iterations]() {
                demo.distributedMutexTest(i, iterations);
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    std::cout << "Distributed version should be faster due to reduced contention\n";
    std::cout << "Total distributed count: " << demo.getTotalDistributedCount() << "\n\n";
}

// ===== THREAD AFFINITY AND NUMA EFFECTS =====

void demonstrateThreadAffinity() {
    std::cout << "=== THREAD AFFINITY CONSIDERATIONS ===\n\n";
    
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << " threads\n";
    
    // Simulate work that benefits from thread affinity
    const int num_threads = std::min(4u, std::thread::hardware_concurrency());
    const int work_size = 1000000;
    
    std::vector<std::vector<int>> thread_local_data(num_threads);
    for (int i = 0; i < num_threads; ++i) {
        thread_local_data[i].resize(work_size);
        std::iota(thread_local_data[i].begin(), thread_local_data[i].end(), i * work_size);
    }
    
    {
        Timer timer("Thread-local data processing");
        std::vector<std::thread> threads;
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&thread_local_data, i]() {
                long long sum = 0;
                for (size_t j = 0; j < thread_local_data[i].size(); ++j) {
                    sum += thread_local_data[i][j] * thread_local_data[i][j];
                }
                
                // Prevent optimization
                volatile long long result = sum;
                std::cout << "Thread " << i << " processed " << thread_local_data[i].size() 
                         << " elements (result: " << result << ")\n";
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    std::cout << "\nNote: Thread affinity can be set using platform-specific APIs\n";
    std::cout << "Linux: pthread_setaffinity_np(), Windows: SetThreadAffinityMask()\n\n";
}

// ===== SCALABILITY ANALYSIS =====

class ScalabilityTester {
private:
    std::atomic<int> shared_counter{0};
    std::mutex shared_mutex;
    
public:
    void atomicIncrement(int iterations) {
        for (int i = 0; i < iterations; ++i) {
            shared_counter.fetch_add(1, std::memory_order_relaxed);
        }
    }
    
    void mutexIncrement(int iterations) {
        for (int i = 0; i < iterations; ++i) {
            std::lock_guard<std::mutex> lock(shared_mutex);
            int current = shared_counter.load();
            shared_counter.store(current + 1);
        }
    }
    
    void reset() {
        shared_counter.store(0);
    }
    
    int getCount() const {
        return shared_counter.load();
    }
};

void analyzeScalability() {
    std::cout << "=== SCALABILITY ANALYSIS ===\n\n";
    
    const int iterations_per_thread = 50000;
    const int max_threads = std::min(8u, std::thread::hardware_concurrency());
    
    ScalabilityTester tester;
    
    std::cout << "Testing scalability with increasing thread count:\n\n";
    
    // Test atomic operations scalability
    std::cout << "Atomic operations:\n";
    for (int num_threads = 1; num_threads <= max_threads; num_threads *= 2) {
        tester.reset();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&tester, iterations_per_thread]() {
                tester.atomicIncrement(iterations_per_thread);
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "  " << num_threads << " threads: " << duration.count() 
                  << " ms (count: " << tester.getCount() << ")\n";
    }
    
    std::cout << "\nMutex operations:\n";
    for (int num_threads = 1; num_threads <= max_threads; num_threads *= 2) {
        tester.reset();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&tester, iterations_per_thread]() {
                tester.mutexIncrement(iterations_per_thread);
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "  " << num_threads << " threads: " << duration.count() 
                  << " ms (count: " << tester.getCount() << ")\n";
    }
    
    std::cout << "\nNote: Atomic operations should scale better than mutex-based operations\n\n";
}

// ===== MEMORY ORDERING PERFORMANCE =====

void benchmarkMemoryOrdering() {
    std::cout << "=== MEMORY ORDERING PERFORMANCE ===\n\n";
    
    const int iterations = 1000000;
    const int num_threads = 4;
    
    // Test different memory orderings
    std::vector<std::string> orderings = {
        "relaxed", "acquire_release", "seq_cst"
    };
    
    for (const auto& ordering : orderings) {
        std::atomic<int> counter{0};
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&counter, iterations, &ordering]() {
                if (ordering == "relaxed") {
                    for (int j = 0; j < iterations; ++j) {
                        counter.fetch_add(1, std::memory_order_relaxed);
                    }
                } else if (ordering == "acquire_release") {
                    for (int j = 0; j < iterations; ++j) {
                        counter.fetch_add(1, std::memory_order_acq_rel);
                    }
                } else {  // seq_cst
                    for (int j = 0; j < iterations; ++j) {
                        counter.fetch_add(1, std::memory_order_seq_cst);
                    }
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << ordering << " ordering: " << duration.count() 
                  << " microseconds (result: " << counter.load() << ")\n";
    }
    
    std::cout << "\nRelaxed ordering should be fastest, seq_cst slowest\n\n";
}

// ===== WORK DISTRIBUTION STRATEGIES =====

void demonstrateWorkDistribution() {
    std::cout << "=== WORK DISTRIBUTION STRATEGIES ===\n\n";
    
    const size_t total_work = 10000000;
    const int num_threads = 4;
    
    std::vector<int> data(total_work);
    std::iota(data.begin(), data.end(), 1);
    
    // Static work distribution
    {
        Timer timer("Static work distribution");
        std::vector<std::thread> threads;
        
        size_t chunk_size = total_work / num_threads;
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&data, i, chunk_size, total_work]() {
                size_t start = i * chunk_size;
                size_t end = (i == 3) ? total_work : (i + 1) * chunk_size;
                
                long long sum = 0;
                for (size_t j = start; j < end; ++j) {
                    sum += data[j] % 17;  // Some work
                }
                
                volatile long long result = sum;
                (void)result;
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    // Dynamic work distribution with atomic counter
    {
        Timer timer("Dynamic work distribution");
        std::atomic<size_t> work_index{0};
        const size_t batch_size = 1000;
        
        std::vector<std::thread> threads;
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&data, &work_index, batch_size, total_work]() {
                long long sum = 0;
                
                while (true) {
                    size_t start = work_index.fetch_add(batch_size, std::memory_order_relaxed);
                    if (start >= total_work) break;
                    
                    size_t end = std::min(start + batch_size, total_work);
                    
                    for (size_t j = start; j < end; ++j) {
                        sum += data[j] % 17;  // Same work as static version
                    }
                }
                
                volatile long long result = sum;
                (void)result;
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    std::cout << "Dynamic distribution provides better load balancing\n\n";
}

// ===== PROFILING UTILITIES =====

void demonstrateProfilingTechniques() {
    std::cout << "=== PROFILING TECHNIQUES ===\n\n";
    
    std::cout << "Thread execution timing:\n";
    
    const int num_threads = 3;
    std::vector<std::chrono::microseconds> thread_times(num_threads);
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&thread_times, i]() {
            auto start = std::chrono::high_resolution_clock::now();
            
            // Simulate different amounts of work
            std::this_thread::sleep_for(std::chrono::milliseconds(100 + i * 50));
            
            // Some computational work
            long long sum = 0;
            for (int j = 0; j < 100000 * (i + 1); ++j) {
                sum += j * j;
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            thread_times[i] = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            volatile long long result = sum;  // Prevent optimization
            (void)result;
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    for (int i = 0; i < num_threads; ++i) {
        std::cout << "Thread " << i << " execution time: " 
                  << thread_times[i].count() << " microseconds\n";
    }
    
    auto max_time = *std::max_element(thread_times.begin(), thread_times.end());
    auto min_time = *std::min_element(thread_times.begin(), thread_times.end());
    
    std::cout << "\nWork imbalance ratio: " 
              << static_cast<double>(max_time.count()) / min_time.count() << ":1\n";
    
    std::cout << "\nProfiling tools and techniques:\n";
    std::cout << "- Intel VTune Profiler for detailed performance analysis\n";
    std::cout << "- Linux perf for system-level profiling\n";
    std::cout << "- ThreadSanitizer for race condition detection\n";
    std::cout << "- Helgrind (Valgrind) for thread error detection\n";
    std::cout << "- Custom timing with std::chrono for micro-benchmarks\n\n";
}

int main() {
    try {
        std::cout << "=== THREADING PERFORMANCE ANALYSIS ===\n";
        std::cout << "This file analyzes performance characteristics of threading\n\n";
        
        demonstrateFalseSharing();
        demonstrateMemoryAccessPatterns();
        demonstrateLockContention();
        demonstrateThreadAffinity();
        analyzeScalability();
        benchmarkMemoryOrdering();
        demonstrateWorkDistribution();
        demonstrateProfilingTechniques();
        
        std::cout << "=== KEY PERFORMANCE INSIGHTS ===\n";
        std::cout << "1. False sharing can significantly impact performance\n";
        std::cout << "2. Memory access patterns affect cache efficiency\n";
        std::cout << "3. Lock contention limits scalability\n";
        std::cout << "4. Thread affinity and NUMA effects matter\n";
        std::cout << "5. Memory ordering has performance implications\n";
        std::cout << "6. Work distribution strategy affects load balancing\n";
        std::cout << "7. Profiling tools are essential for optimization\n\n";
        
        std::cout << "=== OPTIMIZATION GUIDELINES ===\n";
        std::cout << "- Minimize shared mutable state\n";
        std::cout << "- Use cache-line alignment for hot data\n";
        std::cout << "- Prefer lock-free algorithms when appropriate\n";
        std::cout << "- Choose the weakest memory ordering that's correct\n";
        std::cout << "- Design for good cache locality\n";
        std::cout << "- Balance work distribution dynamically\n";
        std::cout << "- Profile early and often\n\n";
        
        std::cout << "=== COMPLETION ===\n";
        std::cout << "You have completed the comprehensive C++ threading series!\n";
        std::cout << "Review the concepts as needed for your interviews.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

/*
Expected Output:
================
=== THREADING PERFORMANCE ANALYSIS ===
This file analyzes performance characteristics of threading

=== FALSE SHARING DEMONSTRATION ===

Unpadded counters (false sharing) took: [longer_time] microseconds
Padded counters (no false sharing) took: [shorter_time] microseconds
Padded version should be significantly faster due to avoided false sharing

=== MEMORY ACCESS PATTERNS ===

Sequential memory access took: [shorter_time] microseconds
Random memory access took: [longer_time] microseconds
Sequential access should be much faster due to cache locality

=== LOCK CONTENTION ANALYSIS ===

High contention (single mutex) took: [longer_time] microseconds
Distributed contention (multiple mutexes) took: [shorter_time] microseconds
Distributed version should be faster due to reduced contention
Total distributed count: 800000

=== THREAD AFFINITY CONSIDERATIONS ===

Hardware concurrency: [N] threads
Thread-local data processing took: [time] microseconds
Thread 0 processed 1000000 elements (result: [large_number])
Thread 1 processed 1000000 elements (result: [large_number])
[additional threads...]

Note: Thread affinity can be set using platform-specific APIs
Linux: pthread_setaffinity_np(), Windows: SetThreadAffinityMask()

=== SCALABILITY ANALYSIS ===

Testing scalability with increasing thread count:

Atomic operations:
  1 threads: [time1] ms (count: 50000)
  2 threads: [time2] ms (count: 100000)
  4 threads: [time3] ms (count: 200000)
  8 threads: [time4] ms (count: 400000)

Mutex operations:
  1 threads: [time1] ms (count: 50000)
  2 threads: [time2] ms (count: 100000)
  4 threads: [time3] ms (count: 200000)
  8 threads: [time4] ms (count: 400000)

Note: Atomic operations should scale better than mutex-based operations

=== MEMORY ORDERING PERFORMANCE ===

relaxed ordering: [fastest_time] microseconds (result: 4000000)
acquire_release ordering: [medium_time] microseconds (result: 4000000)
seq_cst ordering: [slowest_time] microseconds (result: 4000000)

Relaxed ordering should be fastest, seq_cst slowest

=== WORK DISTRIBUTION STRATEGIES ===

Static work distribution took: [time1] microseconds
Dynamic work distribution took: [time2] microseconds
Dynamic distribution provides better load balancing

=== PROFILING TECHNIQUES ===

Thread execution timing:
Thread 0 execution time: [time1] microseconds
Thread 1 execution time: [time2] microseconds
Thread 2 execution time: [time3] microseconds

Work imbalance ratio: [ratio]:1

Profiling tools and techniques:
- Intel VTune Profiler for detailed performance analysis
- Linux perf for system-level profiling
- ThreadSanitizer for race condition detection
- Helgrind (Valgrind) for thread error detection
- Custom timing with std::chrono for micro-benchmarks

=== KEY PERFORMANCE INSIGHTS ===
1. False sharing can significantly impact performance
2. Memory access patterns affect cache efficiency
3. Lock contention limits scalability
4. Thread affinity and NUMA effects matter
5. Memory ordering has performance implications
6. Work distribution strategy affects load balancing
7. Profiling tools are essential for optimization

=== OPTIMIZATION GUIDELINES ===
- Minimize shared mutable state
- Use cache-line alignment for hot data
- Prefer lock-free algorithms when appropriate
- Choose the weakest memory ordering that's correct
- Design for good cache locality
- Balance work distribution dynamically
- Profile early and often

=== COMPLETION ===
You have completed the comprehensive C++ threading series!
Review the concepts as needed for your interviews.

Compilation:
g++ -std=c++17 -Wall -Wextra -O2 -pthread 08_performance_analysis.cpp -o 08_performance_analysis

Key Learning Points:
===================
1. False sharing occurs when threads modify different variables on same cache line
2. Sequential memory access is much faster than random access due to cache locality
3. Lock contention increases with thread count - distribute locks when possible
4. Thread affinity and NUMA topology affect performance on multi-CPU systems
5. Memory ordering strength affects performance - use weakest correct ordering
6. Static work distribution can lead to load imbalance
7. Dynamic work distribution provides better load balancing
8. Profiling is essential for identifying performance bottlenecks
9. Cache-line alignment prevents false sharing
10. Understanding hardware characteristics is crucial for optimization

Performance Optimization Strategies:
===================================
1. Minimize shared state between threads
2. Use thread-local storage when possible
3. Align frequently accessed data to cache line boundaries
4. Prefer atomic operations over locks for simple operations
5. Use lock-free algorithms for high-performance scenarios
6. Consider work-stealing for dynamic load balancing
7. Profile code to identify actual bottlenecks
8. Choose appropriate synchronization primitives for use case
9. Design algorithms with cache efficiency in mind
10. Test scalability with realistic workloads
*/
