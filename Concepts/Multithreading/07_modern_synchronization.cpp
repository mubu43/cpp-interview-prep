#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

// C++20 features - conditional compilation based on availability
#if __cplusplus >= 202002L
#include <barrier>
#include <latch>
#include <semaphore>
#include <stop_token>
#endif

// ===== MODERN C++20 SYNCHRONIZATION PRIMITIVES =====

#if __cplusplus >= 202002L

// std::barrier demonstration (C++20)
void demonstrateBarrier() {
    std::cout << "=== std::barrier (C++20) ===\n\n";
    
    const int num_workers = 4;
    
    // Barrier with completion function
    std::barrier work_done(num_workers, []() noexcept {
        std::cout << "*** All workers completed phase - barrier completion function ***\n";
    });
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_workers; ++i) {
        threads.emplace_back([&work_done, i]() {
            // Phase 1 work
            std::cout << "Worker " << i << " starting phase 1\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100 + i * 30));
            std::cout << "Worker " << i << " completed phase 1\n";
            
            // Synchronization point - all threads wait here
            work_done.arrive_and_wait();
            
            // Phase 2 work - starts only after all threads complete phase 1
            std::cout << "Worker " << i << " starting phase 2\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(80 + i * 20));
            std::cout << "Worker " << i << " completed phase 2\n";
            
            // Another synchronization point
            work_done.arrive_and_wait();
            
            // Phase 3 work
            std::cout << "Worker " << i << " starting phase 3\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(60));
            std::cout << "Worker " << i << " all work completed\n";
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "All workers and phases completed\n\n";
}

// std::latch demonstration (C++20)
void demonstrateLatch() {
    std::cout << "=== std::latch (C++20) ===\n\n";
    
    const int num_workers = 5;
    
    // Latch for coordinating start
    std::latch start_latch(1);
    
    // Latch for coordinating completion
    std::latch completion_latch(num_workers);
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_workers; ++i) {
        threads.emplace_back([&start_latch, &completion_latch, i]() {
            std::cout << "Worker " << i << " ready and waiting for start signal\n";
            
            // Wait for start signal
            start_latch.wait();
            
            std::cout << "Worker " << i << " started working\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(150 + i * 25));
            std::cout << "Worker " << i << " finished work\n";
            
            // Signal completion
            completion_latch.count_down();
        });
    }
    
    // Let threads get ready
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "Main thread releasing all workers...\n";
    start_latch.count_down();  // Release all workers
    
    std::cout << "Main thread waiting for all workers to complete...\n";
    completion_latch.wait();  // Wait for all workers to complete
    
    std::cout << "All workers completed, main thread continuing\n";
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "\n";
}

// std::counting_semaphore demonstration (C++20)
void demonstrateSemaphore() {
    std::cout << "=== std::counting_semaphore (C++20) ===\n\n";
    
    // Semaphore allowing 2 concurrent resource access
    std::counting_semaphore<3> resource_semaphore(2);
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 6; ++i) {
        threads.emplace_back([&resource_semaphore, i]() {
            std::cout << "Thread " << i << " requesting resource access\n";
            
            // Acquire permit
            resource_semaphore.acquire();
            
            std::cout << "Thread " << i << " acquired resource (working...)\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            std::cout << "Thread " << i << " releasing resource\n";
            
            // Release permit
            resource_semaphore.release();
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "All threads completed resource usage\n\n";
}

// Binary semaphore (std::binary_semaphore) demonstration
void demonstrateBinarySemaphore() {
    std::cout << "=== std::binary_semaphore (C++20) ===\n\n";
    
    std::binary_semaphore signal_semaphore(0);  // Initially not available
    
    std::thread producer([&signal_semaphore]() {
        std::cout << "Producer preparing data...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        std::cout << "Producer data ready, signaling consumer\n";
        signal_semaphore.release();  // Signal that data is ready
    });
    
    std::thread consumer([&signal_semaphore]() {
        std::cout << "Consumer waiting for data signal...\n";
        signal_semaphore.acquire();  // Wait for signal
        
        std::cout << "Consumer received signal, processing data\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Consumer finished processing\n";
    });
    
    producer.join();
    consumer.join();
    
    std::cout << "\n";
}

// std::stop_token demonstration (C++20)
void demonstrateStopToken() {
    std::cout << "=== std::stop_token and std::jthread (C++20) ===\n\n";
    
    std::cout << "1. Basic jthread with automatic joining:\n";
    {
        std::jthread worker([](std::stop_token stoken) {
            int count = 0;
            while (!stoken.stop_requested()) {
                std::cout << "  Worker iteration " << ++count << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                if (count >= 5) break;  // Limit iterations for demo
            }
            std::cout << "  Worker thread exiting\n";
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
        std::cout << "  Requesting stop...\n";
        worker.request_stop();
        
        // jthread automatically joins in destructor
    }
    
    std::cout << "\n2. Stop token with stop callback:\n";
    {
        std::jthread interruptible_worker([](std::stop_token stoken) {
            // Register callback for stop request
            std::stop_callback callback(stoken, []() {
                std::cout << "    Stop callback executed - cleaning up\n";
            });
            
            int count = 0;
            while (!stoken.stop_requested()) {
                std::cout << "  Interruptible worker iteration " << ++count << "\n";
                
                // Use stop_token for interruptible sleep
                if (stoken.stop_requested()) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }
            
            std::cout << "  Interruptible worker exiting gracefully\n";
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
        std::cout << "  Requesting stop for interruptible worker...\n";
        interruptible_worker.request_stop();
    }
    
    std::cout << "\n3. Stop source for manual control:\n";
    std::stop_source stop_source;
    std::stop_token stop_token = stop_source.get_token();
    
    std::thread manual_worker([stop_token]() {
        while (!stop_token.stop_requested()) {
            std::cout << "  Manual worker still running\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << "  Manual worker stopped\n";
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::cout << "  Manually requesting stop...\n";
    stop_source.request_stop();
    
    manual_worker.join();
    std::cout << "\n";
}

// Coordination example using multiple C++20 features
void demonstrateCoordination() {
    std::cout << "=== Complex Coordination with C++20 Features ===\n\n";
    
    const int num_workers = 3;
    
    // Latch to wait for all workers to be ready
    std::latch ready_latch(num_workers);
    
    // Barrier for synchronized phases
    std::barrier phase_barrier(num_workers);
    
    // Semaphore for limited resource access
    std::counting_semaphore<2> resource_semaphore(1);  // Only 1 can access initially
    
    std::vector<std::jthread> workers;
    
    for (int i = 0; i < num_workers; ++i) {
        workers.emplace_back([&, i](std::stop_token stoken) {
            std::cout << "Worker " << i << " initializing\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50 + i * 25));
            
            // Signal ready
            std::cout << "Worker " << i << " ready\n";
            ready_latch.count_down();
            
            // Wait for all to be ready
            ready_latch.wait();
            
            // Phase 1: Synchronized start
            std::cout << "Worker " << i << " phase 1 starting\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Worker " << i << " phase 1 complete\n";
            
            phase_barrier.arrive_and_wait();
            
            // Phase 2: Resource contention
            if (!stoken.stop_requested()) {
                std::cout << "Worker " << i << " requesting resource\n";
                resource_semaphore.acquire();
                
                std::cout << "Worker " << i << " using resource\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                
                resource_semaphore.release();
                std::cout << "Worker " << i << " released resource\n";
            }
            
            phase_barrier.arrive_and_wait();
            
            std::cout << "Worker " << i << " all phases complete\n";
        });
    }
    
    // Let workers run
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Increase semaphore permits after phase 1
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Main: Increasing resource availability\n";
    resource_semaphore.release();  // Now 2 can access
    
    // Workers automatically join when jthread destructor is called
    std::cout << "Waiting for all workers to complete...\n";
    workers.clear();  // This will join all jthreads
    
    std::cout << "Complex coordination demonstration complete\n\n";
}

#else  // C++20 features not available

void demonstrateModernFeatures() {
    std::cout << "=== Modern C++20 Features (Not Available) ===\n\n";
    std::cout << "C++20 features (std::barrier, std::latch, std::semaphore, std::jthread)\n";
    std::cout << "are not available in this compiler version.\n";
    std::cout << "Please compile with C++20 support (-std=c++20) to see these features.\n\n";
    
    std::cout << "These features provide:\n";
    std::cout << "- std::barrier: Synchronization point for multiple threads\n";
    std::cout << "- std::latch: One-time synchronization mechanism\n";
    std::cout << "- std::semaphore: Resource counting and access control\n";
    std::cout << "- std::jthread: Auto-joining thread with stop tokens\n";
    std::cout << "- std::stop_token: Cooperative cancellation mechanism\n\n";
}

#endif

// Performance comparison of synchronization primitives
void benchmarkSynchronizationPrimitives() {
    std::cout << "=== Synchronization Primitives Performance Comparison ===\n\n";
    
    const int num_iterations = 100000;
    const int num_threads = 4;
    
    // Benchmark std::atomic
    std::atomic<int> atomic_counter{0};
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> atomic_threads;
    for (int i = 0; i < num_threads; ++i) {
        atomic_threads.emplace_back([&atomic_counter, num_iterations]() {
            for (int j = 0; j < num_iterations; ++j) {
                atomic_counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    for (auto& t : atomic_threads) {
        t.join();
    }
    
    auto atomic_time = std::chrono::high_resolution_clock::now() - start;
    
    // Benchmark std::mutex
    std::mutex mutex_lock;
    int mutex_counter = 0;
    start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> mutex_threads;
    for (int i = 0; i < num_threads; ++i) {
        mutex_threads.emplace_back([&mutex_lock, &mutex_counter, num_iterations]() {
            for (int j = 0; j < num_iterations; ++j) {
                std::lock_guard<std::mutex> lock(mutex_lock);
                ++mutex_counter;
            }
        });
    }
    
    for (auto& t : mutex_threads) {
        t.join();
    }
    
    auto mutex_time = std::chrono::high_resolution_clock::now() - start;
    
    auto atomic_ms = std::chrono::duration_cast<std::chrono::milliseconds>(atomic_time).count();
    auto mutex_ms = std::chrono::duration_cast<std::chrono::milliseconds>(mutex_time).count();
    
    std::cout << "Atomic operations time: " << atomic_ms << " ms (result: " << atomic_counter.load() << ")\n";
    std::cout << "Mutex operations time: " << mutex_ms << " ms (result: " << mutex_counter << ")\n";
    std::cout << "Atomic speedup: " << static_cast<double>(mutex_ms) / atomic_ms << "x\n\n";
}

int main() {
    try {
        std::cout << "=== MODERN C++20 SYNCHRONIZATION PRIMITIVES ===\n";
        std::cout << "This file covers new synchronization features in C++20\n\n";
        
#if __cplusplus >= 202002L
        std::cout << "C++20 features available - demonstrating new primitives:\n\n";
        
        demonstrateBarrier();
        demonstrateLatch();
        demonstrateSemaphore();
        demonstrateBinarySemaphore();
        demonstrateStopToken();
        demonstrateCoordination();
#else
        demonstrateModernFeatures();
#endif
        
        benchmarkSynchronizationPrimitives();
        
        std::cout << "=== KEY CONCEPTS COVERED ===\n";
        std::cout << "1. std::barrier for multi-phase synchronization\n";
        std::cout << "2. std::latch for one-time coordination events\n";
        std::cout << "3. std::counting_semaphore for resource management\n";
        std::cout << "4. std::binary_semaphore for signaling\n";
        std::cout << "5. std::jthread with automatic joining\n";
        std::cout << "6. std::stop_token for cooperative cancellation\n";
        std::cout << "7. Complex coordination patterns\n";
        std::cout << "8. Performance characteristics comparison\n\n";
        
        std::cout << "=== NEXT STEPS ===\n";
        std::cout << "-> Run 08_performance_analysis.cpp to learn about optimization\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

/*
Expected Output (with C++20):
=============================
=== MODERN C++20 SYNCHRONIZATION PRIMITIVES ===
This file covers new synchronization features in C++20

C++20 features available - demonstrating new primitives:

=== std::barrier (C++20) ===

Worker 0 starting phase 1
Worker 1 starting phase 1
Worker 2 starting phase 1
Worker 3 starting phase 1
Worker 0 completed phase 1
Worker 1 completed phase 1
Worker 2 completed phase 1
Worker 3 completed phase 1
*** All workers completed phase - barrier completion function ***
Worker 0 starting phase 2
[all workers start phase 2 simultaneously]
*** All workers completed phase - barrier completion function ***
Worker 0 starting phase 3
[synchronized phases continue]

=== std::latch (C++20) ===

Worker 0 ready and waiting for start signal
Worker 1 ready and waiting for start signal
[all workers wait for start signal]
Main thread releasing all workers...
Worker 0 started working
[all workers start simultaneously]
All workers completed, main thread continuing

=== std::counting_semaphore (C++20) ===

Thread 0 requesting resource access
Thread 1 requesting resource access
Thread 0 acquired resource (working...)
Thread 1 acquired resource (working...)
[only 2 threads can access resource simultaneously]
Thread 2 requesting resource access
[Thread 2 waits until resource available]

=== std::binary_semaphore (C++20) ===

Consumer waiting for data signal...
Producer preparing data...
Producer data ready, signaling consumer
Consumer received signal, processing data
Consumer finished processing

=== std::stop_token and std::jthread (C++20) ===

1. Basic jthread with automatic joining:
  Worker iteration 1
  Worker iteration 2
  Worker iteration 3
  Worker iteration 4
  Requesting stop...
  Worker thread exiting

2. Stop token with stop callback:
  Interruptible worker iteration 1
  Interruptible worker iteration 2
    Stop callback executed - cleaning up
  Interruptible worker exiting gracefully

=== Synchronization Primitives Performance Comparison ===

Atomic operations time: [fast_time] ms (result: 400000)
Mutex operations time: [slower_time] ms (result: 400000)
Atomic speedup: [speedup]x

=== KEY CONCEPTS COVERED ===
1. std::barrier for multi-phase synchronization
2. std::latch for one-time coordination events
3. std::counting_semaphore for resource management
4. std::binary_semaphore for signaling
5. std::jthread with automatic joining
6. std::stop_token for cooperative cancellation
7. Complex coordination patterns
8. Performance characteristics comparison

=== NEXT STEPS ===
-> Run 08_performance_analysis.cpp to learn about optimization

Compilation commands:
C++20: g++ -std=c++20 -Wall -Wextra -O2 -pthread 07_modern_synchronization.cpp -o 07_modern_synchronization
C++17: g++ -std=c++17 -Wall -Wextra -O2 -pthread 07_modern_synchronization.cpp -o 07_modern_synchronization

Key Learning Points:
===================
1. std::barrier enables multi-phase synchronized execution
2. std::latch provides one-time coordination mechanisms
3. std::counting_semaphore controls access to limited resources
4. std::jthread automatically joins, simplifying resource management
5. std::stop_token enables cooperative cancellation
6. Modern primitives often provide better performance and safety
7. Choose the right synchronization primitive for specific use cases
8. C++20 features make concurrent programming more expressive
*/
