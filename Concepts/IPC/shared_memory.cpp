#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <chrono>
#include <thread>
#include <vector>
#include <memory>

// Shared data structure
struct SharedData {
    int counter;
    char message[256];
    bool ready;
    pid_t writer_pid;
    
    SharedData() : counter(0), ready(false), writer_pid(0) {
        memset(message, 0, sizeof(message));
    }
};

class SharedMemoryManager {
private:
    const char* shm_name_;
    const char* sem_name_;
    int shm_fd_;
    SharedData* shared_data_;
    sem_t* semaphore_;
    size_t size_;
    bool is_creator_;

public:
    SharedMemoryManager(const char* shm_name, const char* sem_name, bool create = false) 
        : shm_name_(shm_name), sem_name_(sem_name), shm_fd_(-1), 
          shared_data_(nullptr), semaphore_(nullptr), size_(sizeof(SharedData)), 
          is_creator_(create) {}
    
    ~SharedMemoryManager() {
        cleanup();
    }
    
    bool initialize() {
        if (is_creator_) {
            return createSharedMemory();
        } else {
            return attachToSharedMemory();
        }
    }
    
private:
    bool createSharedMemory() {
        // Remove existing shared memory if it exists
        shm_unlink(shm_name_);
        sem_unlink(sem_name_);
        
        // Create shared memory object
        shm_fd_ = shm_open(shm_name_, O_CREAT | O_RDWR, 0666);
        if (shm_fd_ == -1) {
            perror("shm_open (create)");
            return false;
        }
        
        // Set the size of shared memory
        if (ftruncate(shm_fd_, size_) == -1) {
            perror("ftruncate");
            return false;
        }
        
        // Map shared memory
        shared_data_ = static_cast<SharedData*>(
            mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0)
        );
        
        if (shared_data_ == MAP_FAILED) {
            perror("mmap (create)");
            return false;
        }
        
        // Initialize the shared data
        new (shared_data_) SharedData();
        
        // Create semaphore for synchronization
        semaphore_ = sem_open(sem_name_, O_CREAT, 0666, 1);
        if (semaphore_ == SEM_FAILED) {
            perror("sem_open (create)");
            return false;
        }
        
        std::cout << "Shared memory created successfully\n";
        return true;
    }
    
    bool attachToSharedMemory() {
        // Wait a bit for creator to initialize
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Open existing shared memory
        shm_fd_ = shm_open(shm_name_, O_RDWR, 0666);
        if (shm_fd_ == -1) {
            perror("shm_open (attach)");
            return false;
        }
        
        // Map shared memory
        shared_data_ = static_cast<SharedData*>(
            mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0)
        );
        
        if (shared_data_ == MAP_FAILED) {
            perror("mmap (attach)");
            return false;
        }
        
        // Open existing semaphore
        semaphore_ = sem_open(sem_name_, 0);
        if (semaphore_ == SEM_FAILED) {
            perror("sem_open (attach)");
            return false;
        }
        
        std::cout << "Attached to shared memory successfully\n";
        return true;
    }
    
public:
    void writeData(const std::string& msg, int value) {
        if (!shared_data_ || !semaphore_) {
            std::cerr << "Shared memory not initialized\n";
            return;
        }
        
        // Wait for semaphore (lock)
        sem_wait(semaphore_);
        
        // Critical section
        shared_data_->counter = value;
        strncpy(shared_data_->message, msg.c_str(), sizeof(shared_data_->message) - 1);
        shared_data_->writer_pid = getpid();
        shared_data_->ready = true;
        
        std::cout << "Written to shared memory: counter=" << value 
                  << ", message=" << msg << std::endl;
        
        // Release semaphore (unlock)
        sem_post(semaphore_);
    }
    
    bool readData(std::string& msg, int& value, pid_t& writer_pid) {
        if (!shared_data_ || !semaphore_) {
            std::cerr << "Shared memory not initialized\n";
            return false;
        }
        
        // Wait for semaphore (lock)
        sem_wait(semaphore_);
        
        // Critical section
        if (shared_data_->ready) {
            value = shared_data_->counter;
            msg = std::string(shared_data_->message);
            writer_pid = shared_data_->writer_pid;
            
            std::cout << "Read from shared memory: counter=" << value 
                      << ", message=" << msg 
                      << ", writer_pid=" << writer_pid << std::endl;
            
            // Release semaphore (unlock)
            sem_post(semaphore_);
            return true;
        }
        
        // Release semaphore (unlock)
        sem_post(semaphore_);
        return false;
    }
    
    void cleanup() {
        if (shared_data_ && shared_data_ != MAP_FAILED) {
            munmap(shared_data_, size_);
            shared_data_ = nullptr;
        }
        
        if (shm_fd_ != -1) {
            close(shm_fd_);
            shm_fd_ = -1;
        }
        
        if (semaphore_ && semaphore_ != SEM_FAILED) {
            sem_close(semaphore_);
            semaphore_ = nullptr;
        }
        
        if (is_creator_) {
            shm_unlink(shm_name_);
            sem_unlink(sem_name_);
            std::cout << "Cleaned up shared memory resources\n";
        }
    }
    
    void printInfo() {
        if (shared_data_) {
            sem_wait(semaphore_);
            std::cout << "Shared Memory Info:\n";
            std::cout << "  Counter: " << shared_data_->counter << "\n";
            std::cout << "  Message: " << shared_data_->message << "\n";
            std::cout << "  Ready: " << (shared_data_->ready ? "Yes" : "No") << "\n";
            std::cout << "  Writer PID: " << shared_data_->writer_pid << "\n";
            sem_post(semaphore_);
        }
    }
};

// Simple shared memory example using Boost.Interprocess (alternative approach)
#ifdef USE_BOOST_INTERPROCESS
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

class BoostSharedMemory {
private:
    boost::interprocess::shared_memory_object shm_;
    boost::interprocess::mapped_region region_;
    SharedData* data_;
    bool is_creator_;

public:
    BoostSharedMemory(const char* name, bool create) : is_creator_(create) {
        if (create) {
            boost::interprocess::shared_memory_object::remove(name);
            shm_ = boost::interprocess::shared_memory_object(
                boost::interprocess::create_only, name, boost::interprocess::read_write);
            shm_.truncate(sizeof(SharedData));
        } else {
            shm_ = boost::interprocess::shared_memory_object(
                boost::interprocess::open_only, name, boost::interprocess::read_write);
        }
        
        region_ = boost::interprocess::mapped_region(shm_, boost::interprocess::read_write);
        data_ = static_cast<SharedData*>(region_.get_address());
        
        if (create) {
            new (data_) SharedData();
        }
    }
    
    SharedData* getData() { return data_; }
};
#endif

void demonstrateSharedMemory() {
    std::cout << "=== Shared Memory IPC Demonstration ===\n\n";
    
    const char* shm_name = "/demo_shared_memory";
    const char* sem_name = "/demo_semaphore";
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process (reader)
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        SharedMemoryManager reader(shm_name, sem_name, false);
        if (!reader.initialize()) {
            std::cerr << "Child: Failed to initialize shared memory\n";
            exit(1);
        }
        
        std::cout << "\nChild process reading from shared memory:\n";
        
        // Read multiple times
        for (int i = 0; i < 5; ++i) {
            std::string msg;
            int value;
            pid_t writer_pid;
            
            if (reader.readData(msg, value, writer_pid)) {
                std::cout << "Child read iteration " << i + 1 << " successful\n";
            } else {
                std::cout << "Child: No data ready yet\n";
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        reader.printInfo();
        exit(0);
        
    } else if (pid > 0) {
        // Parent process (writer)
        SharedMemoryManager writer(shm_name, sem_name, true);
        if (!writer.initialize()) {
            std::cerr << "Parent: Failed to initialize shared memory\n";
            return;
        }
        
        std::cout << "Parent process writing to shared memory:\n";
        
        // Write data multiple times
        std::vector<std::string> messages = {
            "Hello from parent!",
            "Shared memory is fast",
            "Zero-copy data sharing",
            "IPC demonstration",
            "Final message"
        };
        
        for (size_t i = 0; i < messages.size(); ++i) {
            writer.writeData(messages[i], static_cast<int>(i + 1));
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        
        // Wait for child to complete
        int status;
        wait(&status);
        
        std::cout << "\nChild process completed\n";
        writer.printInfo();
        
    } else {
        perror("fork failed");
    }
}

// Performance benchmark
void benchmarkSharedMemory() {
    std::cout << "\n=== Shared Memory Performance Benchmark ===\n";
    
    const char* shm_name = "/benchmark_shm";
    const char* sem_name = "/benchmark_sem";
    const int iterations = 100000;
    
    SharedMemoryManager manager(shm_name, sem_name, true);
    if (!manager.initialize()) {
        std::cerr << "Failed to initialize shared memory for benchmark\n";
        return;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        manager.writeData("Benchmark message " + std::to_string(i), i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Performed " << iterations << " writes in " 
              << duration.count() << " microseconds\n";
    std::cout << "Average time per write: " 
              << static_cast<double>(duration.count()) / iterations 
              << " microseconds\n";
}

int main() {
    demonstrateSharedMemory();
    benchmarkSharedMemory();
    
    std::cout << "\n=== Key Shared Memory Concepts ===\n";
    std::cout << "1. Fastest IPC method - zero-copy data sharing\n";
    std::cout << "2. Requires synchronization (semaphores, mutexes)\n";
    std::cout << "3. Memory persists until explicitly removed\n";
    std::cout << "4. Best for high-throughput, low-latency scenarios\n";
    std::cout << "5. Can be dangerous - direct memory access\n";
    
    return 0;
}
