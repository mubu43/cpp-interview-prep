#include <iostream>
#include <string>
#include <cstring>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <chrono>
#include <vector>
#include <random>

// Shared data structure for demonstration
struct SharedResource {
    int counter;
    char buffer[256];
    bool data_ready;
    pid_t last_writer;
    
    SharedResource() : counter(0), data_ready(false), last_writer(0) {
        memset(buffer, 0, sizeof(buffer));
    }
};

class SemaphoreManager {
private:
    sem_t* semaphore_;
    const char* sem_name_;
    bool is_creator_;

public:
    SemaphoreManager(const char* name, unsigned int initial_value = 1, bool create = false)
        : semaphore_(nullptr), sem_name_(name), is_creator_(create) {
        
        if (create) {
            // Remove existing semaphore
            sem_unlink(sem_name_);
            
            // Create new semaphore
            semaphore_ = sem_open(sem_name_, O_CREAT | O_EXCL, 0666, initial_value);
            if (semaphore_ == SEM_FAILED) {
                perror("sem_open (create)");
                return;
            }
            std::cout << "Created semaphore: " << sem_name_ 
                      << " with initial value: " << initial_value << std::endl;
        } else {
            // Open existing semaphore
            semaphore_ = sem_open(sem_name_, 0);
            if (semaphore_ == SEM_FAILED) {
                perror("sem_open (open)");
                return;
            }
            std::cout << "Opened existing semaphore: " << sem_name_ << std::endl;
        }
    }
    
    ~SemaphoreManager() {
        cleanup();
    }
    
    bool isValid() const {
        return semaphore_ != SEM_FAILED && semaphore_ != nullptr;
    }
    
    // Wait (P operation) - acquire semaphore
    bool wait() {
        if (!isValid()) return false;
        
        if (sem_wait(semaphore_) == -1) {
            perror("sem_wait");
            return false;
        }
        return true;
    }
    
    // Try wait - non-blocking acquire
    bool tryWait() {
        if (!isValid()) return false;
        
        if (sem_trywait(semaphore_) == -1) {
            if (errno == EAGAIN) {
                return false;  // Semaphore not available
            }
            perror("sem_trywait");
            return false;
        }
        return true;
    }
    
    // Timed wait - wait with timeout
    bool timedWait(int timeout_seconds) {
        if (!isValid()) return false;
        
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += timeout_seconds;
        
        if (sem_timedwait(semaphore_, &timeout) == -1) {
            if (errno == ETIMEDOUT) {
                std::cout << "Semaphore wait timeout after " << timeout_seconds << " seconds\n";
            } else {
                perror("sem_timedwait");
            }
            return false;
        }
        return true;
    }
    
    // Post (V operation) - release semaphore
    bool post() {
        if (!isValid()) return false;
        
        if (sem_post(semaphore_) == -1) {
            perror("sem_post");
            return false;
        }
        return true;
    }
    
    // Get current semaphore value
    int getValue() {
        if (!isValid()) return -1;
        
        int value;
        if (sem_getvalue(semaphore_, &value) == -1) {
            perror("sem_getvalue");
            return -1;
        }
        return value;
    }
    
    void cleanup() {
        if (semaphore_ && semaphore_ != SEM_FAILED) {
            sem_close(semaphore_);
            semaphore_ = nullptr;
        }
        
        if (is_creator_) {
            sem_unlink(sem_name_);
            std::cout << "Cleaned up semaphore: " << sem_name_ << std::endl;
        }
    }
};

// RAII wrapper for automatic semaphore management
class SemaphoreGuard {
private:
    SemaphoreManager& sem_;
    bool acquired_;

public:
    explicit SemaphoreGuard(SemaphoreManager& sem) : sem_(sem), acquired_(false) {
        acquired_ = sem_.wait();
        if (acquired_) {
            std::cout << "Semaphore acquired by process " << getpid() << std::endl;
        }
    }
    
    ~SemaphoreGuard() {
        if (acquired_) {
            sem_.post();
            std::cout << "Semaphore released by process " << getpid() << std::endl;
        }
    }
    
    bool isAcquired() const { return acquired_; }
};

// Producer-Consumer example using semaphores
class ProducerConsumer {
private:
    SemaphoreManager* empty_slots_;   // Number of empty slots
    SemaphoreManager* filled_slots_;  // Number of filled slots
    SemaphoreManager* mutex_;         // Mutual exclusion for buffer access
    
    SharedResource* shared_buffer_;
    int buffer_size_;

public:
    ProducerConsumer(int buffer_size = 5) : buffer_size_(buffer_size) {
        // Create semaphores
        empty_slots_ = new SemaphoreManager("/empty_slots", buffer_size, true);
        filled_slots_ = new SemaphoreManager("/filled_slots", 0, true);
        mutex_ = new SemaphoreManager("/buffer_mutex", 1, true);
        
        // Create shared buffer
        int shm_fd = shm_open("/producer_consumer_buffer", O_CREAT | O_RDWR, 0666);
        ftruncate(shm_fd, sizeof(SharedResource));
        shared_buffer_ = static_cast<SharedResource*>(
            mmap(0, sizeof(SharedResource), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)
        );
        
        if (shared_buffer_ != MAP_FAILED) {
            new (shared_buffer_) SharedResource();
        }
    }
    
    ~ProducerConsumer() {
        delete empty_slots_;
        delete filled_slots_;
        delete mutex_;
        
        if (shared_buffer_ && shared_buffer_ != MAP_FAILED) {
            munmap(shared_buffer_, sizeof(SharedResource));
        }
        shm_unlink("/producer_consumer_buffer");
    }
    
    void producer(int num_items) {
        for (int i = 0; i < num_items; ++i) {
            // Wait for empty slot
            empty_slots_->wait();
            
            // Get exclusive access to buffer
            mutex_->wait();
            
            // Produce item
            shared_buffer_->counter++;
            snprintf(shared_buffer_->buffer, sizeof(shared_buffer_->buffer), 
                    "Item %d produced by PID %d", i + 1, getpid());
            shared_buffer_->data_ready = true;
            shared_buffer_->last_writer = getpid();
            
            std::cout << "Produced: " << shared_buffer_->buffer << std::endl;
            
            // Release buffer access
            mutex_->post();
            
            // Signal filled slot
            filled_slots_->post();
            
            // Simulate production time
            std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 200));
        }
    }
    
    void consumer(int num_items) {
        for (int i = 0; i < num_items; ++i) {
            // Wait for filled slot
            filled_slots_->wait();
            
            // Get exclusive access to buffer
            mutex_->wait();
            
            // Consume item
            if (shared_buffer_->data_ready) {
                std::cout << "Consumed: " << shared_buffer_->buffer 
                          << " (counter: " << shared_buffer_->counter << ")" << std::endl;
                shared_buffer_->data_ready = false;
            }
            
            // Release buffer access
            mutex_->post();
            
            // Signal empty slot
            empty_slots_->post();
            
            // Simulate consumption time
            std::this_thread::sleep_for(std::chrono::milliseconds(150 + rand() % 100));
        }
    }
};

// Readers-Writers problem using semaphores
class ReadersWriters {
private:
    SemaphoreManager* write_mutex_;    // Exclusive access for writers
    SemaphoreManager* read_count_mutex_; // Protect reader count
    SharedResource* shared_data_;
    int* reader_count_;

public:
    ReadersWriters() {
        write_mutex_ = new SemaphoreManager("/write_mutex", 1, true);
        read_count_mutex_ = new SemaphoreManager("/read_count_mutex", 1, true);
        
        // Create shared memory for data and reader count
        int shm_fd = shm_open("/readers_writers_data", O_CREAT | O_RDWR, 0666);
        ftruncate(shm_fd, sizeof(SharedResource) + sizeof(int));
        
        void* shared_mem = mmap(0, sizeof(SharedResource) + sizeof(int), 
                               PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        
        shared_data_ = static_cast<SharedResource*>(shared_mem);
        reader_count_ = reinterpret_cast<int*>(
            static_cast<char*>(shared_mem) + sizeof(SharedResource));
        
        if (shared_data_ != MAP_FAILED) {
            new (shared_data_) SharedResource();
            *reader_count_ = 0;
        }
    }
    
    ~ReadersWriters() {
        delete write_mutex_;
        delete read_count_mutex_;
        
        if (shared_data_ && shared_data_ != MAP_FAILED) {
            munmap(shared_data_, sizeof(SharedResource) + sizeof(int));
        }
        shm_unlink("/readers_writers_data");
    }
    
    void reader(int reader_id) {
        // Entry section
        read_count_mutex_->wait();
        (*reader_count_)++;
        if (*reader_count_ == 1) {
            write_mutex_->wait();  // First reader blocks writers
        }
        read_count_mutex_->post();
        
        // Critical section - reading
        std::cout << "Reader " << reader_id << " reading: counter=" 
                  << shared_data_->counter << ", buffer=" << shared_data_->buffer << std::endl;
        
        // Simulate reading time
        std::this_thread::sleep_for(std::chrono::milliseconds(200 + rand() % 300));
        
        // Exit section
        read_count_mutex_->wait();
        (*reader_count_)--;
        if (*reader_count_ == 0) {
            write_mutex_->post();  // Last reader unblocks writers
        }
        read_count_mutex_->post();
    }
    
    void writer(int writer_id) {
        // Entry section
        write_mutex_->wait();
        
        // Critical section - writing
        shared_data_->counter++;
        snprintf(shared_data_->buffer, sizeof(shared_data_->buffer), 
                "Data written by writer %d (PID %d)", writer_id, getpid());
        shared_data_->last_writer = getpid();
        
        std::cout << "Writer " << writer_id << " wrote: " << shared_data_->buffer << std::endl;
        
        // Simulate writing time
        std::this_thread::sleep_for(std::chrono::milliseconds(300 + rand() % 200));
        
        // Exit section
        write_mutex_->post();
    }
};

void demonstrateBasicSemaphore() {
    std::cout << "=== Basic Semaphore Demonstration ===\n\n";
    
    const char* sem_name = "/demo_semaphore";
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        SemaphoreManager sem(sem_name, 1, false);
        if (!sem.isValid()) {
            std::cerr << "Child: Failed to open semaphore\n";
            exit(1);
        }
        
        std::cout << "Child: Attempting to acquire semaphore...\n";
        
        // Try non-blocking first
        if (sem.tryWait()) {
            std::cout << "Child: Got semaphore immediately!\n";
        } else {
            std::cout << "Child: Semaphore busy, waiting...\n";
            if (sem.timedWait(3)) {
                std::cout << "Child: Got semaphore after waiting!\n";
            } else {
                std::cout << "Child: Timeout waiting for semaphore\n";
                exit(1);
            }
        }
        
        // Do some work in critical section
        std::cout << "Child: In critical section, working...\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        sem.post();
        std::cout << "Child: Released semaphore\n";
        
        exit(0);
        
    } else if (pid > 0) {
        // Parent process
        SemaphoreManager sem(sem_name, 1, true);
        if (!sem.isValid()) {
            std::cerr << "Parent: Failed to create semaphore\n";
            return;
        }
        
        std::cout << "Parent: Created semaphore with value: " << sem.getValue() << std::endl;
        
        // Use RAII guard for automatic management
        {
            SemaphoreGuard guard(sem);
            if (guard.isAcquired()) {
                std::cout << "Parent: In critical section, working...\n";
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
        }  // Semaphore automatically released here
        
        // Wait for child to complete
        int status;
        wait(&status);
        
        std::cout << "Parent: Child completed\n";
        std::cout << "Final semaphore value: " << sem.getValue() << std::endl;
        
    } else {
        perror("fork failed");
    }
}

void demonstrateProducerConsumer() {
    std::cout << "\n=== Producer-Consumer Demonstration ===\n\n";
    
    ProducerConsumer pc(3);  // Buffer size of 3
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process (consumer)
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        std::cout << "Consumer started\n";
        pc.consumer(8);  // Consume 8 items
        
        exit(0);
        
    } else if (pid > 0) {
        // Parent process (producer)
        std::cout << "Producer started\n";
        pc.producer(8);  // Produce 8 items
        
        // Wait for child to complete
        int status;
        wait(&status);
        
        std::cout << "Producer-Consumer demonstration completed\n";
        
    } else {
        perror("fork failed");
    }
}

void demonstrateReadersWriters() {
    std::cout << "\n=== Readers-Writers Demonstration ===\n\n";
    
    ReadersWriters rw;
    std::vector<pid_t> children;
    
    // Create multiple readers and writers
    for (int i = 0; i < 3; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            // Reader process
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * i));
            for (int j = 0; j < 3; ++j) {
                rw.reader(i + 1);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            exit(0);
        } else if (pid > 0) {
            children.push_back(pid);
        }
    }
    
    for (int i = 0; i < 2; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            // Writer process
            std::this_thread::sleep_for(std::chrono::milliseconds(200 * i));
            for (int j = 0; j < 2; ++j) {
                rw.writer(i + 1);
                std::this_thread::sleep_for(std::chrono::milliseconds(800));
            }
            exit(0);
        } else if (pid > 0) {
            children.push_back(pid);
        }
    }
    
    // Wait for all children to complete
    for (pid_t child : children) {
        int status;
        waitpid(child, &status, 0);
    }
    
    std::cout << "Readers-Writers demonstration completed\n";
}

void benchmarkSemaphore() {
    std::cout << "\n=== Semaphore Performance Benchmark ===\n";
    
    const char* sem_name = "/benchmark_sem";
    const int iterations = 100000;
    
    SemaphoreManager sem(sem_name, 1, true);
    if (!sem.isValid()) {
        std::cerr << "Failed to create semaphore for benchmark\n";
        return;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Benchmark wait/post operations
    for (int i = 0; i < iterations; ++i) {
        sem.wait();
        sem.post();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Performed " << iterations << " wait/post cycles in " 
              << duration.count() << " microseconds\n";
    std::cout << "Average time per cycle: " 
              << static_cast<double>(duration.count()) / iterations 
              << " microseconds\n";
}

int main() {
    // Seed random number generator
    srand(time(nullptr));
    
    demonstrateBasicSemaphore();
    demonstrateProducerConsumer();
    demonstrateReadersWriters();
    benchmarkSemaphore();
    
    std::cout << "\n=== Key Semaphore Concepts ===\n";
    std::cout << "1. Counting semaphore - allows N processes in critical section\n";
    std::cout << "2. Binary semaphore - acts like a mutex (0 or 1)\n";
    std::cout << "3. POSIX semaphores - named (/name) or unnamed\n";
    std::cout << "4. Operations: wait (P), post (V), try_wait, timed_wait\n";
    std::cout << "5. Perfect for producer-consumer and readers-writers problems\n";
    
    return 0;
}
