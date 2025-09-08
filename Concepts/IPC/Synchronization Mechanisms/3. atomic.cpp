#include <iostream>
#include <string>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <memory>
#include <queue>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

// Shared atomic data structure for inter-process communication
struct SharedAtomicData {
    std::atomic<int> counter{0};
    std::atomic<bool> ready{false};
    std::atomic<long> timestamp{0};
    std::atomic<pid_t> last_writer{0};
    
    // Atomic pointer for shared string (using offset in shared memory)
    std::atomic<size_t> message_offset{0};
    char message_buffer[256];
    
    SharedAtomicData() {
        memset(message_buffer, 0, sizeof(message_buffer));
    }
};

// Lock-free stack implementation
template<typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        std::atomic<Node*> next;
        
        Node(const T& item) : data(item), next(nullptr) {}
    };
    
    std::atomic<Node*> head_{nullptr};

public:
    ~LockFreeStack() {
        while (Node* old_head = head_.load()) {
            head_ = old_head->next;
            delete old_head;
        }
    }
    
    void push(const T& item) {
        Node* new_node = new Node(item);
        new_node->next = head_.load();
        
        // Compare-and-swap loop
        while (!head_.compare_exchange_weak(new_node->next, new_node)) {
            // Loop until successful
        }
    }
    
    bool pop(T& result) {
        Node* old_head = head_.load();
        
        while (old_head && !head_.compare_exchange_weak(old_head, old_head->next.load())) {
            // Keep trying until successful or empty
        }
        
        if (old_head) {
            result = old_head->data;
            delete old_head;
            return true;
        }
        
        return false;  // Stack was empty
    }
    
    bool empty() const {
        return head_.load() == nullptr;
    }
};

// Lock-free queue implementation (simplified)
template<typename T>
class LockFreeQueue {
private:
    struct Node {
        std::atomic<T*> data{nullptr};
        std::atomic<Node*> next{nullptr};
    };
    
    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;

public:
    LockFreeQueue() {
        Node* dummy = new Node;
        head_.store(dummy);
        tail_.store(dummy);
    }
    
    ~LockFreeQueue() {
        while (Node* old_head = head_.load()) {
            head_.store(old_head->next);
            delete old_head;
        }
    }
    
    void enqueue(const T& item) {
        Node* new_node = new Node;
        T* data = new T(std::move(item));
        new_node->data.store(data);
        
        while (true) {
            Node* last = tail_.load();
            Node* next = last->next.load();
            
            if (last == tail_.load()) {
                if (next == nullptr) {
                    if (last->next.compare_exchange_weak(next, new_node)) {
                        break;  // Successfully linked new node
                    }
                } else {
                    // Help advance tail
                    tail_.compare_exchange_weak(last, next);
                }
            }
        }
        
        // Advance tail to new node
        tail_.compare_exchange_weak(tail_.load(), new_node);
    }
    
    bool dequeue(T& result) {
        while (true) {
            Node* first = head_.load();
            Node* last = tail_.load();
            Node* next = first->next.load();
            
            if (first == head_.load()) {
                if (first == last) {
                    if (next == nullptr) {
                        return false;  // Queue is empty
                    }
                    // Help advance tail
                    tail_.compare_exchange_weak(last, next);
                } else {
                    if (next == nullptr) {
                        continue;  // Try again
                    }
                    
                    T* data = next->data.load();
                    if (data == nullptr) {
                        continue;  // Try again
                    }
                    
                    if (head_.compare_exchange_weak(first, next)) {
                        result = *data;
                        delete data;
                        delete first;
                        return true;
                    }
                }
            }
        }
    }
};

// Atomic reference counting
class AtomicRefCounted {
private:
    mutable std::atomic<int> ref_count_{1};

public:
    void addRef() const {
        ref_count_.fetch_add(1, std::memory_order_relaxed);
    }
    
    void release() const {
        if (ref_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete this;
        }
    }
    
    int getRefCount() const {
        return ref_count_.load(std::memory_order_relaxed);
    }

protected:
    virtual ~AtomicRefCounted() = default;
};

// Atomic smart pointer
template<typename T>
class AtomicSharedPtr {
private:
    struct ControlBlock {
        T* ptr;
        std::atomic<int> ref_count;
        
        ControlBlock(T* p) : ptr(p), ref_count(1) {}
    };
    
    std::atomic<ControlBlock*> control_block_{nullptr};

public:
    AtomicSharedPtr() = default;
    
    explicit AtomicSharedPtr(T* ptr) {
        if (ptr) {
            control_block_.store(new ControlBlock(ptr));
        }
    }
    
    AtomicSharedPtr(const AtomicSharedPtr& other) {
        ControlBlock* cb = other.control_block_.load();
        if (cb) {
            cb->ref_count.fetch_add(1);
            control_block_.store(cb);
        }
    }
    
    ~AtomicSharedPtr() {
        release();
    }
    
    AtomicSharedPtr& operator=(const AtomicSharedPtr& other) {
        if (this != &other) {
            release();
            ControlBlock* cb = other.control_block_.load();
            if (cb) {
                cb->ref_count.fetch_add(1);
                control_block_.store(cb);
            }
        }
        return *this;
    }
    
    T* get() const {
        ControlBlock* cb = control_block_.load();
        return cb ? cb->ptr : nullptr;
    }
    
    T* operator->() const { return get(); }
    T& operator*() const { return *get(); }
    
    operator bool() const { return get() != nullptr; }

private:
    void release() {
        ControlBlock* cb = control_block_.load();
        if (cb && cb->ref_count.fetch_sub(1) == 1) {
            delete cb->ptr;
            delete cb;
        }
        control_block_.store(nullptr);
    }
};

// Producer-Consumer using atomics
class AtomicProducerConsumer {
private:
    static constexpr size_t BUFFER_SIZE = 1024;
    
    alignas(64) std::atomic<size_t> write_pos_{0};  // Cache line aligned
    alignas(64) std::atomic<size_t> read_pos_{0};   // Cache line aligned
    
    struct alignas(64) Item {  // Cache line aligned
        std::atomic<bool> ready{false};
        int data;
    };
    
    Item buffer_[BUFFER_SIZE];

public:
    bool produce(int item) {
        size_t current_write = write_pos_.load(std::memory_order_relaxed);
        size_t next_write = (current_write + 1) % BUFFER_SIZE;
        
        // Check if buffer is full
        if (next_write == read_pos_.load(std::memory_order_acquire)) {
            return false;  // Buffer full
        }
        
        // Write data and mark as ready
        buffer_[current_write].data = item;
        buffer_[current_write].ready.store(true, std::memory_order_release);
        
        // Advance write position
        write_pos_.store(next_write, std::memory_order_relaxed);
        
        return true;
    }
    
    bool consume(int& item) {
        size_t current_read = read_pos_.load(std::memory_order_relaxed);
        
        // Check if data is ready
        if (!buffer_[current_read].ready.load(std::memory_order_acquire)) {
            return false;  // No data available
        }
        
        // Read data and mark as consumed
        item = buffer_[current_read].data;
        buffer_[current_read].ready.store(false, std::memory_order_relaxed);
        
        // Advance read position
        read_pos_.store((current_read + 1) % BUFFER_SIZE, std::memory_order_release);
        
        return true;
    }
    
    size_t size() const {
        size_t write_pos = write_pos_.load(std::memory_order_relaxed);
        size_t read_pos = read_pos_.load(std::memory_order_relaxed);
        
        if (write_pos >= read_pos) {
            return write_pos - read_pos;
        } else {
            return BUFFER_SIZE - read_pos + write_pos;
        }
    }
};

// Spinlock implementation using atomics
class Spinlock {
private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;

public:
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            // Spin-wait with pause for better performance
            while (flag_.test(std::memory_order_relaxed)) {
                std::this_thread::yield();  // Or use CPU pause instruction
            }
        }
    }
    
    bool try_lock() {
        return !flag_.test_and_set(std::memory_order_acquire);
    }
    
    void unlock() {
        flag_.clear(std::memory_order_release);
    }
};

// Memory ordering demonstration
class MemoryOrderingDemo {
private:
    std::atomic<int> data_{0};
    std::atomic<bool> ready_{false};

public:
    // Writer (relaxed ordering)
    void write_relaxed(int value) {
        data_.store(value, std::memory_order_relaxed);
        ready_.store(true, std::memory_order_relaxed);
    }
    
    // Reader (relaxed ordering)
    int read_relaxed() {
        while (!ready_.load(std::memory_order_relaxed)) {
            std::this_thread::yield();
        }
        return data_.load(std::memory_order_relaxed);
    }
    
    // Writer (acquire-release ordering)
    void write_acq_rel(int value) {
        data_.store(value, std::memory_order_relaxed);
        ready_.store(true, std::memory_order_release);  // Release semantics
    }
    
    // Reader (acquire-release ordering)
    int read_acq_rel() {
        while (!ready_.load(std::memory_order_acquire)) {  // Acquire semantics
            std::this_thread::yield();
        }
        return data_.load(std::memory_order_relaxed);
    }
    
    // Writer (sequential consistency)
    void write_seq_cst(int value) {
        data_.store(value, std::memory_order_seq_cst);
        ready_.store(true, std::memory_order_seq_cst);
    }
    
    // Reader (sequential consistency)
    int read_seq_cst() {
        while (!ready_.load(std::memory_order_seq_cst)) {
            std::this_thread::yield();
        }
        return data_.load(std::memory_order_seq_cst);
    }
};

void demonstrateBasicAtomics() {
    std::cout << "=== Basic Atomic Operations Demonstration ===\n\n";
    
    std::atomic<int> counter{0};
    std::atomic<bool> done{false};
    
    const int num_threads = 4;
    const int increments_per_thread = 100000;
    
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Create threads that increment counter
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&counter, increments_per_thread]() {
            for (int j = 0; j < increments_per_thread; ++j) {
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Expected count: " << num_threads * increments_per_thread << std::endl;
    std::cout << "Actual count: " << counter.load() << std::endl;
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
    
    // Test compare_exchange
    int expected = counter.load();
    int new_value = expected + 1000;
    
    if (counter.compare_exchange_strong(expected, new_value)) {
        std::cout << "Compare-exchange succeeded: " << counter.load() << std::endl;
    } else {
        std::cout << "Compare-exchange failed, current value: " << counter.load() << std::endl;
    }
}

void demonstrateLockFreeDataStructures() {
    std::cout << "\n=== Lock-Free Data Structures Demonstration ===\n\n";
    
    // Lock-free stack
    std::cout << "1. Lock-Free Stack:\n";
    LockFreeStack<int> stack;
    
    // Push some items
    for (int i = 1; i <= 5; ++i) {
        stack.push(i * 10);
        std::cout << "Pushed: " << i * 10 << std::endl;
    }
    
    // Pop some items
    int value;
    while (stack.pop(value)) {
        std::cout << "Popped: " << value << std::endl;
    }
    
    // Lock-free queue
    std::cout << "\n2. Lock-Free Queue:\n";
    LockFreeQueue<std::string> queue;
    
    // Enqueue some items
    std::vector<std::string> items = {"First", "Second", "Third", "Fourth"};
    for (const auto& item : items) {
        queue.enqueue(item);
        std::cout << "Enqueued: " << item << std::endl;
    }
    
    // Dequeue some items
    std::string str_value;
    while (queue.dequeue(str_value)) {
        std::cout << "Dequeued: " << str_value << std::endl;
    }
}

void demonstrateProducerConsumer() {
    std::cout << "\n=== Atomic Producer-Consumer Demonstration ===\n\n";
    
    AtomicProducerConsumer pc;
    std::atomic<bool> stop_flag{false};
    std::atomic<int> produced_count{0};
    std::atomic<int> consumed_count{0};
    
    // Producer thread
    std::thread producer([&pc, &stop_flag, &produced_count]() {
        int item = 1;
        while (!stop_flag.load(std::memory_order_relaxed)) {
            if (pc.produce(item)) {
                produced_count.fetch_add(1, std::memory_order_relaxed);
                std::cout << "Produced: " << item << std::endl;
                item++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    // Consumer thread
    std::thread consumer([&pc, &stop_flag, &consumed_count]() {
        int item;
        while (!stop_flag.load(std::memory_order_relaxed)) {
            if (pc.consume(item)) {
                consumed_count.fetch_add(1, std::memory_order_relaxed);
                std::cout << "Consumed: " << item << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
    });
    
    // Run for 2 seconds
    std::this_thread::sleep_for(std::chrono::seconds(2));
    stop_flag.store(true, std::memory_order_relaxed);
    
    producer.join();
    consumer.join();
    
    std::cout << "Produced: " << produced_count.load() << ", Consumed: " << consumed_count.load() << std::endl;
    std::cout << "Queue size: " << pc.size() << std::endl;
}

void demonstrateMemoryOrdering() {
    std::cout << "\n=== Memory Ordering Demonstration ===\n\n";
    
    MemoryOrderingDemo demo;
    
    // Test different memory orderings
    std::cout << "Testing relaxed memory ordering:\n";
    std::thread writer1([&demo]() {
        demo.write_relaxed(42);
    });
    
    std::thread reader1([&demo]() {
        int value = demo.read_relaxed();
        std::cout << "Read value (relaxed): " << value << std::endl;
    });
    
    writer1.join();
    reader1.join();
    
    std::cout << "\nTesting acquire-release ordering:\n";
    std::thread writer2([&demo]() {
        demo.write_acq_rel(84);
    });
    
    std::thread reader2([&demo]() {
        int value = demo.read_acq_rel();
        std::cout << "Read value (acquire-release): " << value << std::endl;
    });
    
    writer2.join();
    reader2.join();
}

void demonstrateSpinlock() {
    std::cout << "\n=== Spinlock Demonstration ===\n\n";
    
    Spinlock spinlock;
    int shared_counter = 0;
    const int num_threads = 4;
    const int increments_per_thread = 10000;
    
    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&spinlock, &shared_counter, increments_per_thread, i]() {
            for (int j = 0; j < increments_per_thread; ++j) {
                spinlock.lock();
                shared_counter++;
                spinlock.unlock();
            }
            std::cout << "Thread " << i << " completed\n";
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Expected: " << num_threads * increments_per_thread << std::endl;
    std::cout << "Actual: " << shared_counter << std::endl;
    std::cout << "Spinlock time: " << duration.count() << " ms" << std::endl;
}

void demonstrateInterProcessAtomics() {
    std::cout << "\n=== Inter-Process Atomic Demonstration ===\n\n";
    
    // Create shared memory for atomic data
    int shm_fd = shm_open("/atomic_demo_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(SharedAtomicData));
    
    SharedAtomicData* shared_data = static_cast<SharedAtomicData*>(
        mmap(0, sizeof(SharedAtomicData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)
    );
    
    if (shared_data == MAP_FAILED) {
        perror("mmap");
        return;
    }
    
    new (shared_data) SharedAtomicData();
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        std::cout << "Child process modifying atomic data...\n";
        
        for (int i = 0; i < 10; ++i) {
            shared_data->counter.fetch_add(1, std::memory_order_relaxed);
            
            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            shared_data->timestamp.store(now, std::memory_order_relaxed);
            shared_data->last_writer.store(getpid(), std::memory_order_relaxed);
            
            snprintf(shared_data->message_buffer, sizeof(shared_data->message_buffer),
                    "Message %d from child %d", i + 1, getpid());
            
            shared_data->ready.store(true, std::memory_order_release);
            
            std::cout << "Child: counter=" << shared_data->counter.load() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        exit(0);
        
    } else if (pid > 0) {
        // Parent process
        std::cout << "Parent process reading atomic data...\n";
        
        for (int i = 0; i < 15; ++i) {
            if (shared_data->ready.load(std::memory_order_acquire)) {
                std::cout << "Parent read: counter=" << shared_data->counter.load()
                          << ", timestamp=" << shared_data->timestamp.load()
                          << ", writer=" << shared_data->last_writer.load()
                          << ", message=" << shared_data->message_buffer << std::endl;
                shared_data->ready.store(false, std::memory_order_relaxed);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
        
        // Wait for child to complete
        int status;
        wait(&status);
        
        std::cout << "Final counter value: " << shared_data->counter.load() << std::endl;
        
    } else {
        perror("fork failed");
    }
    
    // Cleanup
    munmap(shared_data, sizeof(SharedAtomicData));
    shm_unlink("/atomic_demo_shm");
}

void benchmarkAtomics() {
    std::cout << "\n=== Atomic Operations Performance Benchmark ===\n";
    
    const int iterations = 1000000;
    std::atomic<int> atomic_counter{0};
    
    // Benchmark fetch_add
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        atomic_counter.fetch_add(1, std::memory_order_relaxed);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    std::cout << "fetch_add operations: " << iterations << " in " 
              << duration.count() << " ns" << std::endl;
    std::cout << "Average time per fetch_add: " 
              << static_cast<double>(duration.count()) / iterations << " ns" << std::endl;
    
    // Benchmark compare_exchange
    atomic_counter.store(0);
    start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        int expected = i;
        atomic_counter.compare_exchange_weak(expected, i + 1, std::memory_order_relaxed);
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    std::cout << "compare_exchange operations: " << iterations << " in " 
              << duration.count() << " ns" << std::endl;
    std::cout << "Average time per compare_exchange: " 
              << static_cast<double>(duration.count()) / iterations << " ns" << std::endl;
}

int main() {
    demonstrateBasicAtomics();
    demonstrateLockFreeDataStructures();
    demonstrateProducerConsumer();
    demonstrateMemoryOrdering();
    demonstrateSpinlock();
    demonstrateInterProcessAtomics();
    benchmarkAtomics();
    
    std::cout << "\n=== Key Atomic Concepts ===\n";
    std::cout << "1. Lock-free programming - no blocking synchronization\n";
    std::cout << "2. Memory ordering - control instruction reordering\n";
    std::cout << "3. Compare-and-swap (CAS) - atomic read-modify-write\n";
    std::cout << "4. ABA problem - value changes and changes back\n";
    std::cout << "5. Cache coherency - ensuring consistent view across cores\n";
    std::cout << "6. Memory barriers - synchronization points\n";
    
    return 0;
}
