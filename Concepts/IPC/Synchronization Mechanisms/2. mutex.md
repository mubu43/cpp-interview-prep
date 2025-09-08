# Mutex - Synchronization Mechanism

## Overview
Mutex (Mutual Exclusion) is a synchronization primitive that ensures only one thread or process can access a shared resource at a time. It provides ownership semantics where the thread that locks the mutex must be the one to unlock it.

## Intent
- **Provide mutual exclusion for shared resources**
- **Ensure thread-safe access to critical sections**
- **Implement ownership-based synchronization**
- **Support both thread and process synchronization**

## Problem It Solves
- Race conditions when multiple threads access shared data
- Data corruption from concurrent modifications
- Need for atomic operations on complex data structures
- Coordination between threads and processes
- Protection of critical sections in concurrent programs

## Key Concepts

### 1. Mutual Exclusion
- **Only one thread can hold the mutex at a time**
- **Other threads block until mutex is released**
- **Provides serialized access to shared resources**

### 2. Ownership Semantics
- **Thread that locks mutex must unlock it**
- **Non-transferable ownership**
- **Error checking can detect ownership violations**

### 3. Critical Sections
- **Code regions that must execute atomically**
- **Protected by mutex lock/unlock pairs**
- **Should be kept as small as possible**

## POSIX Mutex Types

### 1. Normal Mutex (PTHREAD_MUTEX_NORMAL)
```cpp
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Basic usage
pthread_mutex_lock(&mutex);
// Critical section
pthread_mutex_unlock(&mutex);
```

### 2. Error-Checking Mutex (PTHREAD_MUTEX_ERRORCHECK)
```cpp
pthread_mutex_t mutex;
pthread_mutexattr_t attr;

pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
pthread_mutex_init(&mutex, &attr);

// Detects errors like double-locking
if (pthread_mutex_lock(&mutex) != 0) {
    // Handle error
}
```

### 3. Recursive Mutex (PTHREAD_MUTEX_RECURSIVE)
```cpp
pthread_mutex_t recursive_mutex;
pthread_mutexattr_t attr;

pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
pthread_mutex_init(&recursive_mutex, &attr);

// Can be locked multiple times by same thread
void recursive_function(int depth) {
    pthread_mutex_lock(&recursive_mutex);
    if (depth > 0) {
        recursive_function(depth - 1);  // Same thread can lock again
    }
    pthread_mutex_unlock(&recursive_mutex);
}
```

## Process-Shared Mutexes

### Shared Memory Mutex
```cpp
// Place mutex in shared memory
struct SharedData {
    pthread_mutex_t mutex;
    int data;
};

// Initialize process-shared mutex
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
pthread_mutex_init(&shared_data->mutex, &attr);

// Use from different processes
pthread_mutex_lock(&shared_data->mutex);
shared_data->data++;
pthread_mutex_unlock(&shared_data->mutex);
```

### Robust Mutexes
```cpp
// Handle process crashes
pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
pthread_mutex_init(&mutex, &attr);

// Check for abandoned mutex
int result = pthread_mutex_lock(&mutex);
if (result == EOWNERDEAD) {
    // Previous owner died, recover state
    recover_shared_state();
    pthread_mutex_consistent(&mutex);
}
```

## Mutex Operations

### Basic Operations
```cpp
// Blocking lock
int pthread_mutex_lock(pthread_mutex_t* mutex);

// Non-blocking lock attempt
int pthread_mutex_trylock(pthread_mutex_t* mutex);

// Timed lock with timeout
int pthread_mutex_timedlock(pthread_mutex_t* mutex, 
                           const struct timespec* timeout);

// Unlock
int pthread_mutex_unlock(pthread_mutex_t* mutex);
```

### Error Handling
```cpp
int result = pthread_mutex_lock(&mutex);
switch (result) {
    case 0:
        // Success
        break;
    case EINVAL:
        // Invalid mutex
        break;
    case EDEADLK:
        // Deadlock detected
        break;
    case EOWNERDEAD:
        // Owner died (robust mutex)
        break;
    default:
        // Other error
        break;
}
```

## RAII and Exception Safety

### C++ Mutex Wrapper
```cpp
class MutexRAII {
    pthread_mutex_t* mutex_;
    bool acquired_;

public:
    explicit MutexRAII(pthread_mutex_t* mutex) 
        : mutex_(mutex), acquired_(false) {
        if (pthread_mutex_lock(mutex_) == 0) {
            acquired_ = true;
        }
    }
    
    ~MutexRAII() {
        if (acquired_) {
            pthread_mutex_unlock(mutex_);
        }
    }
    
    bool isAcquired() const { return acquired_; }
};

// Usage
{
    MutexRAII guard(&mutex);
    if (guard.isAcquired()) {
        // Critical section
        // Mutex automatically released when guard goes out of scope
    }
}
```

### std::mutex (C++11)
```cpp
#include <mutex>

std::mutex mtx;

// Basic usage
mtx.lock();
// Critical section
mtx.unlock();

// RAII with lock_guard
{
    std::lock_guard<std::mutex> lock(mtx);
    // Critical section
    // Automatically unlocked when lock goes out of scope
}

// Flexible locking with unique_lock
{
    std::unique_lock<std::mutex> lock(mtx);
    // Can unlock and relock
    lock.unlock();
    // Do something without mutex
    lock.lock();
    // Critical section continues
}
```

## Advanced Patterns

### 1. Reader-Writer Lock Simulation
```cpp
class ReaderWriterMutex {
    pthread_mutex_t read_count_mutex_;
    pthread_mutex_t write_mutex_;
    int reader_count_;

public:
    void reader_lock() {
        pthread_mutex_lock(&read_count_mutex_);
        reader_count_++;
        if (reader_count_ == 1) {
            pthread_mutex_lock(&write_mutex_);  // Block writers
        }
        pthread_mutex_unlock(&read_count_mutex_);
    }
    
    void reader_unlock() {
        pthread_mutex_lock(&read_count_mutex_);
        reader_count_--;
        if (reader_count_ == 0) {
            pthread_mutex_unlock(&write_mutex_);  // Allow writers
        }
        pthread_mutex_unlock(&read_count_mutex_);
    }
    
    void writer_lock() {
        pthread_mutex_lock(&write_mutex_);
    }
    
    void writer_unlock() {
        pthread_mutex_unlock(&write_mutex_);
    }
};
```

### 2. Priority Inheritance
```cpp
// Set priority inheritance to avoid priority inversion
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
pthread_mutex_init(&mutex, &attr);
```

### 3. Condition Variables with Mutex
```cpp
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
bool ready = false;

// Waiting thread
pthread_mutex_lock(&mutex);
while (!ready) {
    pthread_cond_wait(&condition, &mutex);  // Atomically unlocks and waits
}
// Condition is true, mutex is locked
process_data();
pthread_mutex_unlock(&mutex);

// Signaling thread
pthread_mutex_lock(&mutex);
ready = true;
pthread_cond_signal(&condition);  // Wake up waiting thread
pthread_mutex_unlock(&mutex);
```

### 4. Multiple Mutex Ordering
```cpp
// Always acquire mutexes in same order to prevent deadlock
void transfer(Account& from, Account& to, double amount) {
    pthread_mutex_t* first = &from.mutex;
    pthread_mutex_t* second = &to.mutex;
    
    // Order by address to ensure consistent ordering
    if (first > second) {
        std::swap(first, second);
    }
    
    pthread_mutex_lock(first);
    pthread_mutex_lock(second);
    
    from.balance -= amount;
    to.balance += amount;
    
    pthread_mutex_unlock(second);
    pthread_mutex_unlock(first);
}
```

## Performance Considerations

### Factors Affecting Performance
- **Contention Level**: High contention increases context switches
- **Critical Section Size**: Larger sections increase wait times
- **Lock Granularity**: Fine-grained vs coarse-grained locking
- **Memory Hierarchy**: Cache line sharing and false sharing

### Optimization Strategies
```cpp
// Minimize critical section size
void optimized_function() {
    // Prepare data outside critical section
    Data prepared_data = prepare_data();
    
    pthread_mutex_lock(&mutex);
    // Minimal critical section
    shared_resource = prepared_data;
    pthread_mutex_unlock(&mutex);
    
    // Post-processing outside critical section
    post_process();
}

// Use try_lock to avoid blocking
bool try_update(int new_value) {
    if (pthread_mutex_trylock(&mutex) == 0) {
        shared_value = new_value;
        pthread_mutex_unlock(&mutex);
        return true;
    }
    return false;  // Couldn't acquire lock
}

// Batch operations to reduce lock overhead
void batch_update(const std::vector<int>& values) {
    pthread_mutex_lock(&mutex);
    for (int value : values) {
        shared_container.push_back(value);
    }
    pthread_mutex_unlock(&mutex);
}
```

## Real-World Applications

### 1. Thread-Safe Data Structures
```cpp
template<typename T>
class ThreadSafeQueue {
    std::queue<T> queue_;
    mutable std::mutex mutex_;

public:
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
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
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};
```

### 2. Singleton with Mutex
```cpp
class ThreadSafeSingleton {
    static ThreadSafeSingleton* instance_;
    static std::mutex mutex_;
    
    ThreadSafeSingleton() = default;

public:
    static ThreadSafeSingleton* getInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = new ThreadSafeSingleton();
        }
        return instance_;
    }
};

ThreadSafeSingleton* ThreadSafeSingleton::instance_ = nullptr;
std::mutex ThreadSafeSingleton::mutex_;
```

### 3. Reference Counting
```cpp
class RefCounted {
    mutable std::mutex mutex_;
    mutable int ref_count_;

public:
    RefCounted() : ref_count_(1) {}
    
    void addRef() const {
        std::lock_guard<std::mutex> lock(mutex_);
        ++ref_count_;
    }
    
    void release() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (--ref_count_ == 0) {
            delete this;
        }
    }
};
```

## Common Pitfalls and Solutions

### 1. Deadlock Prevention
```cpp
// WRONG: Inconsistent ordering can cause deadlock
void bad_transfer(Account& a, Account& b) {
    a.lock();
    b.lock();  // Potential deadlock if another thread locks b then a
    // Transfer logic
    b.unlock();
    a.unlock();
}

// CORRECT: Consistent ordering prevents deadlock
void good_transfer(Account& a, Account& b) {
    if (&a < &b) {
        a.lock();
        b.lock();
    } else {
        b.lock();
        a.lock();
    }
    // Transfer logic
    if (&a < &b) {
        b.unlock();
        a.unlock();
    } else {
        a.unlock();
        b.unlock();
    }
}
```

### 2. Exception Safety
```cpp
// WRONG: Exception can cause mutex to remain locked
void unsafe_function() {
    pthread_mutex_lock(&mutex);
    risky_operation();  // May throw exception
    pthread_mutex_unlock(&mutex);  // Never reached if exception thrown
}

// CORRECT: RAII ensures unlock even with exceptions
void safe_function() {
    std::lock_guard<std::mutex> lock(mutex);
    risky_operation();  // Exception safe - lock released automatically
}
```

### 3. Forgetting to Unlock
```cpp
// WRONG: Early return forgets to unlock
int bad_function(int value) {
    pthread_mutex_lock(&mutex);
    if (value < 0) {
        return -1;  // Mutex remains locked!
    }
    process_value(value);
    pthread_mutex_unlock(&mutex);
    return 0;
}

// CORRECT: All paths unlock mutex
int good_function(int value) {
    pthread_mutex_lock(&mutex);
    int result = 0;
    
    if (value >= 0) {
        process_value(value);
    } else {
        result = -1;
    }
    
    pthread_mutex_unlock(&mutex);
    return result;
}
```

## Advantages
1. **Ownership Semantics**: Clear responsibility for unlocking
2. **Simple Concept**: Easy to understand mutual exclusion
3. **Cross-Platform**: Available on all major platforms
4. **Efficient**: Low overhead for uncontended cases
5. **Flexible**: Various types for different use cases

## Disadvantages
1. **Blocking**: Threads wait indefinitely for lock
2. **No Fairness**: No guarantee of acquisition order
3. **Priority Inversion**: Low priority threads can block high priority
4. **Deadlock Prone**: Potential for circular dependencies
5. **Context Switch Overhead**: Can be expensive under contention

## Best Practices

### 1. Design Guidelines
- **Keep critical sections small**: Minimize lock hold time
- **Use RAII**: Automatic resource management
- **Consistent ordering**: Prevent deadlocks with multiple mutexes
- **Avoid recursive locks**: Prefer restructuring code

### 2. Performance Optimization
- **Profile lock contention**: Identify bottlenecks
- **Consider lock-free alternatives**: For high-performance scenarios
- **Use appropriate granularity**: Balance contention vs overhead
- **Minimize shared state**: Reduce need for synchronization

### 3. Error Handling
- **Check return values**: Handle mutex operation failures
- **Use timeout versions**: Avoid indefinite blocking
- **Handle robust mutex states**: Deal with process crashes
- **Implement fallback strategies**: Graceful degradation

### 4. Debugging and Testing
- **Use thread sanitizers**: Detect race conditions and deadlocks
- **Log lock operations**: Aid in debugging
- **Test under load**: Verify behavior under contention
- **Use static analysis**: Catch potential issues early

## Interview Questions & Answers

### Q: What's the difference between mutex and semaphore?
**A:** Mutex provides mutual exclusion with ownership (only the locking thread can unlock), while semaphores are counting mechanisms without ownership. Mutex is binary (locked/unlocked), semaphores can count multiple resources.

### Q: How do you prevent deadlock with multiple mutexes?
**A:** Use consistent ordering (e.g., by memory address), implement timeout mechanisms, use lock hierarchies, or design to avoid needing multiple locks simultaneously.

### Q: What is priority inversion and how do you solve it?
**A:** Priority inversion occurs when a high-priority thread is blocked by a low-priority thread holding a mutex. Solutions include priority inheritance, priority ceiling protocols, or avoiding shared resources between different priority levels.

### Q: When would you use a recursive mutex?
**A:** When the same thread needs to acquire the same mutex multiple times, typically in recursive functions or when a function that locks a mutex calls another function that also needs the same mutex.

### Q: How do condition variables work with mutexes?
**A:** Condition variables allow threads to wait for specific conditions. They work with mutexes by atomically releasing the mutex and waiting, then reacquiring the mutex when signaled.

## Modern C++ Considerations
- Use `std::mutex` and related classes for type safety
- Leverage `std::lock_guard` and `std::unique_lock` for RAII
- Consider `std::shared_mutex` for reader-writer scenarios (C++17)
- Use `std::scoped_lock` for multiple mutex acquisition (C++17)
- Consider atomic operations for simple synchronization needs

## Alternative Synchronization Mechanisms
- **std::atomic**: For lock-free synchronization
- **std::shared_mutex**: For reader-writer patterns
- **std::condition_variable**: For complex waiting conditions
- **std::barrier**: For synchronization points (C++20)
- **Spinlocks**: For very short critical sections
