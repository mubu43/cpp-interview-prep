#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <shared_mutex>

// ===== MUTEX AND BASIC SYNCHRONIZATION =====

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
    
    // Advanced mutex operations
    bool try_increment() {
        std::unique_lock<std::mutex> lock(mutex_, std::try_to_lock);
        if (lock.owns_lock()) {
            ++count_;
            return true;
        }
        return false;
    }
    
    bool try_increment_for(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
        if (lock.try_lock_for(timeout)) {
            ++count_;
            return true;
        }
        return false;
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

// Different lock types demonstration
void demonstrateLockTypes() {
    std::cout << "=== Different Lock Types ===\n\n";
    
    SharedCounter counter;
    
    // 1. lock_guard - RAII, automatic unlock
    std::cout << "1. Using lock_guard (RAII):\n";
    {
        std::thread t1([&counter]() {
            std::lock_guard<std::mutex> lock(counter.mutex_);  // Note: accessing private for demo
            // Lock automatically released when t1 scope ends
            counter.unsafe_increment();  // Safe because we hold the lock
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
        t1.join();
    }
    
    // 2. unique_lock - more flexible
    std::cout << "2. Using unique_lock (flexible):\n";
    std::thread t2([&counter]() {
        std::unique_lock<std::mutex> lock(counter.mutex_, std::defer_lock);
        
        // Lock manually when needed
        lock.lock();
        counter.unsafe_increment();
        lock.unlock();
        
        // Do some work without lock
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        // Lock again
        lock.lock();
        counter.unsafe_increment();
        // Lock automatically released when t2 scope ends
    });
    t2.join();
    
    // 3. try_lock operations
    std::cout << "3. Using try_lock operations:\n";
    bool success = counter.try_increment();
    std::cout << "   try_increment success: " << (success ? "Yes" : "No") << "\n";
    
    success = counter.try_increment_for(std::chrono::milliseconds(10));
    std::cout << "   try_increment_for success: " << (success ? "Yes" : "No") << "\n";
    
    std::cout << "   Final counter value: " << counter.get() << "\n\n";
}

// Multiple mutex handling and deadlock prevention
class BankAccount {
private:
    mutable std::mutex mutex_;
    int balance_;
    int account_id_;
    
public:
    BankAccount(int id, int initial_balance) 
        : balance_(initial_balance), account_id_(id) {}
    
    int get_balance() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return balance_;
    }
    
    int get_id() const { return account_id_; }
    
    // Potential deadlock scenario
    void transfer_unsafe(BankAccount& to, int amount) {
        std::lock_guard<std::mutex> lock1(mutex_);
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate work
        std::lock_guard<std::mutex> lock2(to.mutex_);
        
        if (balance_ >= amount) {
            balance_ -= amount;
            to.balance_ += amount;
        }
    }
    
    // Deadlock-free transfer using lock ordering
    void transfer_safe(BankAccount& to, int amount) {
        if (account_id_ < to.account_id_) {
            std::lock_guard<std::mutex> lock1(mutex_);
            std::lock_guard<std::mutex> lock2(to.mutex_);
            perform_transfer(to, amount);
        } else {
            std::lock_guard<std::mutex> lock1(to.mutex_);
            std::lock_guard<std::mutex> lock2(mutex_);
            perform_transfer(to, amount);
        }
    }
    
    // Deadlock-free transfer using std::lock
    void transfer_with_std_lock(BankAccount& to, int amount) {
        std::lock(mutex_, to.mutex_);
        std::lock_guard<std::mutex> lock1(mutex_, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(to.mutex_, std::adopt_lock);
        
        perform_transfer(to, amount);
    }
    
private:
    void perform_transfer(BankAccount& to, int amount) {
        if (balance_ >= amount) {
            balance_ -= amount;
            to.balance_ += amount;
            std::cout << "Transferred " << amount << " from account " 
                      << account_id_ << " to account " << to.account_id_ << "\n";
        }
    }
    
    friend void demonstrateDeadlockPrevention();
};

void demonstrateDeadlockPrevention() {
    std::cout << "=== Deadlock Prevention Techniques ===\n\n";
    
    BankAccount account1(1, 1000);
    BankAccount account2(2, 1000);
    
    std::cout << "Initial balances - Account 1: " << account1.get_balance()
              << ", Account 2: " << account2.get_balance() << "\n\n";
    
    std::cout << "1. Safe transfers using lock ordering:\n";
    std::vector<std::thread> threads;
    
    // Multiple concurrent transfers
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&account1, &account2, i]() {
            if (i % 2 == 0) {
                account1.transfer_safe(account2, 50);
            } else {
                account2.transfer_safe(account1, 30);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final balances - Account 1: " << account1.get_balance()
              << ", Account 2: " << account2.get_balance() << "\n\n";
    
    std::cout << "2. Using std::lock for multiple mutexes:\n";
    threads.clear();
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&account1, &account2, i]() {
            if (i % 2 == 0) {
                account1.transfer_with_std_lock(account2, 25);
            } else {
                account2.transfer_with_std_lock(account1, 15);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final balances - Account 1: " << account1.get_balance()
              << ", Account 2: " << account2.get_balance() << "\n\n";
}

// Reader-Writer lock demonstration
class SharedData {
private:
    mutable std::shared_mutex rw_mutex_;
    std::vector<int> data_;
    
public:
    SharedData() {
        for (int i = 0; i < 10; ++i) {
            data_.push_back(i);
        }
    }
    
    // Multiple readers can access simultaneously
    int read_data(size_t index) const {
        std::shared_lock<std::shared_mutex> lock(rw_mutex_);
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate read work
        if (index < data_.size()) {
            return data_[index];
        }
        return -1;
    }
    
    // Only one writer can access at a time
    void write_data(size_t index, int value) {
        std::unique_lock<std::shared_mutex> lock(rw_mutex_);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate write work
        if (index < data_.size()) {
            data_[index] = value;
            std::cout << "Writer updated index " << index << " to " << value << "\n";
        }
    }
    
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(rw_mutex_);
        return data_.size();
    }
};

void demonstrateReaderWriterLock() {
    std::cout << "=== Reader-Writer Lock (shared_mutex) ===\n\n";
    
    SharedData shared_data;
    std::vector<std::thread> threads;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Create multiple readers
    for (int i = 0; i < 6; ++i) {
        threads.emplace_back([&shared_data, i]() {
            int value = shared_data.read_data(i % shared_data.size());
            std::cout << "Reader " << i << " read value: " << value << "\n";
        });
    }
    
    // Create fewer writers
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([&shared_data, i]() {
            shared_data.write_data(i, 100 + i);
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Reader-writer operations completed in " << duration.count() << " ms\n\n";
}

// Recursive mutex demonstration
class RecursiveCounter {
private:
    mutable std::recursive_mutex recursive_mutex_;
    int count_ = 0;
    
public:
    void increment() {
        std::lock_guard<std::recursive_mutex> lock(recursive_mutex_);
        ++count_;
    }
    
    void increment_twice() {
        std::lock_guard<std::recursive_mutex> lock(recursive_mutex_);
        increment(); // This would deadlock with regular mutex
        increment(); // But works with recursive_mutex
    }
    
    void recursive_increment(int times) {
        if (times <= 0) return;
        
        std::lock_guard<std::recursive_mutex> lock(recursive_mutex_);
        ++count_;
        recursive_increment(times - 1); // Recursive call with same mutex
    }
    
    int get() const {
        std::lock_guard<std::recursive_mutex> lock(recursive_mutex_);
        return count_;
    }
};

void demonstrateRecursiveMutex() {
    std::cout << "=== Recursive Mutex Demonstration ===\n\n";
    
    RecursiveCounter counter;
    
    std::cout << "1. Basic increment: ";
    counter.increment();
    std::cout << counter.get() << "\n";
    
    std::cout << "2. Double increment: ";
    counter.increment_twice();
    std::cout << counter.get() << "\n";
    
    std::cout << "3. Recursive increment (5 times): ";
    counter.recursive_increment(5);
    std::cout << counter.get() << "\n\n";
}

int main() {
    try {
        std::cout << "=== MUTEX SYNCHRONIZATION CONCEPTS ===\n";
        std::cout << "This file covers mutex types and synchronization patterns\n\n";
        
        demonstrateMutexSynchronization();
        demonstrateLockTypes();
        demonstrateDeadlockPrevention();
        demonstrateReaderWriterLock();
        demonstrateRecursiveMutex();
        
        std::cout << "=== KEY CONCEPTS COVERED ===\n";
        std::cout << "1. std::mutex for basic mutual exclusion\n";
        std::cout << "2. lock_guard vs unique_lock for different use cases\n";
        std::cout << "3. Race conditions and their prevention\n";
        std::cout << "4. Deadlock scenarios and prevention techniques\n";
        std::cout << "5. std::shared_mutex for reader-writer scenarios\n";
        std::cout << "6. std::recursive_mutex for recursive locking\n";
        std::cout << "7. try_lock operations for non-blocking attempts\n";
        std::cout << "8. Performance implications of synchronization\n\n";
        
        std::cout << "=== NEXT STEPS ===\n";
        std::cout << "-> Run 03_condition_variables.cpp to learn about thread communication\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

/*
Expected Output:
================
=== MUTEX SYNCHRONIZATION CONCEPTS ===
This file covers mutex types and synchronization patterns

=== Mutex Synchronization Demonstration ===

Safe counter result: 40000 (expected: 40000)
Time with mutex: [time]ms
Unsafe counter result: [random_number] (expected: 40000)
Time without mutex: [faster_time]ms

=== Different Lock Types ===

1. Using lock_guard (RAII):
2. Using unique_lock (flexible):
3. Using try_lock operations:
   try_increment success: Yes
   try_increment_for success: Yes
   Final counter value: 4

=== Deadlock Prevention Techniques ===

Initial balances - Account 1: 1000, Account 2: 1000

1. Safe transfers using lock ordering:
Transferred 50 from account 1 to account 2
Transferred 30 from account 2 to account 1
[more transfers...]
Final balances - Account 1: [...], Account 2: [...]

2. Using std::lock for multiple mutexes:
Transferred 25 from account 1 to account 2
[more transfers...]
Final balances - Account 1: [...], Account 2: [...]

=== Reader-Writer Lock (shared_mutex) ===

Reader 0 read value: 0
Reader 1 read value: 1
[readers can run concurrently]
Writer updated index 0 to 100
Writer updated index 1 to 101
Reader-writer operations completed in [time] ms

=== Recursive Mutex Demonstration ===

1. Basic increment: 1
2. Double increment: 3
3. Recursive increment (5 times): 8

=== KEY CONCEPTS COVERED ===
1. std::mutex for basic mutual exclusion
2. lock_guard vs unique_lock for different use cases
3. Race conditions and their prevention
4. Deadlock scenarios and prevention techniques
5. std::shared_mutex for reader-writer scenarios
6. std::recursive_mutex for recursive locking
7. try_lock operations for non-blocking attempts
8. Performance implications of synchronization

=== NEXT STEPS ===
-> Run 03_condition_variables.cpp to learn about thread communication

Compilation command:
g++ -std=c++17 -Wall -Wextra -O2 -pthread 02_mutex_synchronization.cpp -o 02_mutex_synchronization

Key Learning Points:
===================
1. Always use RAII locks (lock_guard, unique_lock) to prevent deadlocks
2. Race conditions occur when multiple threads access shared data unsynchronized
3. Mutex provides mutual exclusion but has performance overhead
4. Deadlocks can be prevented with consistent lock ordering or std::lock
5. shared_mutex allows multiple readers or single writer
6. recursive_mutex allows same thread to lock multiple times
7. try_lock operations provide non-blocking alternatives
8. Lock contention can significantly impact performance
*/
