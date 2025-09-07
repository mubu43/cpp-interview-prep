#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <string>

// ===== BASIC THREAD CREATION AND MANAGEMENT =====

// Basic thread creation and management demonstrations
void demonstrateBasicThreads() {
    std::cout << "=== Basic Thread Creation and Management ===\n\n";
    
    // 1. Lambda function thread
    std::thread t1([]() {
        std::cout << "Thread " << std::this_thread::get_id() 
                  << ": Lambda function execution\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    // 2. Function pointer thread
    auto simple_function = []() {
        std::cout << "Thread " << std::this_thread::get_id() 
                  << ": Function pointer execution\n";
    };
    std::thread t2(simple_function);
    
    // 3. Member function thread
    class Worker {
    public:
        void work(int id) {
            std::cout << "Thread " << std::this_thread::get_id() 
                      << ": Worker " << id << " executing\n";
        }
    };
    
    Worker worker;
    std::thread t3(&Worker::work, &worker, 42);
    
    // 4. Thread with parameters
    auto parameterized_function = [](const std::string& message, int count) {
        for (int i = 0; i < count; ++i) {
            std::cout << "Thread " << std::this_thread::get_id() 
                      << ": " << message << " (" << i + 1 << ")\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };
    std::thread t4(parameterized_function, "Hello from thread", 3);
    
    // Join all threads
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    
    std::cout << "All basic threads completed\n\n";
}

// Thread lifecycle demonstration
void demonstrateThreadLifecycle() {
    std::cout << "=== Thread Lifecycle Demonstration ===\n\n";
    
    std::cout << "Main thread ID: " << std::this_thread::get_id() << "\n";
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << " threads\n\n";
    
    // 1. Thread creation and immediate join
    std::cout << "1. Thread creation and immediate join:\n";
    std::thread t1([]() {
        std::cout << "  Worker thread started: " << std::this_thread::get_id() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "  Worker thread finishing\n";
    });
    
    std::cout << "  Main thread waiting for worker...\n";
    t1.join();
    std::cout << "  Worker thread joined\n\n";
    
    // 2. Detached thread
    std::cout << "2. Detached thread (fire and forget):\n";
    std::thread t2([]() {
        std::cout << "  Detached thread running: " << std::this_thread::get_id() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "  Detached thread completing\n";
    });
    
    t2.detach();
    std::cout << "  Thread detached, main continues immediately\n";
    
    // Give detached thread time to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    // 3. Multiple threads with different timing
    std::cout << "\n3. Multiple threads with different timing:\n";
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([i]() {
            std::cout << "  Thread " << i << " (ID: " << std::this_thread::get_id() 
                      << ") starting\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * (i + 1)));
            std::cout << "  Thread " << i << " completing\n";
        });
    }
    
    std::cout << "  Main thread waiting for all workers...\n";
    for (auto& t : threads) {
        t.join();
    }
    std::cout << "  All worker threads joined\n\n";
}

// Thread properties and utilities
void demonstrateThreadProperties() {
    std::cout << "=== Thread Properties and Utilities ===\n\n";
    
    // Thread yield demonstration
    std::cout << "1. Thread yielding:\n";
    std::thread yielding_thread([]() {
        for (int i = 0; i < 5; ++i) {
            std::cout << "  Yielding thread iteration " << i << "\n";
            std::this_thread::yield();  // Give other threads a chance
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    // Competing thread
    std::thread competing_thread([]() {
        for (int i = 0; i < 5; ++i) {
            std::cout << "  Competing thread iteration " << i << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    yielding_thread.join();
    competing_thread.join();
    
    // Thread sleep demonstrations
    std::cout << "\n2. Thread sleep variations:\n";
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "  sleep_for(100ms) actual duration: " << duration.count() << "ms\n";
    
    // Sleep until specific time
    auto wake_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(50);
    std::this_thread::sleep_until(wake_time);
    std::cout << "  sleep_until completed\n\n";
}

// Exception handling in basic threads
void demonstrateBasicExceptionHandling() {
    std::cout << "=== Basic Exception Handling in Threads ===\n\n";
    
    std::cout << "1. Thread with exception (uncaught - terminates program):\n";
    std::cout << "   (Demonstration skipped to prevent termination)\n\n";
    
    std::cout << "2. Thread with internal exception handling:\n";
    std::thread safe_thread([]() {
        try {
            std::cout << "  Thread starting risky operation\n";
            // Simulate some work that might throw
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            
            // Uncomment to see exception handling:
            // throw std::runtime_error("Simulated error");
            
            std::cout << "  Thread completed successfully\n";
        } catch (const std::exception& e) {
            std::cout << "  Thread caught exception: " << e.what() << "\n";
        }
    });
    
    safe_thread.join();
    std::cout << "  Safe thread completed\n\n";
}

int main() {
    try {
        std::cout << "=== BASIC THREADING CONCEPTS ===\n";
        std::cout << "This file covers fundamental thread creation and management\n\n";
        
        demonstrateThreadLifecycle();
        demonstrateBasicThreads();
        demonstrateThreadProperties();
        demonstrateBasicExceptionHandling();
        
        std::cout << "=== KEY CONCEPTS COVERED ===\n";
        std::cout << "1. Thread creation with lambdas, functions, and member functions\n";
        std::cout << "2. Thread lifecycle: creation, execution, joining, detaching\n";
        std::cout << "3. Thread properties: ID, hardware concurrency\n";
        std::cout << "4. Thread utilities: sleep_for, sleep_until, yield\n";
        std::cout << "5. Basic exception safety in threads\n";
        std::cout << "6. Resource management with RAII (join in destructors)\n\n";
        
        std::cout << "=== NEXT STEPS ===\n";
        std::cout << "-> Run 02_mutex_synchronization.cpp to learn about thread synchronization\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

/*
Expected Output:
================
=== BASIC THREADING CONCEPTS ===
This file covers fundamental thread creation and management

=== Thread Lifecycle Demonstration ===

Main thread ID: [thread_id]
Hardware concurrency: 8 threads

1. Thread creation and immediate join:
  Main thread waiting for worker...
  Worker thread started: [worker_id]
  Worker thread finishing
  Worker thread joined

2. Detached thread (fire and forget):
  Thread detached, main continues immediately
  Detached thread running: [detached_id]
  Detached thread completing

3. Multiple threads with different timing:
  Main thread waiting for all workers...
  Thread 0 (ID: [id1]) starting
  Thread 1 (ID: [id2]) starting
  Thread 2 (ID: [id3]) starting
  Thread 0 completing
  Thread 1 completing
  Thread 2 completing
  All worker threads joined

=== Basic Thread Creation and Management ===

Thread [id]: Lambda function execution
Thread [id]: Function pointer execution
Thread [id]: Worker 42 executing
Thread [id]: Hello from thread (1)
Thread [id]: Hello from thread (2)
Thread [id]: Hello from thread (3)
All basic threads completed

=== Thread Properties and Utilities ===

1. Thread yielding:
  Yielding thread iteration 0
  Competing thread iteration 0
  [interleaved output due to yielding]
  ...

2. Thread sleep variations:
  sleep_for(100ms) actual duration: 100ms
  sleep_until completed

=== Basic Exception Handling in Threads ===

1. Thread with exception (uncaught - terminates program):
   (Demonstration skipped to prevent termination)

2. Thread with internal exception handling:
  Thread starting risky operation
  Thread completed successfully
  Safe thread completed

=== KEY CONCEPTS COVERED ===
1. Thread creation with lambdas, functions, and member functions
2. Thread lifecycle: creation, execution, joining, detaching
3. Thread properties: ID, hardware concurrency
4. Thread utilities: sleep_for, sleep_until, yield
5. Basic exception safety in threads
6. Resource management with RAII (join in destructors)

=== NEXT STEPS ===
-> Run 02_mutex_synchronization.cpp to learn about thread synchronization

Compilation command:
g++ -std=c++20 -Wall -Wextra -O2 -pthread 01_basic_threading.cpp -o 01_basic_threading

Key Learning Points:
===================
1. std::thread can be created with various callable objects
2. Always join() or detach() threads before destruction
3. Thread IDs are unique and can be used for identification
4. std::this_thread provides utilities for current thread
5. Exceptions in threads must be handled internally
6. Detached threads run independently but can't be joined
7. Thread creation has some overhead - consider thread pools for many tasks
8. RAII pattern ensures proper thread cleanup
*/
