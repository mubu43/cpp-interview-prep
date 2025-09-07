#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <vector>
#include <string>
#include <exception>
#include <functional>

// ===== FUTURES, PROMISES, AND ASYNC PROGRAMMING =====

// Basic std::async demonstration
void demonstrateAsync() {
    std::cout << "=== std::async Demonstration ===\n\n";
    
    // 1. std::async with different launch policies
    std::cout << "1. Using std::async with launch::async:\n";
    auto future1 = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "   Async task running on thread: " << std::this_thread::get_id() << "\n";
        return 42;
    });
    
    auto future2 = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        std::cout << "   Another async task on thread: " << std::this_thread::get_id() << "\n";
        return 24;
    });
    
    std::cout << "   Main thread: " << std::this_thread::get_id() << "\n";
    std::cout << "   Waiting for async results...\n";
    
    std::cout << "   Result 1: " << future1.get() << "\n";
    std::cout << "   Result 2: " << future2.get() << "\n\n";
    
    // 2. std::async with launch::deferred
    std::cout << "2. Using std::async with launch::deferred:\n";
    auto deferred_future = std::async(std::launch::deferred, []() {
        std::cout << "   Deferred task executing when get() is called\n";
        return "Deferred result";
    });
    
    std::cout << "   Deferred task created but not yet executed\n";
    std::string result = deferred_future.get();
    std::cout << "   Deferred result: " << result << "\n\n";
    
    // 3. std::async with function parameters
    std::cout << "3. std::async with function parameters:\n";
    auto parameterized_task = [](int a, int b, const std::string& msg) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "   " << msg << ": " << a << " + " << b << " = " << (a + b) << "\n";
        return a + b;
    };
    
    auto param_future = std::async(std::launch::async, parameterized_task, 10, 20, "Addition task");
    int sum = param_future.get();
    std::cout << "   Sum result: " << sum << "\n\n";
}

// Promise and Future demonstration
void demonstratePromiseFuture() {
    std::cout << "=== std::promise and std::future ===\n\n";
    
    // 1. Basic promise/future communication
    std::cout << "1. Basic promise/future communication:\n";
    std::promise<std::string> promise;
    std::future<std::string> future = promise.get_future();
    
    std::thread worker([&promise]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        promise.set_value("Hello from worker thread!");
    });
    
    std::cout << "   Main thread waiting for message...\n";
    std::string message = future.get();
    std::cout << "   Received: " << message << "\n";
    worker.join();
    
    // 2. Promise with exception
    std::cout << "\n2. Promise with exception handling:\n";
    std::promise<int> exception_promise;
    std::future<int> exception_future = exception_promise.get_future();
    
    std::thread exception_worker([&exception_promise]() {
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            throw std::runtime_error("Something went wrong in worker");
            exception_promise.set_value(42);  // This won't be reached
        } catch (...) {
            exception_promise.set_exception(std::current_exception());
        }
    });
    
    try {
        int result = exception_future.get();
        std::cout << "   Result: " << result << "\n";
    } catch (const std::exception& e) {
        std::cout << "   Caught exception: " << e.what() << "\n";
    }
    
    exception_worker.join();
    
    // 3. Multiple promises with one future
    std::cout << "\n3. Shared future with multiple threads:\n";
    std::promise<int> shared_promise;
    std::shared_future<int> shared_future = shared_promise.get_future();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([shared_future, i]() {
            int value = shared_future.get();  // All threads can call get()
            std::cout << "   Thread " << i << " received: " << value << "\n";
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    shared_promise.set_value(100);
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "\n";
}

// Packaged task demonstration
void demonstratePackagedTask() {
    std::cout << "=== std::packaged_task Demonstration ===\n\n";
    
    // 1. Basic packaged task
    std::cout << "1. Basic packaged task:\n";
    std::packaged_task<int(int, int)> task([](int a, int b) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "   Computing " << a << " * " << b << "\n";
        return a * b;
    });
    
    std::future<int> task_future = task.get_future();
    std::thread task_thread(std::move(task), 6, 7);
    
    std::cout << "   Waiting for task result...\n";
    int result = task_future.get();
    std::cout << "   Task result: " << result << "\n";
    task_thread.join();
    
    // 2. Packaged task with function object
    std::cout << "\n2. Packaged task with function object:\n";
    
    struct Calculator {
        double operator()(double x, double y) {
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            std::cout << "   Calculating " << x << " / " << y << "\n";
            if (y == 0) throw std::runtime_error("Division by zero");
            return x / y;
        }
    };
    
    std::packaged_task<double(double, double)> calc_task(Calculator{});
    std::future<double> calc_future = calc_task.get_future();
    
    std::thread calc_thread(std::move(calc_task), 15.0, 3.0);
    
    try {
        double calc_result = calc_future.get();
        std::cout << "   Calculation result: " << calc_result << "\n";
    } catch (const std::exception& e) {
        std::cout << "   Error: " << e.what() << "\n";
    }
    
    calc_thread.join();
    std::cout << "\n";
}

// Future status and timeout operations
void demonstrateFutureStatus() {
    std::cout << "=== Future Status and Timeouts ===\n\n";
    
    // 1. Future with timeout
    std::cout << "1. Future timeout operations:\n";
    auto slow_task = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return "Slow task completed";
    });
    
    // Check if ready immediately
    std::future_status status = slow_task.wait_for(std::chrono::milliseconds(0));
    std::cout << "   Immediate status: " << 
        (status == std::future_status::ready ? "ready" : 
         status == std::future_status::timeout ? "timeout" : "deferred") << "\n";
    
    // Wait with timeout
    status = slow_task.wait_for(std::chrono::milliseconds(200));
    std::cout << "   Status after 200ms: " << 
        (status == std::future_status::ready ? "ready" : 
         status == std::future_status::timeout ? "timeout" : "deferred") << "\n";
    
    // Wait until completion
    std::cout << "   Waiting for completion...\n";
    std::string result = slow_task.get();
    std::cout << "   Result: " << result << "\n";
    
    // 2. wait_until with absolute time
    std::cout << "\n2. Using wait_until with absolute deadline:\n";
    auto timed_task = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        return 99;
    });
    
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(150);
    status = timed_task.wait_until(deadline);
    
    std::cout << "   Status at deadline: " << 
        (status == std::future_status::ready ? "ready" : "timeout") << "\n";
    
    if (status == std::future_status::timeout) {
        std::cout << "   Task still running, waiting for completion...\n";
        int timed_result = timed_task.get();
        std::cout << "   Final result: " << timed_result << "\n";
    }
    
    std::cout << "\n";
}

// Multiple futures and coordination
void demonstrateMultipleFutures() {
    std::cout << "=== Multiple Futures Coordination ===\n\n";
    
    // 1. Parallel execution with multiple futures
    std::cout << "1. Parallel execution with result aggregation:\n";
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 4; ++i) {
        futures.push_back(std::async(std::launch::async, [i]() {
            // Simulate different processing times
            std::this_thread::sleep_for(std::chrono::milliseconds(100 + i * 50));
            int result = i * i;
            std::cout << "   Task " << i << " completed with result: " << result << "\n";
            return result;
        }));
    }
    
    // Collect all results
    int total = 0;
    for (auto& future : futures) {
        total += future.get();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "   Total sum: " << total << "\n";
    std::cout << "   Parallel execution time: " << duration.count() << " ms\n";
    
    // 2. Sequential vs parallel comparison
    std::cout << "\n2. Sequential execution for comparison:\n";
    start_time = std::chrono::high_resolution_clock::now();
    
    total = 0;
    for (int i = 0; i < 4; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + i * 50));
        int result = i * i;
        total += result;
        std::cout << "   Sequential task " << i << " completed: " << result << "\n";
    }
    
    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "   Sequential total: " << total << "\n";
    std::cout << "   Sequential execution time: " << duration.count() << " ms\n\n";
}

// Advanced async patterns
class AsyncCalculator {
public:
    // Chain of dependent async operations
    std::future<double> calculate_complex(double x) {
        return std::async(std::launch::async, [this, x]() {
            // Step 1: Square the input
            auto step1_future = std::async(std::launch::async, [x]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                return x * x;
            });
            
            double step1_result = step1_future.get();
            std::cout << "   Step 1 complete: " << x << "² = " << step1_result << "\n";
            
            // Step 2: Add constant
            auto step2_future = std::async(std::launch::async, [step1_result]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                return step1_result + 10.0;
            });
            
            double step2_result = step2_future.get();
            std::cout << "   Step 2 complete: " << step1_result << " + 10 = " << step2_result << "\n";
            
            // Step 3: Take square root
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            double final_result = std::sqrt(step2_result);
            std::cout << "   Step 3 complete: √" << step2_result << " = " << final_result << "\n";
            
            return final_result;
        });
    }
    
    // Async operation with cancellation-like behavior
    std::future<std::string> long_running_task(std::chrono::milliseconds timeout) {
        auto promise = std::make_shared<std::promise<std::string>>();
        auto future = promise->get_future();
        
        std::thread([promise, timeout]() {
            auto start = std::chrono::steady_clock::now();
            
            for (int i = 0; i < 100; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                
                // Check for timeout
                auto elapsed = std::chrono::steady_clock::now() - start;
                if (elapsed > timeout) {
                    promise->set_exception(
                        std::make_exception_ptr(std::runtime_error("Task timeout"))
                    );
                    return;
                }
                
                if (i % 20 == 0) {
                    std::cout << "   Progress: " << i << "%\n";
                }
            }
            
            promise->set_value("Long running task completed successfully");
        }).detach();
        
        return future;
    }
};

void demonstrateAdvancedAsync() {
    std::cout << "=== Advanced Async Patterns ===\n\n";
    
    AsyncCalculator calc;
    
    // 1. Chained async operations
    std::cout << "1. Chained async operations:\n";
    auto complex_future = calc.calculate_complex(5.0);
    
    std::cout << "   Starting complex calculation...\n";
    double result = complex_future.get();
    std::cout << "   Final result: " << result << "\n";
    
    // 2. Async with timeout simulation
    std::cout << "\n2. Long-running task with timeout:\n";
    auto timeout_future = calc.long_running_task(std::chrono::milliseconds(1000));
    
    try {
        std::string timeout_result = timeout_future.get();
        std::cout << "   " << timeout_result << "\n";
    } catch (const std::exception& e) {
        std::cout << "   Task failed: " << e.what() << "\n";
    }
    
    std::cout << "\n";
}

// Exception propagation in async
void demonstrateAsyncExceptions() {
    std::cout << "=== Exception Handling in Async Operations ===\n\n";
    
    // 1. Exception in std::async
    std::cout << "1. Exception propagation from std::async:\n";
    auto exception_future = std::async(std::launch::async, []() -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        throw std::runtime_error("Error in async task");
        return 42;  // Never reached
    });
    
    try {
        int result = exception_future.get();
        std::cout << "   Unexpected result: " << result << "\n";
    } catch (const std::exception& e) {
        std::cout << "   Caught async exception: " << e.what() << "\n";
    }
    
    // 2. Multiple async tasks with exceptions
    std::cout << "\n2. Multiple tasks with some exceptions:\n";
    std::vector<std::future<int>> exception_futures;
    
    for (int i = 0; i < 4; ++i) {
        exception_futures.push_back(std::async(std::launch::async, [i]() -> int {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            
            if (i == 2) {
                throw std::runtime_error("Task " + std::to_string(i) + " failed");
            }
            
            return i * 10;
        }));
    }
    
    // Process results, handling exceptions
    for (size_t i = 0; i < exception_futures.size(); ++i) {
        try {
            int result = exception_futures[i].get();
            std::cout << "   Task " << i << " succeeded: " << result << "\n";
        } catch (const std::exception& e) {
            std::cout << "   Task " << i << " failed: " << e.what() << "\n";
        }
    }
    
    std::cout << "\n";
}

int main() {
    try {
        std::cout << "=== FUTURES, PROMISES, AND ASYNC PROGRAMMING ===\n";
        std::cout << "This file covers asynchronous programming in C++\n\n";
        
        demonstrateAsync();
        demonstratePromiseFuture();
        demonstratePackagedTask();
        demonstrateFutureStatus();
        demonstrateMultipleFutures();
        demonstrateAdvancedAsync();
        demonstrateAsyncExceptions();
        
        std::cout << "=== KEY CONCEPTS COVERED ===\n";
        std::cout << "1. std::async for launching asynchronous tasks\n";
        std::cout << "2. std::promise and std::future for thread communication\n";
        std::cout << "3. std::packaged_task for wrapping callable objects\n";
        std::cout << "4. Launch policies: async vs deferred\n";
        std::cout << "5. Future status checking and timeouts\n";
        std::cout << "6. Multiple futures coordination and aggregation\n";
        std::cout << "7. Exception propagation in async operations\n";
        std::cout << "8. Advanced patterns: chaining and cancellation\n\n";
        
        std::cout << "=== NEXT STEPS ===\n";
        std::cout << "-> Run 06_thread_pool.cpp to learn about thread pool patterns\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

/*
Expected Output:
================
=== FUTURES, PROMISES, AND ASYNC PROGRAMMING ===
This file covers asynchronous programming in C++

=== std::async Demonstration ===

1. Using std::async with launch::async:
   Main thread: [main_thread_id]
   Waiting for async results...
   Another async task on thread: [thread_id_2]
   Async task running on thread: [thread_id_1]
   Result 1: 42
   Result 2: 24

2. Using std::async with launch::deferred:
   Deferred task created but not yet executed
   Deferred task executing when get() is called
   Deferred result: Deferred result

3. std::async with function parameters:
   Addition task: 10 + 20 = 30
   Sum result: 30

=== std::promise and std::future ===

1. Basic promise/future communication:
   Main thread waiting for message...
   Received: Hello from worker thread!

2. Promise with exception handling:
   Caught exception: Something went wrong in worker

3. Shared future with multiple threads:
   Thread 0 received: 100
   Thread 1 received: 100
   Thread 2 received: 100

=== std::packaged_task Demonstration ===

1. Basic packaged task:
   Waiting for task result...
   Computing 6 * 7
   Task result: 42

2. Packaged task with function object:
   Calculating 15 / 3
   Calculation result: 5

=== Future Status and Timeouts ===

1. Future timeout operations:
   Immediate status: timeout
   Status after 200ms: timeout
   Waiting for completion...
   Result: Slow task completed

2. Using wait_until with absolute deadline:
   Status at deadline: timeout
   Task still running, waiting for completion...
   Final result: 99

=== Multiple Futures Coordination ===

1. Parallel execution with result aggregation:
   Task 0 completed with result: 0
   Task 1 completed with result: 1
   Task 2 completed with result: 4
   Task 3 completed with result: 9
   Total sum: 14
   Parallel execution time: ~300 ms

2. Sequential execution for comparison:
   Sequential task 0 completed: 0
   Sequential task 1 completed: 1
   Sequential task 2 completed: 4
   Sequential task 3 completed: 9
   Sequential total: 14
   Sequential execution time: ~800 ms

=== Advanced Async Patterns ===

1. Chained async operations:
   Starting complex calculation...
   Step 1 complete: 5² = 25
   Step 2 complete: 25 + 10 = 35
   Step 3 complete: √35 = 5.91608
   Final result: 5.91608

2. Long-running task with timeout:
   Progress: 0%
   Task failed: Task timeout

=== Exception Handling in Async Operations ===

1. Exception propagation from std::async:
   Caught async exception: Error in async task

2. Multiple tasks with some exceptions:
   Task 0 succeeded: 0
   Task 1 succeeded: 10
   Task 2 failed: Task 2 failed
   Task 3 succeeded: 30

=== KEY CONCEPTS COVERED ===
1. std::async for launching asynchronous tasks
2. std::promise and std::future for thread communication
3. std::packaged_task for wrapping callable objects
4. Launch policies: async vs deferred
5. Future status checking and timeouts
6. Multiple futures coordination and aggregation
7. Exception propagation in async operations
8. Advanced patterns: chaining and cancellation

=== NEXT STEPS ===
-> Run 06_thread_pool.cpp to learn about thread pool patterns

Compilation command:
g++ -std=c++17 -Wall -Wextra -O2 -pthread 05_futures_promises.cpp -o 05_futures_promises

Key Learning Points:
===================
1. std::async provides easy way to run tasks asynchronously
2. std::promise/future enables one-way communication between threads
3. std::packaged_task wraps callable objects for future execution
4. Launch policies control when and how tasks execute
5. Futures support timeout operations for non-blocking checks
6. Exception propagation works seamlessly through futures
7. Parallel execution can provide significant speedup
8. Always handle exceptions in async operations
*/
