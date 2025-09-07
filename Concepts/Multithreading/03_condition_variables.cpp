#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <chrono>
#include <random>
#include <atomic>

// ===== CONDITION VARIABLES AND THREAD COMMUNICATION =====

// Thread-safe queue using condition variables
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
    
    bool wait_until_pop(T& item, std::chrono::steady_clock::time_point deadline) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (condition_.wait_until(lock, deadline, [this] { return !queue_.empty(); })) {
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

// Producer-Consumer pattern demonstration
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
            std::uniform_int_distribution<> dis(50, 150);
            
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
                    std::this_thread::sleep_for(std::chrono::milliseconds(75));
                }
            }
            std::cout << "Consumer " << i << " stopped\n";
        });
    }
    
    // Run for 2 seconds
    std::this_thread::sleep_for(std::chrono::seconds(2));
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

// Condition variable spurious wakeup demonstration
void demonstrateSpuriousWakeups() {
    std::cout << "=== Spurious Wakeups and Predicate Importance ===\n\n";
    
    std::mutex mutex;
    std::condition_variable cv;
    bool ready = false;
    int data = 0;
    
    // Worker thread waiting for signal
    std::thread worker([&]() {
        std::unique_lock<std::mutex> lock(mutex);
        
        // WRONG: Without predicate - vulnerable to spurious wakeups
        // cv.wait(lock);
        
        // CORRECT: With predicate - handles spurious wakeups
        cv.wait(lock, [&ready] { return ready; });
        
        std::cout << "Worker received data: " << data << "\n";
    });
    
    // Main thread preparing data and signaling
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    {
        std::lock_guard<std::mutex> lock(mutex);
        data = 42;
        ready = true;
    }
    cv.notify_one();
    
    worker.join();
    std::cout << "Spurious wakeup demonstration completed\n\n";
}

// Multiple condition variables for complex synchronization
class WorkQueue {
private:
    std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    std::queue<int> queue_;
    const size_t max_size_;
    bool shutdown_;
    
public:
    WorkQueue(size_t max_size) : max_size_(max_size), shutdown_(false) {}
    
    void push(int item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Wait until there's space or shutdown
        not_full_.wait(lock, [this] { 
            return queue_.size() < max_size_ || shutdown_; 
        });
        
        if (shutdown_) {
            throw std::runtime_error("Queue is shut down");
        }
        
        queue_.push(item);
        not_empty_.notify_one();
    }
    
    bool pop(int& item, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000)) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (not_empty_.wait_for(lock, timeout, [this] { 
            return !queue_.empty() || shutdown_; 
        })) {
            if (!queue_.empty()) {
                item = queue_.front();
                queue_.pop();
                not_full_.notify_one();
                return true;
            }
        }
        return false;
    }
    
    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
        not_empty_.notify_all();
        not_full_.notify_all();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};

void demonstrateBoundedQueue() {
    std::cout << "=== Bounded Queue with Multiple Condition Variables ===\n\n";
    
    WorkQueue queue(3);  // Small queue to demonstrate blocking
    
    // Fast producer
    std::thread producer([&queue]() {
        try {
            for (int i = 0; i < 10; ++i) {
                queue.push(i);
                std::cout << "Produced: " << i << " (queue size: " << queue.size() << ")\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        } catch (const std::exception& e) {
            std::cout << "Producer error: " << e.what() << "\n";
        }
    });
    
    // Slow consumer
    std::thread consumer([&queue]() {
        int item;
        while (queue.pop(item, std::chrono::milliseconds(2000))) {
            std::cout << "Consumed: " << item << " (queue size: " << queue.size() << ")\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(200));  // Slower than producer
        }
    });
    
    // Let them run for a while
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // Shutdown the queue
    queue.shutdown();
    
    producer.join();
    consumer.join();
    
    std::cout << "Bounded queue demonstration completed\n\n";
}

// Condition variable with timeout patterns
class TimedWaitDemo {
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool event_occurred_ = false;
    
public:
    void trigger_event() {
        std::lock_guard<std::mutex> lock(mutex_);
        event_occurred_ = true;
        cv_.notify_one();
    }
    
    bool wait_for_event(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_for(lock, timeout, [this] { return event_occurred_; });
    }
    
    bool wait_until_event(std::chrono::steady_clock::time_point deadline) {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_until(lock, deadline, [this] { return event_occurred_; });
    }
    
    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        event_occurred_ = false;
    }
};

void demonstrateTimedWaits() {
    std::cout << "=== Timed Waits with Condition Variables ===\n\n";
    
    TimedWaitDemo demo;
    
    // Test 1: wait_for with timeout
    std::cout << "1. Testing wait_for with 500ms timeout (no event):\n";
    auto start = std::chrono::steady_clock::now();
    bool result = demo.wait_for_event(std::chrono::milliseconds(500));
    auto duration = std::chrono::steady_clock::now() - start;
    std::cout << "   Result: " << (result ? "Event occurred" : "Timeout") 
              << " after " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << "ms\n\n";
    
    // Test 2: wait_for with early event
    std::cout << "2. Testing wait_for with event after 200ms:\n";
    std::thread trigger_thread([&demo]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        demo.trigger_event();
    });
    
    start = std::chrono::steady_clock::now();
    result = demo.wait_for_event(std::chrono::milliseconds(1000));
    duration = std::chrono::steady_clock::now() - start;
    std::cout << "   Result: " << (result ? "Event occurred" : "Timeout") 
              << " after " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << "ms\n\n";
    
    trigger_thread.join();
    
    // Test 3: wait_until with absolute deadline
    demo.reset();
    std::cout << "3. Testing wait_until with absolute deadline:\n";
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
    start = std::chrono::steady_clock::now();
    result = demo.wait_until_event(deadline);
    duration = std::chrono::steady_clock::now() - start;
    std::cout << "   Result: " << (result ? "Event occurred" : "Timeout") 
              << " after " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << "ms\n\n";
}

// notify_one vs notify_all demonstration
class NotificationDemo {
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    int counter_ = 0;
    bool finished_ = false;
    
public:
    void wait_for_signal(int worker_id) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return counter_ > 0 || finished_; });
        
        if (!finished_ && counter_ > 0) {
            --counter_;
            std::cout << "Worker " << worker_id << " got signal (remaining: " << counter_ << ")\n";
        } else {
            std::cout << "Worker " << worker_id << " exiting (finished)\n";
        }
    }
    
    void signal_one(int count) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            counter_ += count;
        }
        for (int i = 0; i < count; ++i) {
            cv_.notify_one();  // Wake up one thread at a time
        }
    }
    
    void signal_all(int count) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            counter_ += count;
        }
        cv_.notify_all();  // Wake up all waiting threads
    }
    
    void finish() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            finished_ = true;
        }
        cv_.notify_all();
    }
};

void demonstrateNotificationTypes() {
    std::cout << "=== notify_one vs notify_all Demonstration ===\n\n";
    
    NotificationDemo demo;
    
    // Create multiple waiting workers
    std::vector<std::thread> workers;
    for (int i = 0; i < 5; ++i) {
        workers.emplace_back([&demo, i]() {
            demo.wait_for_signal(i);
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "1. Using notify_one (signaling 2 workers):\n";
    demo.signal_one(2);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "\n2. Using notify_all (signaling 3 more):\n";
    demo.signal_all(3);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "\n3. Finishing all workers:\n";
    demo.finish();
    
    for (auto& worker : workers) {
        worker.join();
    }
    
    std::cout << "Notification demonstration completed\n\n";
}

int main() {
    try {
        std::cout << "=== CONDITION VARIABLE CONCEPTS ===\n";
        std::cout << "This file covers condition variables and thread communication\n\n";
        
        demonstrateProducerConsumer();
        demonstrateSpuriousWakeups();
        demonstrateBoundedQueue();
        demonstrateTimedWaits();
        demonstrateNotificationTypes();
        
        std::cout << "=== KEY CONCEPTS COVERED ===\n";
        std::cout << "1. condition_variable for thread coordination\n";
        std::cout << "2. Producer-consumer pattern implementation\n";
        std::cout << "3. Spurious wakeups and predicate functions\n";
        std::cout << "4. Bounded queues with multiple condition variables\n";
        std::cout << "5. Timed waits: wait_for and wait_until\n";
        std::cout << "6. notify_one vs notify_all strategies\n";
        std::cout << "7. Thread-safe queue implementation\n";
        std::cout << "8. Graceful shutdown patterns\n\n";
        
        std::cout << "=== NEXT STEPS ===\n";
        std::cout << "-> Run 04_atomic_operations.cpp to learn about lock-free programming\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

/*
Expected Output:
================
=== CONDITION VARIABLE CONCEPTS ===
This file covers condition variables and thread communication

=== Producer-Consumer with Condition Variables ===

Producer 0 produced: 0
Producer 1 produced: 1000
Consumer 0 consumed: 0
Consumer 1 consumed: 1000
[interleaved production and consumption...]
Producer 0 stopped
Producer 1 stopped
Consumer 0 stopped
Consumer 1 stopped
Consumer 2 stopped
Production completed. Produced: [count], Consumed: [count], Remaining: [count]

=== Spurious Wakeups and Predicate Importance ===

Worker received data: 42
Spurious wakeup demonstration completed

=== Bounded Queue with Multiple Condition Variables ===

Produced: 0 (queue size: 1)
Produced: 1 (queue size: 2)
Produced: 2 (queue size: 3)
Consumed: 0 (queue size: 2)
Produced: 3 (queue size: 3)
[producer blocks when queue is full]
Consumed: 1 (queue size: 2)
[pattern continues...]
Producer error: Queue is shut down
Bounded queue demonstration completed

=== Timed Waits with Condition Variables ===

1. Testing wait_for with 500ms timeout (no event):
   Result: Timeout after ~500ms

2. Testing wait_for with event after 200ms:
   Result: Event occurred after ~200ms

3. Testing wait_until with absolute deadline:
   Result: Timeout after ~300ms

=== notify_one vs notify_all Demonstration ===

1. Using notify_one (signaling 2 workers):
Worker 0 got signal (remaining: 1)
Worker 1 got signal (remaining: 0)

2. Using notify_all (signaling 3 more):
Worker 2 got signal (remaining: 2)
Worker 3 got signal (remaining: 1)
Worker 4 got signal (remaining: 0)

3. Finishing all workers:
Notification demonstration completed

=== KEY CONCEPTS COVERED ===
1. condition_variable for thread coordination
2. Producer-consumer pattern implementation
3. Spurious wakeups and predicate functions
4. Bounded queues with multiple condition variables
5. Timed waits: wait_for and wait_until
6. notify_one vs notify_all strategies
7. Thread-safe queue implementation
8. Graceful shutdown patterns

=== NEXT STEPS ===
-> Run 04_atomic_operations.cpp to learn about lock-free programming

Compilation command:
g++ -std=c++17 -Wall -Wextra -O2 -pthread 03_condition_variables.cpp -o 03_condition_variables

Key Learning Points:
===================
1. condition_variable enables threads to wait for specific conditions
2. Always use predicates with wait() to handle spurious wakeups
3. notify_one wakes single thread, notify_all wakes all waiting threads
4. Producer-consumer is classic pattern for thread communication
5. Bounded queues prevent unlimited memory growth
6. Timed waits prevent indefinite blocking
7. Multiple condition variables can manage complex states
8. Graceful shutdown requires careful state management
*/
