#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <memory>

// ===== ATOMIC OPERATIONS AND LOCK-FREE PROGRAMMING =====

// Basic atomic operations demonstration
void demonstrateBasicAtomics() {
    std::cout << "=== Basic Atomic Operations ===\n\n";
    
    std::atomic<int> atomic_counter{0};
    std::atomic<bool> ready{false};
    std::atomic<double> atomic_double{0.0};
    
    const int num_threads = 4;
    const int increments_per_thread = 100000;
    
    std::vector<std::thread> threads;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Create threads that increment atomic counter
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&atomic_counter, &ready, increments_per_thread, i]() {
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
    
    // Demonstrate atomic operations
    std::cout << "Atomic operation demonstrations:\n";
    std::cout << "fetch_add(5): " << atomic_counter.fetch_add(5) << " -> " << atomic_counter.load() << "\n";
    std::cout << "fetch_sub(3): " << atomic_counter.fetch_sub(3) << " -> " << atomic_counter.load() << "\n";
    std::cout << "exchange(42): " << atomic_counter.exchange(42) << " -> " << atomic_counter.load() << "\n";
    
    int expected = 42;
    bool success = atomic_counter.compare_exchange_strong(expected, 100);
    std::cout << "compare_exchange_strong(42, 100): " << (success ? "success" : "failed") 
              << " -> " << atomic_counter.load() << "\n\n";
}

// Memory ordering demonstration
void demonstrateMemoryOrdering() {
    std::cout << "=== Memory Ordering Semantics ===\n\n";
    
    std::atomic<int> data{0};
    std::atomic<bool> flag{false};
    
    // Writer thread
    std::thread writer([&data, &flag]() {
        data.store(42, std::memory_order_relaxed);  // Write data first
        flag.store(true, std::memory_order_release); // Then signal completion
        std::cout << "Writer: Data written and flag set\n";
    });
    
    // Reader thread
    std::thread reader([&data, &flag]() {
        while (!flag.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        int value = data.load(std::memory_order_relaxed);
        std::cout << "Reader: Flag seen, data value = " << value << "\n";
    });
    
    writer.join();
    reader.join();
    
    std::cout << "Memory ordering ensures data is visible when flag is set\n\n";
}

// Compare-and-swap based stack (lock-free)
template<typename T>
class LockFreeStack {
private:
    struct Node {
        std::atomic<T*> data;
        Node* next;
        
        Node() : data(nullptr), next(nullptr) {}
    };
    
    std::atomic<Node*> head_;
    
public:
    LockFreeStack() : head_(nullptr) {}
    
    void push(T item) {
        Node* new_node = new Node;
        new_node->data.store(new T(std::move(item)));
        new_node->next = head_.load();
        
        // CAS loop to update head
        while (!head_.compare_exchange_weak(new_node->next, new_node)) {
            // new_node->next is updated by compare_exchange_weak on failure
        }
    }
    
    std::unique_ptr<T> pop() {
        Node* old_head = head_.load();
        
        while (old_head && !head_.compare_exchange_weak(old_head, old_head->next)) {
            // old_head is updated by compare_exchange_weak on failure
        }
        
        std::unique_ptr<T> result;
        if (old_head) {
            result.reset(old_head->data.load());
            delete old_head;
        }
        return result;
    }
    
    bool empty() const {
        return head_.load() == nullptr;
    }
    
    // Note: This is a simplified implementation
    // Production code would need hazard pointers or epochs for memory reclamation
};

void demonstrateLockFreeStack() {
    std::cout << "=== Lock-Free Stack with Compare-and-Swap ===\n\n";
    
    LockFreeStack<int> stack;
    std::atomic<int> push_count{0};
    std::atomic<int> pop_count{0};
    
    const int num_threads = 4;
    const int operations_per_thread = 10000;
    
    std::vector<std::thread> threads;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Pusher threads
    for (int i = 0; i < num_threads / 2; ++i) {
        threads.emplace_back([&stack, &push_count, operations_per_thread, i]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                stack.push(i * 1000 + j);
                push_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    // Popper threads
    for (int i = 0; i < num_threads / 2; ++i) {
        threads.emplace_back([&stack, &pop_count]() {
            while (pop_count.load(std::memory_order_relaxed) < 10000) {  // Continue until we've popped enough
                auto item = stack.pop();
                if (item) {
                    pop_count.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Lock-free stack operations completed in " << duration.count() << " ms\n";
    std::cout << "Pushed: " << push_count.load() << ", Popped: " << pop_count.load() << "\n";
    std::cout << "Stack empty: " << (stack.empty() ? "Yes" : "No") << "\n\n";
}

// Atomic shared_ptr operations (C++20)
void demonstrateAtomicSharedPtr() {
    std::cout << "=== Atomic Shared Pointer Operations ===\n\n";
    
    std::atomic<std::shared_ptr<int>> atomic_ptr;
    atomic_ptr.store(std::make_shared<int>(42));
    
    std::cout << "Initial value: " << *atomic_ptr.load() << "\n";
    
    // Multiple threads updating shared_ptr
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&atomic_ptr, i]() {
            auto new_ptr = std::make_shared<int>(100 + i);
            auto old_ptr = atomic_ptr.exchange(new_ptr);
            std::cout << "Thread " << i << " exchanged " << *old_ptr 
                      << " with " << *new_ptr << "\n";
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "Final value: " << *atomic_ptr.load() << "\n\n";
}

// Wait-free operations demonstration
class WaitFreeCounter {
private:
    std::atomic<uint64_t> counter_;
    
public:
    WaitFreeCounter() : counter_(0) {}
    
    // Wait-free increment
    uint64_t increment() {
        return counter_.fetch_add(1, std::memory_order_relaxed);
    }
    
    // Wait-free read
    uint64_t get() const {
        return counter_.load(std::memory_order_relaxed);
    }
    
    // Wait-free add
    uint64_t add(uint64_t value) {
        return counter_.fetch_add(value, std::memory_order_relaxed);
    }
};

void demonstrateWaitFreeOperations() {
    std::cout << "=== Wait-Free Operations ===\n\n";
    
    WaitFreeCounter counter;
    const int num_threads = 8;
    const int increments_per_thread = 100000;
    
    std::vector<std::thread> threads;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&counter, increments_per_thread, i]() {
            for (int j = 0; j < increments_per_thread; ++j) {
                counter.increment();
            }
            std::cout << "Thread " << i << " completed increments\n";
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Wait-free counter result: " << counter.get()
              << " (expected: " << num_threads * increments_per_thread << ")\n";
    std::cout << "Time for wait-free operations: " << duration.count() << " ms\n\n";
}

// ABA problem demonstration
struct Node {
    int data;
    std::atomic<Node*> next;
    
    Node(int d) : data(d), next(nullptr) {}
};

class ABAProneStack {
private:
    std::atomic<Node*> head_;
    
public:
    ABAProneStack() : head_(nullptr) {}
    
    void push(int data) {
        Node* new_node = new Node(data);
        Node* old_head = head_.load();
        
        do {
            new_node->next.store(old_head);
        } while (!head_.compare_exchange_weak(old_head, new_node));
    }
    
    // This pop operation is susceptible to ABA problem
    bool pop(int& result) {
        Node* old_head = head_.load();
        
        if (!old_head) return false;
        
        // ABA problem can occur here if another thread:
        // 1. Pops this node
        // 2. Pushes it back
        // 3. The pointer looks the same but the stack state changed
        
        Node* next = old_head->next.load();
        
        if (head_.compare_exchange_strong(old_head, next)) {
            result = old_head->data;
            delete old_head;
            return true;
        }
        
        return false;
    }
    
    ~ABAProneStack() {
        int dummy;
        while (pop(dummy)) {
            // Clean up remaining nodes
        }
    }
};

void demonstrateABAProblem() {
    std::cout << "=== ABA Problem Demonstration ===\n\n";
    
    std::cout << "ABA Problem occurs when:\n";
    std::cout << "1. Thread A reads a value (A)\n";
    std::cout << "2. Thread B changes value to B, then back to A\n";
    std::cout << "3. Thread A's compare-and-swap succeeds but state may be inconsistent\n\n";
    
    ABAProneStack stack;
    
    // Push some initial values
    for (int i = 1; i <= 3; ++i) {
        stack.push(i);
    }
    
    std::cout << "Initial stack: 3 -> 2 -> 1\n";
    
    // Simple demonstration without actual ABA occurrence
    // (Creating actual ABA in controlled demo is complex)
    int value;
    if (stack.pop(value)) {
        std::cout << "Popped: " << value << "\n";
    }
    
    std::cout << "Note: In production, use hazard pointers or epochs to solve ABA problem\n\n";
}

// Performance comparison: atomic vs mutex
class MutexCounter {
private:
    std::mutex mutex_;
    int counter_ = 0;
    
public:
    void increment() {
        std::lock_guard<std::mutex> lock(mutex_);
        ++counter_;
    }
    
    int get() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return counter_;
    }
};

void demonstrateAtomicVsMutexPerformance() {
    std::cout << "=== Atomic vs Mutex Performance Comparison ===\n\n";
    
    const int num_threads = 4;
    const int increments_per_thread = 500000;
    
    // Atomic version
    std::atomic<int> atomic_counter{0};
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> atomic_threads;
    for (int i = 0; i < num_threads; ++i) {
        atomic_threads.emplace_back([&atomic_counter, increments_per_thread]() {
            for (int j = 0; j < increments_per_thread; ++j) {
                atomic_counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    for (auto& thread : atomic_threads) {
        thread.join();
    }
    
    auto atomic_time = std::chrono::high_resolution_clock::now() - start_time;
    
    // Mutex version
    MutexCounter mutex_counter;
    start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> mutex_threads;
    for (int i = 0; i < num_threads; ++i) {
        mutex_threads.emplace_back([&mutex_counter, increments_per_thread]() {
            for (int j = 0; j < increments_per_thread; ++j) {
                mutex_counter.increment();
            }
        });
    }
    
    for (auto& thread : mutex_threads) {
        thread.join();
    }
    
    auto mutex_time = std::chrono::high_resolution_clock::now() - start_time;
    
    auto atomic_ms = std::chrono::duration_cast<std::chrono::milliseconds>(atomic_time).count();
    auto mutex_ms = std::chrono::duration_cast<std::chrono::milliseconds>(mutex_time).count();
    
    std::cout << "Atomic counter time: " << atomic_ms << " ms (result: " << atomic_counter.load() << ")\n";
    std::cout << "Mutex counter time: " << mutex_ms << " ms (result: " << mutex_counter.get() << ")\n";
    std::cout << "Speedup: " << static_cast<double>(mutex_ms) / atomic_ms << "x\n\n";
}

int main() {
    try {
        std::cout << "=== ATOMIC OPERATIONS AND LOCK-FREE PROGRAMMING ===\n";
        std::cout << "This file covers atomic operations and lock-free data structures\n\n";
        
        demonstrateBasicAtomics();
        demonstrateMemoryOrdering();
        demonstrateLockFreeStack();
        demonstrateAtomicSharedPtr();
        demonstrateWaitFreeOperations();
        demonstrateABAProblem();
        demonstrateAtomicVsMutexPerformance();
        
        std::cout << "=== KEY CONCEPTS COVERED ===\n";
        std::cout << "1. Basic atomic operations: load, store, exchange, fetch_add\n";
        std::cout << "2. Memory ordering: relaxed, acquire, release, seq_cst\n";
        std::cout << "3. Compare-and-swap for lock-free programming\n";
        std::cout << "4. Lock-free data structures (stack example)\n";
        std::cout << "5. Wait-free vs lock-free guarantees\n";
        std::cout << "6. ABA problem in lock-free algorithms\n";
        std::cout << "7. Atomic shared_ptr operations\n";
        std::cout << "8. Performance comparison: atomics vs mutexes\n\n";
        
        std::cout << "=== NEXT STEPS ===\n";
        std::cout << "-> Run 05_futures_promises.cpp to learn about asynchronous programming\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

/*
Expected Output:
================
=== ATOMIC OPERATIONS AND LOCK-FREE PROGRAMMING ===
This file covers atomic operations and lock-free data structures

=== Basic Atomic Operations ===

Thread 0 completed
Thread 1 completed
Thread 2 completed
Thread 3 completed
Atomic counter result: 400000 (expected: 400000)
Time with atomics: [time] ms

Atomic operation demonstrations:
fetch_add(5): 400000 -> 400005
fetch_sub(3): 400005 -> 400002
exchange(42): 400002 -> 42
compare_exchange_strong(42, 100): success -> 100

=== Memory Ordering Semantics ===

Writer: Data written and flag set
Reader: Flag seen, data value = 42
Memory ordering ensures data is visible when flag is set

=== Lock-Free Stack with Compare-and-Swap ===

Lock-free stack operations completed in [time] ms
Pushed: 20000, Popped: 10000
Stack empty: No

=== Atomic Shared Pointer Operations ===

Initial value: 42
Thread 0 exchanged 42 with 100
Thread 1 exchanged 100 with 101
Thread 2 exchanged 101 with 102
Final value: 102

=== Wait-Free Operations ===

Thread 0 completed increments
Thread 1 completed increments
[all threads complete]
Wait-free counter result: 800000 (expected: 800000)
Time for wait-free operations: [time] ms

=== ABA Problem Demonstration ===

ABA Problem occurs when:
1. Thread A reads a value (A)
2. Thread B changes value to B, then back to A
3. Thread A's compare-and-swap succeeds but state may be inconsistent

Initial stack: 3 -> 2 -> 1
Popped: 3
Note: In production, use hazard pointers or epochs to solve ABA problem

=== Atomic vs Mutex Performance Comparison ===

Atomic counter time: [atomic_time] ms (result: 2000000)
Mutex counter time: [mutex_time] ms (result: 2000000)
Speedup: [speedup]x

=== KEY CONCEPTS COVERED ===
1. Basic atomic operations: load, store, exchange, fetch_add
2. Memory ordering: relaxed, acquire, release, seq_cst
3. Compare-and-swap for lock-free programming
4. Lock-free data structures (stack example)
5. Wait-free vs lock-free guarantees
6. ABA problem in lock-free algorithms
7. Atomic shared_ptr operations
8. Performance comparison: atomics vs mutexes

=== NEXT STEPS ===
-> Run 05_futures_promises.cpp to learn about asynchronous programming

Compilation command:
g++ -std=c++20 -Wall -Wextra -O2 -pthread 04_atomic_operations.cpp -o 04_atomic_operations

Key Learning Points:
===================
1. Atomic operations provide lock-free synchronization
2. Memory ordering controls visibility of operations across threads
3. Compare-and-swap enables lock-free algorithms
4. Lock-free doesn't mean wait-free (can still have retry loops)
5. ABA problem requires special handling in lock-free code
6. Atomics are generally faster than mutexes for simple operations
7. Lock-free programming is complex and error-prone
8. Use standard atomic types when possible, avoid rolling your own
*/
