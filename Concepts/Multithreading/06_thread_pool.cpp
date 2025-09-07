#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <future>
#include <functional>
#include <atomic>
#include <chrono>

// ===== THREAD POOL IMPLEMENTATION AND PATTERNS =====

// Basic thread pool implementation
class ThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    
public:
    ThreadPool(size_t num_threads) : stop_(false) {
        std::cout << "Creating thread pool with " << num_threads << " threads\n";
        
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this, i] {
                std::cout << "Worker thread " << i << " started (ID: " 
                          << std::this_thread::get_id() << ")\n";
                
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this] { 
                            return stop_.load() || !tasks_.empty(); 
                        });
                        
                        if (stop_.load() && tasks_.empty()) {
                            std::cout << "Worker thread " << i << " shutting down\n";
                            return;
                        }
                        
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    
                    try {
                        task();
                    } catch (const std::exception& e) {
                        std::cout << "Worker thread " << i << " caught exception: " 
                                  << e.what() << "\n";
                    } catch (...) {
                        std::cout << "Worker thread " << i << " caught unknown exception\n";
                    }
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
            
            if (stop_.load()) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            
            tasks_.emplace([task] { (*task)(); });
        }
        
        condition_.notify_one();
        return result;
    }
    
    size_t queue_size() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return tasks_.size();
    }
    
    ~ThreadPool() {
        std::cout << "Shutting down thread pool...\n";
        
        stop_.store(true);
        condition_.notify_all();
        
        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        std::cout << "Thread pool shutdown complete\n";
    }
};

void demonstrateThreadPool() {
    std::cout << "=== Basic Thread Pool Demonstration ===\n\n";
    
    ThreadPool pool(4);
    std::vector<std::future<int>> results;
    
    std::cout << "\nSubmitting tasks to thread pool:\n";
    
    // Submit various tasks to thread pool
    for (int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.enqueue([i] {
                std::cout << "Task " << i << " running on thread " 
                          << std::this_thread::get_id() << std::endl;
                
                // Simulate different amounts of work
                std::this_thread::sleep_for(std::chrono::milliseconds(100 + i * 50));
                
                return i * i;
            })
        );
    }
    
    std::cout << "Queue size after submitting: " << pool.queue_size() << "\n\n";
    
    // Collect results
    std::cout << "Collecting results:\n";
    for (size_t i = 0; i < results.size(); ++i) {
        int result = results[i].get();
        std::cout << "Task " << i << " result: " << result << std::endl;
    }
    
    std::cout << "\n";
}

// Work-stealing thread pool (simplified version)
class WorkStealingThreadPool {
private:
    struct ThreadData {
        std::queue<std::function<void()>> local_queue;
        std::mutex queue_mutex;
        std::thread worker_thread;
    };
    
    std::vector<std::unique_ptr<ThreadData>> threads_;
    std::atomic<bool> stop_;
    std::condition_variable global_condition_;
    std::mutex global_mutex_;
    
public:
    WorkStealingThreadPool(size_t num_threads) : stop_(false) {
        std::cout << "Creating work-stealing thread pool with " << num_threads << " threads\n";
        
        for (size_t i = 0; i < num_threads; ++i) {
            auto thread_data = std::make_unique<ThreadData>();
            
            thread_data->worker_thread = std::thread([this, i, &thread_data = *thread_data] {
                std::cout << "Work-stealing worker " << i << " started\n";
                
                while (!stop_.load()) {
                    std::function<void()> task;
                    
                    // Try to get task from local queue first
                    {
                        std::lock_guard<std::mutex> lock(thread_data.queue_mutex);
                        if (!thread_data.local_queue.empty()) {
                            task = std::move(thread_data.local_queue.front());
                            thread_data.local_queue.pop();
                        }
                    }
                    
                    // If no local task, try to steal from other threads
                    if (!task) {
                        for (auto& other_thread : threads_) {
                            if (other_thread.get() == &thread_data) continue;
                            
                            std::lock_guard<std::mutex> lock(other_thread->queue_mutex);
                            if (!other_thread->local_queue.empty()) {
                                task = std::move(other_thread->local_queue.front());
                                other_thread->local_queue.pop();
                                std::cout << "Worker " << i << " stole task\n";
                                break;
                            }
                        }
                    }
                    
                    if (task) {
                        try {
                            task();
                        } catch (const std::exception& e) {
                            std::cout << "Worker " << i << " exception: " << e.what() << "\n";
                        }
                    } else {
                        // No work available, wait briefly
                        std::unique_lock<std::mutex> lock(global_mutex_);
                        global_condition_.wait_for(lock, std::chrono::milliseconds(10));
                    }
                }
                
                std::cout << "Work-stealing worker " << i << " shutting down\n";
            });
            
            threads_.push_back(std::move(thread_data));
        }
    }
    
    void submit(std::function<void()> task) {
        // Simple round-robin distribution
        static std::atomic<size_t> next_thread{0};
        size_t thread_idx = next_thread.fetch_add(1) % threads_.size();
        
        {
            std::lock_guard<std::mutex> lock(threads_[thread_idx]->queue_mutex);
            threads_[thread_idx]->local_queue.push(std::move(task));
        }
        
        global_condition_.notify_one();
    }
    
    ~WorkStealingThreadPool() {
        std::cout << "Shutting down work-stealing thread pool...\n";
        
        stop_.store(true);
        global_condition_.notify_all();
        
        for (auto& thread_data : threads_) {
            if (thread_data->worker_thread.joinable()) {
                thread_data->worker_thread.join();
            }
        }
        
        std::cout << "Work-stealing thread pool shutdown complete\n";
    }
};

void demonstrateWorkStealingPool() {
    std::cout << "=== Work-Stealing Thread Pool ===\n\n";
    
    WorkStealingThreadPool pool(3);
    
    std::cout << "\nSubmitting imbalanced workload:\n";
    
    // Submit tasks with imbalanced distribution
    for (int i = 0; i < 12; ++i) {
        pool.submit([i] {
            std::cout << "Work-stealing task " << i << " on thread " 
                      << std::this_thread::get_id() << "\n";
            
            // Varying work amounts
            int work_time = (i % 3 == 0) ? 200 : 50;
            std::this_thread::sleep_for(std::chrono::milliseconds(work_time));
        });
    }
    
    // Wait for completion
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "\n";
}

// Priority thread pool
class PriorityThreadPool {
private:
    enum class Priority { LOW = 0, NORMAL = 1, HIGH = 2 };
    
    struct Task {
        std::function<void()> function;
        Priority priority;
        int id;
        
        Task(std::function<void()> f, Priority p, int task_id) 
            : function(std::move(f)), priority(p), id(task_id) {}
        
        bool operator<(const Task& other) const {
            // Higher priority tasks have lower value for priority_queue
            return priority < other.priority;
        }
    };
    
    std::vector<std::thread> workers_;
    std::priority_queue<Task> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    std::atomic<int> task_counter_;
    
public:
    PriorityThreadPool(size_t num_threads) : stop_(false), task_counter_(0) {
        std::cout << "Creating priority thread pool with " << num_threads << " threads\n";
        
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this, i] {
                while (true) {
                    Task task([]{}, Priority::LOW, -1);
                    
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this] { 
                            return stop_.load() || !tasks_.empty(); 
                        });
                        
                        if (stop_.load() && tasks_.empty()) {
                            return;
                        }
                        
                        task = std::move(const_cast<Task&>(tasks_.top()));
                        tasks_.pop();
                    }
                    
                    std::cout << "Worker " << i << " executing task " << task.id 
                              << " (priority: " << static_cast<int>(task.priority) << ")\n";
                    
                    try {
                        task.function();
                    } catch (const std::exception& e) {
                        std::cout << "Task " << task.id << " exception: " << e.what() << "\n";
                    }
                }
            });
        }
    }
    
    void submit_high_priority(std::function<void()> task) {
        submit_task(std::move(task), Priority::HIGH);
    }
    
    void submit_normal_priority(std::function<void()> task) {
        submit_task(std::move(task), Priority::NORMAL);
    }
    
    void submit_low_priority(std::function<void()> task) {
        submit_task(std::move(task), Priority::LOW);
    }
    
private:
    void submit_task(std::function<void()> task, Priority priority) {
        int task_id = task_counter_.fetch_add(1);
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            tasks_.emplace(std::move(task), priority, task_id);
        }
        
        condition_.notify_one();
    }
    
public:
    ~PriorityThreadPool() {
        std::cout << "Shutting down priority thread pool...\n";
        
        stop_.store(true);
        condition_.notify_all();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        std::cout << "Priority thread pool shutdown complete\n";
    }
};

void demonstratePriorityPool() {
    std::cout << "=== Priority Thread Pool ===\n\n";
    
    PriorityThreadPool pool(2);
    
    std::cout << "\nSubmitting mixed priority tasks:\n";
    
    // Submit tasks with different priorities
    for (int i = 0; i < 3; ++i) {
        pool.submit_low_priority([i] {
            std::cout << "  LOW priority task " << i << " executing\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    for (int i = 0; i < 3; ++i) {
        pool.submit_normal_priority([i] {
            std::cout << "  NORMAL priority task " << i << " executing\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    for (int i = 0; i < 3; ++i) {
        pool.submit_high_priority([i] {
            std::cout << "  HIGH priority task " << i << " executing\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    // Wait for completion
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "\n";
}

// Thread pool with different task types
class TypedThreadPool {
private:
    ThreadPool cpu_pool_;
    ThreadPool io_pool_;
    
public:
    TypedThreadPool() : cpu_pool_(std::thread::hardware_concurrency()), 
                       io_pool_(std::thread::hardware_concurrency() * 2) {
        std::cout << "Created CPU pool (" << std::thread::hardware_concurrency() 
                  << " threads) and I/O pool (" << std::thread::hardware_concurrency() * 2 
                  << " threads)\n";
    }
    
    template<typename F, typename... Args>
    auto submit_cpu_task(F&& f, Args&&... args) {
        return cpu_pool_.enqueue(std::forward<F>(f), std::forward<Args>(args)...);
    }
    
    template<typename F, typename... Args>
    auto submit_io_task(F&& f, Args&&... args) {
        return io_pool_.enqueue(std::forward<F>(f), std::forward<Args>(args)...);
    }
};

void demonstrateTypedPool() {
    std::cout << "=== Typed Thread Pool (CPU vs I/O) ===\n\n";
    
    TypedThreadPool pool;
    
    std::vector<std::future<int>> cpu_results;
    std::vector<std::future<std::string>> io_results;
    
    std::cout << "\nSubmitting CPU-intensive tasks:\n";
    for (int i = 0; i < 4; ++i) {
        cpu_results.push_back(pool.submit_cpu_task([i] {
            std::cout << "CPU task " << i << " on thread " << std::this_thread::get_id() << "\n";
            
            // Simulate CPU-intensive work
            int sum = 0;
            for (int j = 0; j < 1000000; ++j) {
                sum += j % 1000;
            }
            
            return sum + i;
        }));
    }
    
    std::cout << "\nSubmitting I/O tasks:\n";
    for (int i = 0; i < 6; ++i) {
        io_results.push_back(pool.submit_io_task([i] {
            std::cout << "I/O task " << i << " on thread " << std::this_thread::get_id() << "\n";
            
            // Simulate I/O wait
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            
            return "I/O result " + std::to_string(i);
        }));
    }
    
    std::cout << "\nCollecting CPU results:\n";
    for (size_t i = 0; i < cpu_results.size(); ++i) {
        int result = cpu_results[i].get();
        std::cout << "CPU task " << i << " result: " << result << "\n";
    }
    
    std::cout << "\nCollecting I/O results:\n";
    for (size_t i = 0; i < io_results.size(); ++i) {
        std::string result = io_results[i].get();
        std::cout << "I/O task " << i << " result: " << result << "\n";
    }
    
    std::cout << "\n";
}

// Performance comparison
void benchmarkThreadPoolPerformance() {
    std::cout << "=== Thread Pool Performance Benchmark ===\n\n";
    
    const int num_tasks = 10000;
    const int work_per_task = 1000;
    
    // Single-threaded baseline
    auto start = std::chrono::high_resolution_clock::now();
    
    int single_thread_result = 0;
    for (int i = 0; i < num_tasks; ++i) {
        for (int j = 0; j < work_per_task; ++j) {
            single_thread_result += i * j;
        }
    }
    
    auto single_thread_time = std::chrono::high_resolution_clock::now() - start;
    
    // Thread pool version
    ThreadPool pool(std::thread::hardware_concurrency());
    start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::future<int>> futures;
    for (int i = 0; i < num_tasks; ++i) {
        futures.push_back(pool.enqueue([i, work_per_task] {
            int result = 0;
            for (int j = 0; j < work_per_task; ++j) {
                result += i * j;
            }
            return result;
        }));
    }
    
    int pool_result = 0;
    for (auto& future : futures) {
        pool_result += future.get();
    }
    
    auto pool_time = std::chrono::high_resolution_clock::now() - start;
    
    auto single_ms = std::chrono::duration_cast<std::chrono::milliseconds>(single_thread_time).count();
    auto pool_ms = std::chrono::duration_cast<std::chrono::milliseconds>(pool_time).count();
    
    std::cout << "Single-threaded time: " << single_ms << " ms\n";
    std::cout << "Thread pool time: " << pool_ms << " ms\n";
    std::cout << "Speedup: " << static_cast<double>(single_ms) / pool_ms << "x\n";
    std::cout << "Results match: " << (single_thread_result == pool_result ? "Yes" : "No") << "\n\n";
}

int main() {
    try {
        std::cout << "=== THREAD POOL PATTERNS AND IMPLEMENTATIONS ===\n";
        std::cout << "This file covers various thread pool designs and patterns\n\n";
        
        demonstrateThreadPool();
        demonstrateWorkStealingPool();
        demonstratePriorityPool();
        demonstrateTypedPool();
        benchmarkThreadPoolPerformance();
        
        std::cout << "=== KEY CONCEPTS COVERED ===\n";
        std::cout << "1. Basic thread pool with task queue\n";
        std::cout << "2. Work-stealing for load balancing\n";
        std::cout << "3. Priority-based task scheduling\n";
        std::cout << "4. Specialized pools for different workloads\n";
        std::cout << "5. Exception handling in worker threads\n";
        std::cout << "6. Graceful shutdown mechanisms\n";
        std::cout << "7. Performance benefits and overhead considerations\n";
        std::cout << "8. Resource management and thread lifecycle\n\n";
        
        std::cout << "=== NEXT STEPS ===\n";
        std::cout << "-> Run 07_modern_synchronization.cpp to learn about C++20 features\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

/*
Expected Output:
================
=== THREAD POOL PATTERNS AND IMPLEMENTATIONS ===
This file covers various thread pool designs and patterns

=== Basic Thread Pool Demonstration ===

Creating thread pool with 4 threads
Worker thread 0 started (ID: [thread_id])
Worker thread 1 started (ID: [thread_id])
Worker thread 2 started (ID: [thread_id])
Worker thread 3 started (ID: [thread_id])

Submitting tasks to thread pool:
Queue size after submitting: [count]

Task 0 running on thread [thread_id]
Task 1 running on thread [thread_id]
[tasks execute in parallel on different threads]

Collecting results:
Task 0 result: 0
Task 1 result: 1
[results collected as tasks complete]

=== Work-Stealing Thread Pool ===

Creating work-stealing thread pool with 3 threads
Work-stealing worker 0 started
Work-stealing worker 1 started
Work-stealing worker 2 started

Submitting imbalanced workload:
Work-stealing task 0 on thread [thread_id]
Worker 1 stole task
[work stealing occurs when threads are idle]

=== Priority Thread Pool ===

Creating priority thread pool with 2 threads

Submitting mixed priority tasks:
Worker 0 executing task 8 (priority: 2)
  HIGH priority task 0 executing
Worker 1 executing task 7 (priority: 2)
  HIGH priority task 1 executing
[HIGH priority tasks execute first]

=== Typed Thread Pool (CPU vs I/O) ===

Created CPU pool (8 threads) and I/O pool (16 threads)

Submitting CPU-intensive tasks:
CPU task 0 on thread [cpu_thread_id]
[CPU tasks on CPU pool]

Submitting I/O tasks:
I/O task 0 on thread [io_thread_id]
[I/O tasks on I/O pool]

=== Thread Pool Performance Benchmark ===

Single-threaded time: [time] ms
Thread pool time: [faster_time] ms
Speedup: [speedup]x
Results match: Yes

Shutting down thread pool...
Worker thread 0 shutting down
[all threads shutdown gracefully]

=== KEY CONCEPTS COVERED ===
1. Basic thread pool with task queue
2. Work-stealing for load balancing
3. Priority-based task scheduling
4. Specialized pools for different workloads
5. Exception handling in worker threads
6. Graceful shutdown mechanisms
7. Performance benefits and overhead considerations
8. Resource management and thread lifecycle

=== NEXT STEPS ===
-> Run 07_modern_synchronization.cpp to learn about C++20 features

Compilation command:
g++ -std=c++17 -Wall -Wextra -O2 -pthread 06_thread_pool.cpp -o 06_thread_pool

Key Learning Points:
===================
1. Thread pools reuse threads to avoid creation overhead
2. Work stealing helps balance load across threads
3. Priority queues enable task scheduling based on importance
4. Specialized pools can optimize for specific workload types
5. Exception handling is crucial in worker threads
6. Graceful shutdown prevents resource leaks
7. Thread pools are ideal for many small tasks
8. Consider CPU vs I/O bound tasks when designing pools
*/
