# C++ Multithreading with std::thread - Complete Study Guide

## Table of Contents
1. [Introduction](#introduction)
2. [Basic Thread Management](#basic-thread-management)
3. [Thread Synchronization](#thread-synchronization)
4. [Advanced Synchronization](#advanced-synchronization)
5. [Asynchronous Programming](#asynchronous-programming)
6. [Thread Pools](#thread-pools)
7. [Performance Considerations](#performance-considerations)
8. [Best Practices](#best-practices)
9. [Common Pitfalls](#common-pitfalls)
10. [Interview Questions](#interview-questions)

## Introduction

C++11 introduced `std::thread` as part of the standard library, providing a portable way to create and manage threads. Modern C++ threading offers powerful abstractions for concurrent programming, from basic thread creation to sophisticated synchronization primitives.

### Key Features
- **Platform Independence**: Works across different operating systems
- **RAII Support**: Automatic resource management
- **Type Safety**: Strong typing for thread-safe operations
- **Modern Syntax**: Lambda support and move semantics
- **Rich Ecosystem**: Futures, promises, atomics, and more

## Basic Thread Management

### 1. Thread Creation

#### Different Ways to Create Threads
```cpp
// 1. Lambda function
std::thread t1([]() {
    std::cout << "Lambda thread\n";
});

// 2. Function pointer
void my_function() { /*...*/ }
std::thread t2(my_function);

// 3. Function object
struct Functor {
    void operator()() { /*...*/ }
};
std::thread t3(Functor{});

// 4. Member function
class MyClass {
public:
    void member_func() { /*...*/ }
};
MyClass obj;
std::thread t4(&MyClass::member_func, &obj);

// 5. With parameters
std::thread t5([](int x, std::string s) {
    // Use x and s
}, 42, "hello");
```

### 2. Thread Lifecycle

#### States
- **Created**: Thread object exists but not running
- **Running**: Thread is executing
- **Joinable**: Can be joined or detached
- **Finished**: Thread completed execution
- **Joined/Detached**: Thread resources cleaned up

#### Management Operations
```cpp
std::thread t([]() { /* work */ });

// Check if thread is joinable
if (t.joinable()) {
    // Option 1: Wait for completion
    t.join();
    
    // Option 2: Detach (runs independently)
    // t.detach();
}

// Get thread ID
std::thread::id id = t.get_id();

// Hardware concurrency hint
unsigned int cores = std::thread::hardware_concurrency();
```

### 3. Thread Properties

#### Thread Identification
```cpp
// Current thread ID
auto this_id = std::this_thread::get_id();

// Compare thread IDs
if (t.get_id() == this_id) {
    // Same thread
}
```

#### Thread Control
```cpp
// Yield CPU to other threads
std::this_thread::yield();

// Sleep for duration
std::this_thread::sleep_for(std::chrono::seconds(1));

// Sleep until time point
auto wake_time = std::chrono::steady_clock::now() + std::chrono::seconds(5);
std::this_thread::sleep_until(wake_time);
```

## Thread Synchronization

### 1. Mutexes

#### Basic Mutex Usage
```cpp
std::mutex mtx;
int shared_data = 0;

void safe_increment() {
    std::lock_guard<std::mutex> lock(mtx);  // RAII
    ++shared_data;
}  // Automatic unlock

void manual_locking() {
    mtx.lock();
    try {
        ++shared_data;
        mtx.unlock();
    } catch (...) {
        mtx.unlock();
        throw;
    }
}
```

#### Mutex Types
```cpp
// 1. Basic mutex
std::mutex basic_mutex;

// 2. Recursive mutex (same thread can lock multiple times)
std::recursive_mutex recursive_mutex;

// 3. Timed mutex (supports timeout)
std::timed_mutex timed_mutex;
if (timed_mutex.try_lock_for(std::chrono::seconds(1))) {
    // Got lock within 1 second
    timed_mutex.unlock();
}

// 4. Shared mutex (readers-writers lock)
std::shared_mutex shared_mutex;

// Reader
std::shared_lock<std::shared_mutex> read_lock(shared_mutex);

// Writer
std::unique_lock<std::shared_mutex> write_lock(shared_mutex);
```

#### Lock Types
```cpp
// 1. lock_guard - Simple RAII lock
std::lock_guard<std::mutex> guard(mtx);

// 2. unique_lock - More flexible
std::unique_lock<std::mutex> lock(mtx);
lock.unlock();  // Manual unlock
lock.lock();    // Manual relock

// 3. shared_lock - For shared mutexes
std::shared_lock<std::shared_mutex> shared_lock(shared_mtx);

// 4. scoped_lock - Multiple mutexes (C++17)
std::scoped_lock lock(mtx1, mtx2, mtx3);
```

### 2. Condition Variables

#### Basic Usage
```cpp
std::mutex mtx;
std::condition_variable cv;
bool ready = false;

// Waiting thread
void wait_for_signal() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return ready; });  // Wait until ready is true
    // Process data
}

// Signaling thread
void send_signal() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
    }
    cv.notify_one();  // or cv.notify_all()
}
```

#### Producer-Consumer Example
```cpp
template<typename T>
class ThreadSafeQueue {
private:
    mutable std::mutex mtx_;
    std::queue<T> queue_;
    std::condition_variable condition_;

public:
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(item);
        condition_.notify_one();
    }
    
    void wait_and_pop(T& item) {
        std::unique_lock<std::mutex> lock(mtx_);
        condition_.wait(lock, [this] { return !queue_.empty(); });
        item = queue_.front();
        queue_.pop();
    }
    
    bool try_pop(T& item) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (queue_.empty()) return false;
        item = queue_.front();
        queue_.pop();
        return true;
    }
};
```

### 3. Atomic Operations

#### Basic Atomic Types
```cpp
std::atomic<int> counter{0};
std::atomic<bool> flag{false};
std::atomic<double*> pointer{nullptr};

// Atomic operations
int old_value = counter.fetch_add(1);  // Atomic increment
bool was_set = flag.exchange(true);    // Atomic exchange

// Compare and swap
int expected = 10;
bool success = counter.compare_exchange_strong(expected, 20);
```

#### Memory Ordering
```cpp
// Relaxed ordering (no synchronization)
counter.store(42, std::memory_order_relaxed);

// Acquire-release ordering
flag.store(true, std::memory_order_release);
while (!flag.load(std::memory_order_acquire)) {
    // Wait
}

// Sequential consistency (default, strongest)
counter.store(42, std::memory_order_seq_cst);
```

## Advanced Synchronization

### 1. Modern C++20 Primitives

#### std::barrier
```cpp
std::barrier<> work_done(num_threads);

void worker_thread() {
    // Phase 1 work
    do_phase1_work();
    
    work_done.arrive_and_wait();  // Synchronization point
    
    // Phase 2 work
    do_phase2_work();
}
```

#### std::latch
```cpp
std::latch start_signal(1);
std::latch completion_signal(num_workers);

void worker() {
    start_signal.wait();        // Wait for start
    do_work();
    completion_signal.count_down();  // Signal completion
}

void coordinator() {
    setup_work();
    start_signal.count_down();       // Start all workers
    completion_signal.wait();        // Wait for all to complete
}
```

#### std::counting_semaphore
```cpp
std::counting_semaphore<10> resource_pool(5);  // 5 available resources

void use_resource() {
    resource_pool.acquire();    // Get resource
    use_limited_resource();
    resource_pool.release();    // Return resource
}
```

### 2. Read-Write Locks
```cpp
class ThreadSafeData {
private:
    mutable std::shared_mutex mtx_;
    std::vector<int> data_;

public:
    // Multiple readers can access simultaneously
    int read_data(size_t index) const {
        std::shared_lock<std::shared_mutex> lock(mtx_);
        return data_[index];
    }
    
    // Exclusive writer access
    void write_data(size_t index, int value) {
        std::unique_lock<std::shared_mutex> lock(mtx_);
        data_[index] = value;
    }
};
```

## Asynchronous Programming

### 1. std::async

#### Launch Policies
```cpp
// Asynchronous execution (new thread)
auto future1 = std::async(std::launch::async, []() {
    return compute_something();
});

// Deferred execution (lazy evaluation)
auto future2 = std::async(std::launch::deferred, []() {
    return compute_something_else();
});

// Implementation chooses
auto future3 = std::async([]() {
    return compute_default();
});

// Get results
int result1 = future1.get();  // Blocks until ready
int result2 = future2.get();  // Executes now if deferred
```

#### Timeout and Status
```cpp
auto future = std::async(std::launch::async, long_computation);

// Check status without blocking
if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
    int result = future.get();
} else {
    std::cout << "Still computing...\n";
}

// Wait with timeout
auto status = future.wait_for(std::chrono::seconds(5));
switch (status) {
    case std::future_status::ready:
        // Result is available
        break;
    case std::future_status::timeout:
        // Timeout occurred
        break;
    case std::future_status::deferred:
        // Deferred function not started
        break;
}
```

### 2. std::promise and std::future

#### Basic Usage
```cpp
std::promise<int> promise;
std::future<int> future = promise.get_future();

std::thread worker([&promise]() {
    // Do some work
    int result = compute_result();
    promise.set_value(result);
});

// Get result from another thread
int result = future.get();
worker.join();
```

#### Exception Handling
```cpp
std::promise<int> promise;
std::future<int> future = promise.get_future();

std::thread worker([&promise]() {
    try {
        int result = risky_computation();
        promise.set_value(result);
    } catch (...) {
        promise.set_exception(std::current_exception());
    }
});

try {
    int result = future.get();
} catch (const std::exception& e) {
    std::cout << "Caught: " << e.what() << std::endl;
}

worker.join();
```

### 3. std::packaged_task

#### Function Wrapping
```cpp
// Wrap a function in a packaged_task
std::packaged_task<int(int, int)> task([](int a, int b) {
    return a + b;
});

std::future<int> future = task.get_future();

// Execute in another thread
std::thread worker(std::move(task), 10, 20);

int result = future.get();  // Get result
worker.join();
```

## Thread Pools

### 1. Basic Thread Pool Implementation

```cpp
class ThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;

public:
    ThreadPool(size_t num_threads) : stop_(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this] { 
                            return stop_ || !tasks_.empty(); 
                        });
                        
                        if (stop_ && tasks_.empty()) return;
                        
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    
                    task();
                }
            });
        }
    }
    
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        
        using return_type = typename std::result_of<F(Args...)>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        
        condition_.notify_one();
        return result;
    }
    
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (std::thread& worker : workers_) {
            worker.join();
        }
    }
};
```

### 2. Usage Examples
```cpp
ThreadPool pool(4);

// Submit tasks and get futures
std::vector<std::future<int>> results;
for (int i = 0; i < 8; ++i) {
    results.emplace_back(
        pool.enqueue([i] {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return i * i;
        })
    );
}

// Collect results
for (auto& result : results) {
    std::cout << result.get() << " ";
}
```

## Performance Considerations

### 1. Thread Creation Overhead

#### Costs
- **OS Resources**: Thread stack, kernel structures
- **Context Switching**: CPU time switching between threads
- **Memory**: Each thread has its own stack (typically 1-8MB)
- **Synchronization**: Overhead of coordination primitives

#### Optimization Strategies
```cpp
// 1. Thread pools - reuse threads
ThreadPool pool(std::thread::hardware_concurrency());

// 2. Work stealing - balance load
// 3. Lock-free data structures - avoid blocking
std::atomic<int> counter;  // Instead of mutex + int

// 4. Thread-local storage - reduce contention
thread_local Cache thread_cache;
```

### 2. Cache Effects

#### False Sharing
```cpp
// BAD: Variables on same cache line
struct BadLayout {
    alignas(64) std::atomic<int> counter1;
    std::atomic<int> counter2;  // Likely same cache line
};

// GOOD: Prevent false sharing
struct GoodLayout {
    alignas(64) std::atomic<int> counter1;
    alignas(64) std::atomic<int> counter2;  // Different cache lines
};
```

#### Data Locality
```cpp
// Process data in chunks for better cache utilization
void process_data_chunks(std::vector<int>& data, int num_threads) {
    int chunk_size = data.size() / num_threads;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        int start = i * chunk_size;
        int end = (i == num_threads - 1) ? data.size() : start + chunk_size;
        
        threads.emplace_back([&data, start, end]() {
            for (int j = start; j < end; ++j) {
                process_element(data[j]);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}
```

### 3. Load Balancing

#### Static vs Dynamic
```cpp
// Static: Fixed work distribution
void static_distribution(int total_work, int num_threads) {
    int work_per_thread = total_work / num_threads;
    // Each thread gets fixed amount
}

// Dynamic: Work stealing
class WorkStealingPool {
    std::vector<std::queue<Task>> thread_queues_;
    // Threads steal from other queues when idle
};
```

## Best Practices

### 1. Thread Safety Design

#### Immutable Data
```cpp
// Prefer immutable data structures
class ImmutableVector {
    const std::vector<int> data_;
public:
    ImmutableVector(std::vector<int> data) : data_(std::move(data)) {}
    
    int operator[](size_t index) const { return data_[index]; }
    size_t size() const { return data_.size(); }
    
    // No mutation methods
};
```

#### Local Variables
```cpp
void thread_function() {
    // Local variables are thread-safe by default
    int local_var = 42;
    std::vector<int> local_vector;
    
    // No synchronization needed for locals
    process_data(local_var, local_vector);
}
```

### 2. Resource Management

#### RAII for Threads
```cpp
class ThreadGuard {
    std::thread& t_;
public:
    explicit ThreadGuard(std::thread& t) : t_(t) {}
    
    ~ThreadGuard() {
        if (t_.joinable()) {
            t_.join();
        }
    }
    
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
};

void function() {
    std::thread t([]() { /* work */ });
    ThreadGuard guard(t);
    
    // Thread automatically joined on scope exit
    may_throw_exception();
}
```

#### Exception Safety
```cpp
void exception_safe_threading() {
    std::vector<std::thread> threads;
    
    try {
        for (int i = 0; i < 4; ++i) {
            threads.emplace_back(worker_function);
        }
        
        // Do work that might throw
        risky_operation();
        
    } catch (...) {
        // Ensure all threads are cleaned up
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
        throw;
    }
    
    // Normal cleanup
    for (auto& t : threads) {
        t.join();
    }
}
```

### 3. Debugging and Testing

#### Thread Sanitizer
```bash
# Compile with thread sanitizer
g++ -fsanitize=thread -g -o program program.cpp

# Detects:
# - Data races
# - Use after free
# - Thread leaks
```

#### Logging
```cpp
#include <sstream>

thread_local std::ostringstream thread_log;

void thread_safe_log(const std::string& message) {
    static std::mutex log_mutex;
    std::lock_guard<std::mutex> lock(log_mutex);
    
    std::cout << "[Thread " << std::this_thread::get_id() 
              << "] " << message << std::endl;
}
```

## Common Pitfalls

### 1. Data Races
```cpp
// WRONG: Race condition
int global_counter = 0;
void increment() {
    global_counter++;  // Not atomic!
}

// CORRECT: Protected access
std::mutex counter_mutex;
int global_counter = 0;
void safe_increment() {
    std::lock_guard<std::mutex> lock(counter_mutex);
    global_counter++;
}
```

### 2. Deadlocks
```cpp
// WRONG: Potential deadlock
std::mutex mutex1, mutex2;

void function1() {
    std::lock_guard<std::mutex> lock1(mutex1);
    std::lock_guard<std::mutex> lock2(mutex2);
}

void function2() {
    std::lock_guard<std::mutex> lock2(mutex2);  // Reverse order!
    std::lock_guard<std::mutex> lock1(mutex1);
}

// CORRECT: Consistent ordering or std::lock
void safe_function1() {
    std::scoped_lock lock(mutex1, mutex2);  // C++17
}

void safe_function2() {
    std::scoped_lock lock(mutex1, mutex2);  // Same order
}
```

### 3. Detached Thread Issues
```cpp
// WRONG: Accessing local data in detached thread
void problematic_function() {
    int local_data = 42;
    
    std::thread t([&local_data]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << local_data << std::endl;  // Dangling reference!
    });
    
    t.detach();  // Function returns, local_data destroyed
}

// CORRECT: Copy data or use proper lifetime management
void safe_function() {
    int local_data = 42;
    
    std::thread t([local_data]() {  // Copy, not reference
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << local_data << std::endl;
    });
    
    t.detach();  // Safe now
}
```

### 4. Exception in Threads
```cpp
// WRONG: Unhandled exception terminates program
void dangerous_thread() {
    std::thread t([]() {
        throw std::runtime_error("Oops");  // Terminates program!
    });
    t.join();
}

// CORRECT: Handle exceptions in thread
void safe_thread() {
    std::exception_ptr exception;
    
    std::thread t([&exception]() {
        try {
            throw std::runtime_error("Oops");
        } catch (...) {
            exception = std::current_exception();
        }
    });
    
    t.join();
    
    if (exception) {
        std::rethrow_exception(exception);
    }
}
```

## Interview Questions

### Basic Level

1. **What is std::thread and how do you create one?**
   - Standard C++ class for creating threads
   - Can pass function, lambda, or callable object
   - Must call join() or detach() before destruction

2. **What's the difference between join() and detach()?**
   - join(): Wait for thread completion, can get return value
   - detach(): Thread runs independently, no way to wait

3. **What is a data race?**
   - Multiple threads accessing same memory location
   - At least one access is a write
   - No synchronization between accesses

### Intermediate Level

4. **Explain mutex types in C++.**
   - std::mutex: Basic mutual exclusion
   - std::recursive_mutex: Same thread can lock multiple times
   - std::timed_mutex: Supports timeout operations
   - std::shared_mutex: Reader-writer lock

5. **How do condition variables work?**
   - Allow threads to wait for specific conditions
   - Must be used with mutex for protection
   - notify_one() vs notify_all() for waking waiters

6. **What is std::atomic and when to use it?**
   - Provides lock-free thread-safe operations
   - Good for simple operations like counters
   - Different memory ordering guarantees available

### Advanced Level

7. **Compare different synchronization primitives.**
   - Mutex: Mutual exclusion, can cause blocking
   - Atomic: Lock-free, limited to simple operations
   - Condition variable: Event-based synchronization
   - Barrier/Latch: Synchronization points

8. **Explain memory ordering in atomic operations.**
   - relaxed: No ordering constraints
   - acquire/release: Synchronizes-with relationship
   - seq_cst: Sequential consistency (default)

9. **How do you implement a thread pool?**
   - Fixed number of worker threads
   - Work queue with synchronization
   - Task submission returns future for result

10. **What is false sharing and how to prevent it?**
    - Multiple threads accessing different variables on same cache line
    - Causes cache line bouncing between cores
    - Prevent with alignment (alignas) or padding

### Expert Level

11. **How do you handle exceptions in multi-threaded code?**
    - Exceptions don't cross thread boundaries
    - Use std::exception_ptr to transfer exceptions
    - Promise/future can propagate exceptions

12. **Explain the happens-before relationship.**
    - Ordering constraint between operations
    - Established by synchronization operations
    - Fundamental to memory model reasoning

13. **How do you debug multi-threaded programs?**
    - Thread sanitizer for race detection
    - Careful logging with synchronization
    - Reproducible test cases with stress testing

### Performance Questions

14. **When should you use more threads than CPU cores?**
    - I/O bound tasks that block frequently
    - Different types of work (CPU vs I/O)
    - Consider context switching overhead

15. **How do you measure thread performance?**
    - Wall clock time vs CPU time
    - Throughput vs latency metrics
    - Scalability with increasing cores

## Summary

C++ multithreading with `std::thread` provides powerful tools for concurrent programming:

**Core Concepts:**
- Thread creation and lifecycle management
- Synchronization primitives (mutex, condition variables, atomics)
- Asynchronous programming (futures, promises, async)
- Modern synchronization tools (barriers, latches, semaphores)

**Key Benefits:**
- Platform-independent threading
- Type-safe and RAII-friendly
- Rich ecosystem of synchronization primitives
- Integration with modern C++ features

**Best Practices:**
- Prefer immutable data and local variables
- Use RAII for resource management
- Choose appropriate synchronization primitives
- Test thoroughly with thread sanitizer
- Design for scalability and maintainability

Understanding these concepts is crucial for writing efficient, safe, and maintainable concurrent C++ applications.
