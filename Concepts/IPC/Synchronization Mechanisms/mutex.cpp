#include <iostream>
#include <string>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <mutex>

// Shared data structure for demonstration
struct SharedResource {
    pthread_mutex_t mutex;
    int counter;
    char buffer[256];
    bool data_ready;
    pid_t last_writer;
    
    SharedResource() : counter(0), data_ready(false), last_writer(0) {
        memset(buffer, 0, sizeof(buffer));
        
        // Initialize process-shared mutex
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
        pthread_mutex_init(&mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }
    
    ~SharedResource() {
        pthread_mutex_destroy(&mutex);
    }
};

class MutexManager {
private:
    pthread_mutex_t* mutex_;
    bool owns_mutex_;

public:
    // For process-shared mutex in shared memory
    MutexManager(pthread_mutex_t* shared_mutex) 
        : mutex_(shared_mutex), owns_mutex_(false) {}
    
    // For creating a new mutex
    MutexManager() : owns_mutex_(true) {
        mutex_ = new pthread_mutex_t;
        
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
        pthread_mutex_init(mutex_, &attr);
        pthread_mutexattr_destroy(&attr);
    }
    
    ~MutexManager() {
        if (owns_mutex_) {
            pthread_mutex_destroy(mutex_);
            delete mutex_;
        }
    }
    
    // Lock operations
    bool lock() {
        int result = pthread_mutex_lock(mutex_);
        if (result != 0) {
            std::cerr << "Mutex lock failed: " << strerror(result) << std::endl;
            return false;
        }
        return true;
    }
    
    bool tryLock() {
        int result = pthread_mutex_trylock(mutex_);
        if (result == EBUSY) {
            return false;  // Mutex is busy
        } else if (result != 0) {
            std::cerr << "Mutex trylock failed: " << strerror(result) << std::endl;
            return false;
        }
        return true;
    }
    
    bool timedLock(int timeout_seconds) {
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += timeout_seconds;
        
        int result = pthread_mutex_timedlock(mutex_, &timeout);
        if (result == ETIMEDOUT) {
            std::cout << "Mutex lock timeout after " << timeout_seconds << " seconds\n";
            return false;
        } else if (result != 0) {
            std::cerr << "Mutex timedlock failed: " << strerror(result) << std::endl;
            return false;
        }
        return true;
    }
    
    bool unlock() {
        int result = pthread_mutex_unlock(mutex_);
        if (result != 0) {
            std::cerr << "Mutex unlock failed: " << strerror(result) << std::endl;
            return false;
        }
        return true;
    }
    
    pthread_mutex_t* getMutex() { return mutex_; }
};

// RAII wrapper for automatic mutex management
class MutexGuard {
private:
    MutexManager& mutex_;
    bool acquired_;

public:
    explicit MutexGuard(MutexManager& mutex) : mutex_(mutex), acquired_(false) {
        acquired_ = mutex_.lock();
        if (acquired_) {
            std::cout << "Mutex acquired by process " << getpid() 
                      << " thread " << std::this_thread::get_id() << std::endl;
        }
    }
    
    ~MutexGuard() {
        if (acquired_) {
            mutex_.unlock();
            std::cout << "Mutex released by process " << getpid() 
                      << " thread " << std::this_thread::get_id() << std::endl;
        }
    }
    
    bool isAcquired() const { return acquired_; }
};

// Recursive mutex example
class RecursiveMutexExample {
private:
    pthread_mutex_t recursive_mutex_;

public:
    RecursiveMutexExample() {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&recursive_mutex_, &attr);
        pthread_mutexattr_destroy(&attr);
    }
    
    ~RecursiveMutexExample() {
        pthread_mutex_destroy(&recursive_mutex_);
    }
    
    void recursiveFunction(int depth) {
        pthread_mutex_lock(&recursive_mutex_);
        
        std::cout << "Recursive function depth: " << depth << std::endl;
        
        if (depth > 0) {
            recursiveFunction(depth - 1);  // Recursive call with same mutex
        }
        
        pthread_mutex_unlock(&recursive_mutex_);
    }
};

// Reader-Writer lock implementation using mutexes
class ReaderWriterMutex {
private:
    pthread_mutex_t read_count_mutex_;
    pthread_mutex_t write_mutex_;
    int reader_count_;

public:
    ReaderWriterMutex() : reader_count_(0) {
        pthread_mutex_init(&read_count_mutex_, nullptr);
        pthread_mutex_init(&write_mutex_, nullptr);
    }
    
    ~ReaderWriterMutex() {
        pthread_mutex_destroy(&read_count_mutex_);
        pthread_mutex_destroy(&write_mutex_);
    }
    
    void readerLock() {
        pthread_mutex_lock(&read_count_mutex_);
        reader_count_++;
        if (reader_count_ == 1) {
            pthread_mutex_lock(&write_mutex_);  // First reader blocks writers
        }
        pthread_mutex_unlock(&read_count_mutex_);
    }
    
    void readerUnlock() {
        pthread_mutex_lock(&read_count_mutex_);
        reader_count_--;
        if (reader_count_ == 0) {
            pthread_mutex_unlock(&write_mutex_);  // Last reader unblocks writers
        }
        pthread_mutex_unlock(&read_count_mutex_);
    }
    
    void writerLock() {
        pthread_mutex_lock(&write_mutex_);
    }
    
    void writerUnlock() {
        pthread_mutex_unlock(&write_mutex_);
    }
};

// Condition variable with mutex example
class ProducerConsumerCV {
private:
    pthread_mutex_t mutex_;
    pthread_cond_t not_empty_;
    pthread_cond_t not_full_;
    std::vector<int> buffer_;
    size_t max_size_;

public:
    ProducerConsumerCV(size_t max_size) : max_size_(max_size) {
        pthread_mutex_init(&mutex_, nullptr);
        pthread_cond_init(&not_empty_, nullptr);
        pthread_cond_init(&not_full_, nullptr);
    }
    
    ~ProducerConsumerCV() {
        pthread_mutex_destroy(&mutex_);
        pthread_cond_destroy(&not_empty_);
        pthread_cond_destroy(&not_full_);
    }
    
    void produce(int item) {
        pthread_mutex_lock(&mutex_);
        
        // Wait while buffer is full
        while (buffer_.size() >= max_size_) {
            pthread_cond_wait(&not_full_, &mutex_);
        }
        
        buffer_.push_back(item);
        std::cout << "Produced item: " << item << " (buffer size: " 
                  << buffer_.size() << ")" << std::endl;
        
        pthread_cond_signal(&not_empty_);  // Signal consumers
        pthread_mutex_unlock(&mutex_);
    }
    
    int consume() {
        pthread_mutex_lock(&mutex_);
        
        // Wait while buffer is empty
        while (buffer_.empty()) {
            pthread_cond_wait(&not_empty_, &mutex_);
        }
        
        int item = buffer_.back();
        buffer_.pop_back();
        std::cout << "Consumed item: " << item << " (buffer size: " 
                  << buffer_.size() << ")" << std::endl;
        
        pthread_cond_signal(&not_full_);   // Signal producers
        pthread_mutex_unlock(&mutex_);
        
        return item;
    }
};

// Priority-based mutex (simulation)
class PriorityMutex {
private:
    pthread_mutex_t mutex_;
    pthread_cond_t high_priority_cv_;
    pthread_cond_t low_priority_cv_;
    int high_priority_waiters_;
    bool locked_;

public:
    PriorityMutex() : high_priority_waiters_(0), locked_(false) {
        pthread_mutex_init(&mutex_, nullptr);
        pthread_cond_init(&high_priority_cv_, nullptr);
        pthread_cond_init(&low_priority_cv_, nullptr);
    }
    
    ~PriorityMutex() {
        pthread_mutex_destroy(&mutex_);
        pthread_cond_destroy(&high_priority_cv_);
        pthread_cond_destroy(&low_priority_cv_);
    }
    
    void lockHighPriority() {
        pthread_mutex_lock(&mutex_);
        
        high_priority_waiters_++;
        while (locked_) {
            pthread_cond_wait(&high_priority_cv_, &mutex_);
        }
        high_priority_waiters_--;
        
        locked_ = true;
        pthread_mutex_unlock(&mutex_);
    }
    
    void lockLowPriority() {
        pthread_mutex_lock(&mutex_);
        
        // Wait for both unlock and no high priority waiters
        while (locked_ || high_priority_waiters_ > 0) {
            pthread_cond_wait(&low_priority_cv_, &mutex_);
        }
        
        locked_ = true;
        pthread_mutex_unlock(&mutex_);
    }
    
    void unlock() {
        pthread_mutex_lock(&mutex_);
        locked_ = false;
        
        if (high_priority_waiters_ > 0) {
            pthread_cond_signal(&high_priority_cv_);
        } else {
            pthread_cond_signal(&low_priority_cv_);
        }
        
        pthread_mutex_unlock(&mutex_);
    }
};

void demonstrateBasicMutex() {
    std::cout << "=== Basic Mutex Demonstration ===\n\n";
    
    // Create shared memory
    int shm_fd = shm_open("/demo_mutex_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(SharedResource));
    
    SharedResource* shared_data = static_cast<SharedResource*>(
        mmap(0, sizeof(SharedResource), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)
    );
    
    if (shared_data == MAP_FAILED) {
        perror("mmap");
        return;
    }
    
    new (shared_data) SharedResource();
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        MutexManager mutex(&shared_data->mutex);
        
        std::cout << "Child: Attempting to acquire mutex...\n";
        
        // Try non-blocking first
        if (mutex.tryLock()) {
            std::cout << "Child: Got mutex immediately!\n";
        } else {
            std::cout << "Child: Mutex busy, waiting with timeout...\n";
            if (mutex.timedLock(3)) {
                std::cout << "Child: Got mutex after waiting!\n";
            } else {
                std::cout << "Child: Timeout waiting for mutex\n";
                exit(1);
            }
        }
        
        // Critical section
        shared_data->counter++;
        snprintf(shared_data->buffer, sizeof(shared_data->buffer), 
                "Data from child process %d", getpid());
        shared_data->last_writer = getpid();
        shared_data->data_ready = true;
        
        std::cout << "Child: In critical section, counter=" << shared_data->counter << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        mutex.unlock();
        std::cout << "Child: Released mutex\n";
        
        exit(0);
        
    } else if (pid > 0) {
        // Parent process
        MutexManager mutex(&shared_data->mutex);
        
        std::cout << "Parent: Acquiring mutex...\n";
        
        // Use RAII guard for automatic management
        {
            MutexGuard guard(mutex);
            if (guard.isAcquired()) {
                shared_data->counter++;
                snprintf(shared_data->buffer, sizeof(shared_data->buffer), 
                        "Data from parent process %d", getpid());
                shared_data->last_writer = getpid();
                
                std::cout << "Parent: In critical section, counter=" << shared_data->counter << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
        }  // Mutex automatically released here
        
        // Wait for child to complete
        int status;
        wait(&status);
        
        std::cout << "Parent: Final counter value: " << shared_data->counter << std::endl;
        std::cout << "Parent: Final buffer: " << shared_data->buffer << std::endl;
        
    } else {
        perror("fork failed");
    }
    
    // Cleanup
    munmap(shared_data, sizeof(SharedResource));
    shm_unlink("/demo_mutex_shm");
}

void demonstrateRecursiveMutex() {
    std::cout << "\n=== Recursive Mutex Demonstration ===\n\n";
    
    RecursiveMutexExample example;
    
    std::cout << "Calling recursive function with depth 3:\n";
    example.recursiveFunction(3);
    
    std::cout << "Recursive mutex demonstration completed\n";
}

void demonstrateReaderWriter() {
    std::cout << "\n=== Reader-Writer Mutex Demonstration ===\n\n";
    
    ReaderWriterMutex rw_mutex;
    int shared_data = 0;
    std::vector<std::thread> threads;
    
    // Create reader threads
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&rw_mutex, &shared_data, i]() {
            for (int j = 0; j < 3; ++j) {
                rw_mutex.readerLock();
                
                std::cout << "Reader " << i << " reading: " << shared_data << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                
                rw_mutex.readerUnlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        });
    }
    
    // Create writer threads
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([&rw_mutex, &shared_data, i]() {
            for (int j = 0; j < 2; ++j) {
                rw_mutex.writerLock();
                
                shared_data++;
                std::cout << "Writer " << i << " wrote: " << shared_data << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(400));
                
                rw_mutex.writerUnlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "Reader-Writer demonstration completed\n";
}

void demonstrateConditionVariable() {
    std::cout << "\n=== Condition Variable with Mutex Demonstration ===\n\n";
    
    ProducerConsumerCV pc(5);  // Buffer size of 5
    std::vector<std::thread> threads;
    
    // Producer threads
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([&pc, i]() {
            for (int j = 0; j < 5; ++j) {
                pc.produce(i * 10 + j);
                std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 200));
            }
        });
    }
    
    // Consumer threads
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([&pc]() {
            for (int j = 0; j < 5; ++j) {
                pc.consume();
                std::this_thread::sleep_for(std::chrono::milliseconds(150 + rand() % 100));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "Condition variable demonstration completed\n";
}

void demonstratePriorityMutex() {
    std::cout << "\n=== Priority Mutex Demonstration ===\n\n";
    
    PriorityMutex priority_mutex;
    std::vector<std::thread> threads;
    
    // Low priority threads
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&priority_mutex, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50 * i));
            
            std::cout << "Low priority thread " << i << " requesting lock\n";
            priority_mutex.lockLowPriority();
            
            std::cout << "Low priority thread " << i << " got lock\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            
            priority_mutex.unlock();
            std::cout << "Low priority thread " << i << " released lock\n";
        });
    }
    
    // High priority thread (starts later but should get priority)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    threads.emplace_back([&priority_mutex]() {
        std::cout << "High priority thread requesting lock\n";
        priority_mutex.lockHighPriority();
        
        std::cout << "High priority thread got lock\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        priority_mutex.unlock();
        std::cout << "High priority thread released lock\n";
    });
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "Priority mutex demonstration completed\n";
}

void benchmarkMutex() {
    std::cout << "\n=== Mutex Performance Benchmark ===\n";
    
    const int iterations = 1000000;
    MutexManager mutex;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Benchmark lock/unlock operations
    for (int i = 0; i < iterations; ++i) {
        mutex.lock();
        mutex.unlock();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    std::cout << "Performed " << iterations << " lock/unlock cycles in " 
              << duration.count() << " nanoseconds\n";
    std::cout << "Average time per cycle: " 
              << static_cast<double>(duration.count()) / iterations 
              << " nanoseconds\n";
    
    // Benchmark with contention
    std::cout << "\nBenchmarking with contention...\n";
    
    std::atomic<int> counter{0};
    const int num_threads = 4;
    const int ops_per_thread = iterations / num_threads;
    
    start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&mutex, &counter, ops_per_thread]() {
            for (int j = 0; j < ops_per_thread; ++j) {
                mutex.lock();
                counter++;
                mutex.unlock();
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    std::cout << "Contended operations: " << counter.load() << " in " 
              << duration.count() << " nanoseconds\n";
    std::cout << "Average time per contended operation: " 
              << static_cast<double>(duration.count()) / counter.load() 
              << " nanoseconds\n";
}

int main() {
    // Seed random number generator
    srand(time(nullptr));
    
    demonstrateBasicMutex();
    demonstrateRecursiveMutex();
    demonstrateReaderWriter();
    demonstrateConditionVariable();
    demonstratePriorityMutex();
    benchmarkMutex();
    
    std::cout << "\n=== Key Mutex Concepts ===\n";
    std::cout << "1. Mutual exclusion - only one thread/process at a time\n";
    std::cout << "2. Ownership - thread that locks must unlock\n";
    std::cout << "3. Types: normal, recursive, error-checking\n";
    std::cout << "4. Process-shared mutexes for IPC synchronization\n";
    std::cout << "5. Always use RAII for exception safety\n";
    std::cout << "6. Condition variables for complex synchronization\n";
    
    return 0;
}
