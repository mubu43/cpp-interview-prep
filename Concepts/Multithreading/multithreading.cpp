#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <chrono>
#include <vector>
#include <queue>
#include <functional>
#include <random>
#include <memory>
#include <barrier>
#include <latch>
#include <semaphore>
#include <stop_token>

// Basic thread creation and management
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

// Thread synchronization with mutex
class SharedCounter {
private:
    int count_ = 0;
    mutable std::mutex mutex_;
    
public:
    void increment() {
        std::lock_guard<std::mutex> lock(mutex_);
        ++count_;
    }
    
    void decrement() {
        std::lock_guard<std::mutex> lock(mutex_);
        --count_;
    }
    
    int get() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }
    
    void unsafe_increment() {
        ++count_;  // Demonstrates race condition
    }
    
    int unsafe_get() const {
        return count_;  // Demonstrates race condition
    }
};

void demonstrateMutexSynchronization() {
    std::cout << "=== Mutex Synchronization Demonstration ===\n\n";
    
    const int num_threads = 4;
    const int increments_per_thread = 10000;
    
    // Safe counter with mutex
    SharedCounter safe_counter;
    std::vector<std::thread> safe_threads;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_threads; ++i) {
        safe_threads.emplace_back([&safe_counter, increments_per_thread]() {
            for (int j = 0; j < increments_per_thread; ++j) {
                safe_counter.increment();
            }
        });
    }
    
    for (auto& thread : safe_threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Safe counter result: " << safe_counter.get() 
              << " (expected: " << num_threads * increments_per_thread << ")\n";
    std::cout << "Time with mutex: " << duration.count() << " ms\n";
    
    // Unsafe counter without mutex (demonstrates race condition)
    SharedCounter unsafe_counter;
    std::vector<std::thread> unsafe_threads;
    
    start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_threads; ++i) {
        unsafe_threads.emplace_back([&unsafe_counter, increments_per_thread]() {
            for (int j = 0; j < increments_per_thread; ++j) {
                unsafe_counter.unsafe_increment();
            }
        });
    }
    
    for (auto& thread : unsafe_threads) {
        thread.join();
    }
    
    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Unsafe counter result: " << unsafe_counter.unsafe_get() 
              << " (expected: " << num_threads * increments_per_thread << ")\n";
    std::cout << "Time without mutex: " << duration.count() << " ms\n\n";
}

// Producer-Consumer pattern with condition variables
template<typename T>
class ThreadSafeQueue {
private:
    mutable std::mutex mutex_;
    std::queue<T> queue_;
    std::condition_variable condition_;
    
public:
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
        condition_.notify_one();
    }
    
    bool try_pop(T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        item = queue_.front();
        queue_.pop();
        return true;
    }
    
    void wait_and_pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return !queue_.empty(); });
        item = queue_.front();
        queue_.pop();
    }
    
    bool wait_for_pop(T& item, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (condition_.wait_for(lock, timeout, [this] { return !queue_.empty(); })) {
            item = queue_.front();
            queue_.pop();
            return true;
        }
        return false;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};

void demonstrateProducerConsumer() {
    std::cout << "=== Producer-Consumer with Condition Variables ===\n\n";
    
    ThreadSafeQueue<int> queue;
    std::atomic<bool> stop_production{false};
    std::atomic<int> produced_count{0};
    std::atomic<int> consumed_count{0};
    
    // Multiple producers
    std::vector<std::thread> producers;
    for (int i = 0; i < 2; ++i) {
        producers.emplace_back([&queue, &stop_production, &produced_count, i]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(1, 100);
            
            int item_counter = i * 1000;
            while (!stop_production.load()) {
                int item = item_counter++;
                queue.push(item);
                produced_count.fetch_add(1);
                std::cout << "Producer " << i << " produced: " << item << std::endl;
                
                // Random production rate
                std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
            }
            std::cout << "Producer " << i << " stopped\n";
        });
    }
    
    // Multiple consumers
    std::vector<std::thread> consumers;
    for (int i = 0; i < 3; ++i) {
        consumers.emplace_back([&queue, &stop_production, &consumed_count, i]() {
            int item;
            while (!stop_production.load() || !queue.empty()) {
                if (queue.wait_for_pop(item, std::chrono::milliseconds(100))) {
                    consumed_count.fetch_add(1);
                    std::cout << "Consumer " << i << " consumed: " << item << std::endl;
                    
                    // Simulate processing time
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }
            std::cout << "Consumer " << i << " stopped\n";
        });
    }
    
    // Run for 3 seconds
    std::this_thread::sleep_for(std::chrono::seconds(3));
    stop_production.store(true);
    
    // Wait for all threads to complete
    for (auto& producer : producers) {
        producer.join();
    }
    for (auto& consumer : consumers) {
        consumer.join();
    }
    
    std::cout << "Production completed. Produced: " << produced_count.load()
              << ", Consumed: " << consumed_count.load()
              << ", Remaining: " << queue.size() << "\n\n";
}

// Thread pool implementation
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
                        condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                        
                        if (stop_ && tasks_.empty()) {
                            return;
                        }
                        
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
            
            tasks_.emplace([task] { (*task)(); });
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

void demonstrateThreadPool() {
    std::cout << "=== Thread Pool Demonstration ===\n\n";
    
    ThreadPool pool(4);
    std::vector<std::future<int>> results;
    
    // Submit tasks to thread pool
    for (int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.enqueue([i] {
                std::cout << "Task " << i << " running on thread " 
                          << std::this_thread::get_id() << std::endl;
                
                // Simulate work
                std::this_thread::sleep_for(std::chrono::milliseconds(100 + i * 50));
                
                return i * i;
            })
        );
    }
    
    // Collect results
    for (size_t i = 0; i < results.size(); ++i) {
        int result = results[i].get();
        std::cout << "Task " << i << " result: " << result << std::endl;
    }
    
    std::cout << "Thread pool tasks completed\n\n";
}

// Future and Promise demonstration
void demonstrateFuturePromise() {
    std::cout << "=== Future and Promise Demonstration ===\n\n";
    
    // 1. std::async
    std::cout << "1. Using std::async:\n";
    auto future1 = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return 42;
    });
    
    auto future2 = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        return 24;
    });
    
    std::cout << "Waiting for async results...\n";
    std::cout << "Result 1: " << future1.get() << std::endl;
    std::cout << "Result 2: " << future2.get() << std::endl;
    
    // 2. std::promise and std::future
    std::cout << "\n2. Using std::promise and std::future:\n";
    std::promise<std::string> promise;
    std::future<std::string> future = promise.get_future();
    
    std::thread worker([&promise]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        promise.set_value("Hello from worker thread!");
    });
    
    std::cout << "Message from worker: " << future.get() << std::endl;
    worker.join();
    
    // 3. std::packaged_task
    std::cout << "\n3. Using std::packaged_task:\n";
    std::packaged_task<int(int, int)> task([](int a, int b) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return a + b;
    });
    
    std::future<int> task_future = task.get_future();
    std::thread task_thread(std::move(task), 10, 20);
    
    std::cout << "Packaged task result: " << task_future.get() << std::endl;
    task_thread.join();
    
    std::cout << std::endl;
}

// Atomic operations and lock-free programming
void demonstrateAtomicOperations() {
    std::cout << "=== Atomic Operations with Threads ===\n\n";
    
    std::atomic<int> atomic_counter{0};
    std::atomic<bool> ready{false};
    std::atomic<bool> done{false};
    
    const int num_threads = 4;
    const int increments_per_thread = 100000;
    
    std::vector<std::thread> threads;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Create threads that increment atomic counter
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&atomic_counter, &ready, &done, increments_per_thread, i]() {
            // Wait for signal to start
            while (!ready.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            
            for (int j = 0; j < increments_per_thread; ++j) {
                atomic_counter.fetch_add(1, std::memory_order_relaxed);
            }
            
            std::cout << "Thread " << i << " completed\n";
        });
    }
    
    // Signal all threads to start
    ready.store(true, std::memory_order_release);
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Atomic counter result: " << atomic_counter.load()
              << " (expected: " << num_threads * increments_per_thread << ")\n";
    std::cout << "Time with atomics: " << duration.count() << " ms\n\n";
}

// Modern C++20 synchronization primitives
void demonstrateModernSynchronization() {
    std::cout << "=== Modern C++20 Synchronization Primitives ===\n\n";
    
    // 1. std::barrier (C++20)
    std::cout << "1. Using std::barrier:\n";
    const int num_workers = 3;
    std::barrier work_done(num_workers);
    
    std::vector<std::thread> barrier_threads;
    for (int i = 0; i < num_workers; ++i) {
        barrier_threads.emplace_back([&work_done, i]() {
            // Phase 1 work
            std::cout << "Worker " << i << " completing phase 1\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100 + i * 50));
            
            work_done.arrive_and_wait();  // Synchronization point
            
            // Phase 2 work
            std::cout << "Worker " << i << " completing phase 2\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100 + i * 30));
        });
    }
    
    for (auto& thread : barrier_threads) {
        thread.join();
    }
    
    // 2. std::latch (C++20)
    std::cout << "\n2. Using std::latch:\n";
    std::latch start_latch(1);
    std::latch completion_latch(num_workers);
    
    std::vector<std::thread> latch_threads;
    for (int i = 0; i < num_workers; ++i) {
        latch_threads.emplace_back([&start_latch, &completion_latch, i]() {
            start_latch.wait();  // Wait for start signal
            
            std::cout << "Worker " << i << " started and working\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(200 + i * 30));
            
            completion_latch.count_down();  // Signal completion
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Starting all workers...\n";
    start_latch.count_down();  // Release all workers
    
    completion_latch.wait();  // Wait for all workers to complete
    std::cout << "All workers completed\n";
    
    for (auto& thread : latch_threads) {
        thread.join();
    }
    
    // 3. std::counting_semaphore (C++20)
    std::cout << "\n3. Using std::counting_semaphore:\n";
    std::counting_semaphore<3> semaphore(2);  // Allow 2 concurrent access
    
    std::vector<std::thread> semaphore_threads;
    for (int i = 0; i < 5; ++i) {
        semaphore_threads.emplace_back([&semaphore, i]() {
            semaphore.acquire();  // Acquire permit
            
            std::cout << "Thread " << i << " acquired resource\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            std::cout << "Thread " << i << " releasing resource\n";
            
            semaphore.release();  // Release permit
        });
    }
    
    for (auto& thread : semaphore_threads) {
        thread.join();
    }
    
    std::cout << std::endl;
}

// Exception handling in threads
void demonstrateExceptionHandling() {
    std::cout << "=== Exception Handling in Threads ===\n\n";
    
    // 1. Exception in std::async
    std::cout << "1. Exception in std::async:\n";
    auto future_with_exception = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        throw std::runtime_error("Exception from async task");
        return 42;
    });
    
    try {
        int result = future_with_exception.get();
        std::cout << "Result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    // 2. Exception in thread with promise
    std::cout << "\n2. Exception in thread with promise:\n";
    std::promise<int> promise_with_exception;
    std::future<int> future_from_promise = promise_with_exception.get_future();
    
    std::thread exception_thread([&promise_with_exception]() {
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            throw std::logic_error("Exception from promise thread");
        } catch (...) {
            promise_with_exception.set_exception(std::current_exception());
        }
    });
    
    try {
        int result = future_from_promise.get();
        std::cout << "Result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    exception_thread.join();
    std::cout << std::endl;
}

// Thread-local storage demonstration
thread_local int thread_local_counter = 0;

void demonstrateThreadLocalStorage() {
    std::cout << "=== Thread-Local Storage Demonstration ===\n\n";
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([i]() {
            thread_local_counter = i * 100;
            
            for (int j = 0; j < 5; ++j) {
                ++thread_local_counter;
                std::cout << "Thread " << i << ": counter = " 
                          << thread_local_counter << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "Main thread counter: " << thread_local_counter << std::endl;
    std::cout << std::endl;
}

// Performance benchmarking
void benchmarkThreadPerformance() {
    std::cout << "=== Thread Performance Benchmark ===\n";
    
    const int num_operations = 1000000;
    
    // Single-threaded baseline
    auto start = std::chrono::high_resolution_clock::now();
    
    int single_thread_result = 0;
    for (int i = 0; i < num_operations; ++i) {
        single_thread_result += i;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto single_thread_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Multi-threaded version
    const int num_threads = std::thread::hardware_concurrency();
    const int operations_per_thread = num_operations / num_threads;
    
    start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    std::vector<int> partial_results(num_threads);
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([t, operations_per_thread, &partial_results]() {
            int start_val = t * operations_per_thread;
            int end_val = start_val + operations_per_thread;
            
            int partial_sum = 0;
            for (int i = start_val; i < end_val; ++i) {
                partial_sum += i;
            }
            partial_results[t] = partial_sum;
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    int multi_thread_result = 0;
    for (int result : partial_results) {
        multi_thread_result += result;
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto multi_thread_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Single-threaded time: " << single_thread_time.count() << " ms\n";
    std::cout << "Multi-threaded time (" << num_threads << " threads): " 
              << multi_thread_time.count() << " ms\n";
    std::cout << "Speedup: " << static_cast<double>(single_thread_time.count()) / multi_thread_time.count() << "x\n";
    std::cout << "Results match: " << (single_thread_result == multi_thread_result ? "Yes" : "No") << "\n";
}

int main() {
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << " threads\n\n";
    
    demonstrateBasicThreads();
    demonstrateMutexSynchronization();
    demonstrateProducerConsumer();
    demonstrateThreadPool();
    demonstrateFuturePromise();
    demonstrateAtomicOperations();
    demonstrateModernSynchronization();
    demonstrateExceptionHandling();
    demonstrateThreadLocalStorage();
    benchmarkThreadPerformance();
    
    std::cout << "\n=== Key Threading Concepts ===\n";
    std::cout << "1. Thread lifecycle - creation, execution, joining\n";
    std::cout << "2. Synchronization - mutexes, condition variables, atomics\n";
    std::cout << "3. Data races - prevention and detection\n";
    std::cout << "4. Deadlock avoidance - lock ordering, timeouts\n";
    std::cout << "5. Performance - thread pools, load balancing\n";
    std::cout << "6. Modern features - futures, promises, barriers\n";
    
    return 0;
}
