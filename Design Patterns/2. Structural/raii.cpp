#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>

// RAII Demonstration - Resource Acquisition Is Initialization

// 1. Basic RAII for File Handling
class FileHandler {
private:
    std::string filename_;
    std::fstream file_;

public:
    explicit FileHandler(const std::string& filename) 
        : filename_(filename) {
        file_.open(filename_, std::ios::in | std::ios::out | std::ios::app);
        if (!file_.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename_);
        }
        std::cout << "File opened: " << filename_ << std::endl;
    }
    
    // Destructor automatically closes file
    ~FileHandler() {
        if (file_.is_open()) {
            file_.close();
            std::cout << "File closed: " << filename_ << std::endl;
        }
    }
    
    // Delete copy constructor and assignment to prevent resource duplication
    FileHandler(const FileHandler&) = delete;
    FileHandler& operator=(const FileHandler&) = delete;
    
    // Move constructor and assignment
    FileHandler(FileHandler&& other) noexcept 
        : filename_(std::move(other.filename_)), file_(std::move(other.file_)) {
        std::cout << "File moved: " << filename_ << std::endl;
    }
    
    FileHandler& operator=(FileHandler&& other) noexcept {
        if (this != &other) {
            if (file_.is_open()) {
                file_.close();
            }
            filename_ = std::move(other.filename_);
            file_ = std::move(other.file_);
        }
        return *this;
    }
    
    void write(const std::string& data) {
        if (file_.is_open()) {
            file_ << data << std::endl;
            file_.flush();
        }
    }
    
    bool isOpen() const {
        return file_.is_open();
    }
};

// 2. RAII for Memory Management (Smart Pointers)
class Resource {
private:
    std::string name_;
    std::vector<int> data_;

public:
    explicit Resource(const std::string& name, size_t size = 1000) 
        : name_(name), data_(size) {
        std::cout << "Resource created: " << name_ << " (size: " << size << ")" << std::endl;
    }
    
    ~Resource() {
        std::cout << "Resource destroyed: " << name_ << std::endl;
    }
    
    void process() {
        std::cout << "Processing resource: " << name_ << std::endl;
        // Simulate some work
        for (size_t i = 0; i < data_.size(); ++i) {
            data_[i] = static_cast<int>(i);
        }
    }
    
    const std::string& getName() const { return name_; }
};

// 3. RAII for Lock Management
class ThreadSafeCounter {
private:
    mutable std::mutex mutex_;
    int count_ = 0;

public:
    void increment() {
        std::lock_guard<std::mutex> lock(mutex_);  // RAII lock
        ++count_;
        std::cout << "Incremented to: " << count_ << std::endl;
    }
    
    void decrement() {
        std::lock_guard<std::mutex> lock(mutex_);  // RAII lock
        --count_;
        std::cout << "Decremented to: " << count_ << std::endl;
    }
    
    int getValue() const {
        std::lock_guard<std::mutex> lock(mutex_);  // RAII lock
        return count_;
    }
};

// 4. Custom RAII Timer for Performance Measurement
class ScopedTimer {
private:
    std::string name_;
    std::chrono::high_resolution_clock::time_point start_;

public:
    explicit ScopedTimer(const std::string& name) 
        : name_(name), start_(std::chrono::high_resolution_clock::now()) {
        std::cout << "Timer started: " << name_ << std::endl;
    }
    
    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        std::cout << "Timer finished: " << name_ 
                  << " (Duration: " << duration.count() << " microseconds)" << std::endl;
    }
    
    // Non-copyable and non-movable for simplicity
    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
    ScopedTimer(ScopedTimer&&) = delete;
    ScopedTimer& operator=(ScopedTimer&&) = delete;
};

// 5. RAII for Network Connection
class NetworkConnection {
private:
    std::string address_;
    bool connected_;

public:
    explicit NetworkConnection(const std::string& address) 
        : address_(address), connected_(false) {
        connect();
    }
    
    ~NetworkConnection() {
        disconnect();
    }
    
    // Non-copyable but movable
    NetworkConnection(const NetworkConnection&) = delete;
    NetworkConnection& operator=(const NetworkConnection&) = delete;
    
    NetworkConnection(NetworkConnection&& other) noexcept
        : address_(std::move(other.address_)), connected_(other.connected_) {
        other.connected_ = false;
        std::cout << "Connection moved to: " << address_ << std::endl;
    }
    
    NetworkConnection& operator=(NetworkConnection&& other) noexcept {
        if (this != &other) {
            disconnect();
            address_ = std::move(other.address_);
            connected_ = other.connected_;
            other.connected_ = false;
        }
        return *this;
    }
    
    void sendData(const std::string& data) {
        if (connected_) {
            std::cout << "Sending to " << address_ << ": " << data << std::endl;
        } else {
            std::cout << "Cannot send data - not connected to " << address_ << std::endl;
        }
    }
    
    bool isConnected() const { return connected_; }

private:
    void connect() {
        std::cout << "Connecting to: " << address_ << std::endl;
        // Simulate connection logic
        connected_ = true;
        std::cout << "Connected to: " << address_ << std::endl;
    }
    
    void disconnect() {
        if (connected_) {
            std::cout << "Disconnecting from: " << address_ << std::endl;
            connected_ = false;
            std::cout << "Disconnected from: " << address_ << std::endl;
        }
    }
};

// 6. RAII for Database Transaction
class DatabaseTransaction {
private:
    std::string connection_;
    bool committed_;
    bool active_;

public:
    explicit DatabaseTransaction(const std::string& connection)
        : connection_(connection), committed_(false), active_(true) {
        std::cout << "Transaction started on: " << connection_ << std::endl;
    }
    
    ~DatabaseTransaction() {
        if (active_ && !committed_) {
            rollback();
        }
    }
    
    // Non-copyable and non-movable for safety
    DatabaseTransaction(const DatabaseTransaction&) = delete;
    DatabaseTransaction& operator=(const DatabaseTransaction&) = delete;
    DatabaseTransaction(DatabaseTransaction&&) = delete;
    DatabaseTransaction& operator=(DatabaseTransaction&&) = delete;
    
    void execute(const std::string& sql) {
        if (active_) {
            std::cout << "Executing SQL on " << connection_ << ": " << sql << std::endl;
        } else {
            throw std::runtime_error("Transaction is not active");
        }
    }
    
    void commit() {
        if (active_ && !committed_) {
            std::cout << "Committing transaction on: " << connection_ << std::endl;
            committed_ = true;
            active_ = false;
        }
    }
    
    void rollback() {
        if (active_) {
            std::cout << "Rolling back transaction on: " << connection_ << std::endl;
            active_ = false;
        }
    }
    
    bool isActive() const { return active_; }
};

// 7. Template RAII Wrapper
template<typename Resource, typename Deleter>
class RAIIWrapper {
private:
    Resource resource_;
    Deleter deleter_;
    bool active_;

public:
    template<typename... Args>
    RAIIWrapper(Deleter deleter, Args&&... args)
        : resource_(std::forward<Args>(args)...), deleter_(deleter), active_(true) {}
    
    ~RAIIWrapper() {
        if (active_) {
            deleter_(resource_);
        }
    }
    
    // Move semantics
    RAIIWrapper(RAIIWrapper&& other) noexcept
        : resource_(std::move(other.resource_)), deleter_(std::move(other.deleter_)), active_(other.active_) {
        other.active_ = false;
    }
    
    RAIIWrapper& operator=(RAIIWrapper&& other) noexcept {
        if (this != &other) {
            if (active_) {
                deleter_(resource_);
            }
            resource_ = std::move(other.resource_);
            deleter_ = std::move(other.deleter_);
            active_ = other.active_;
            other.active_ = false;
        }
        return *this;
    }
    
    // Delete copy operations
    RAIIWrapper(const RAIIWrapper&) = delete;
    RAIIWrapper& operator=(const RAIIWrapper&) = delete;
    
    Resource& get() { return resource_; }
    const Resource& get() const { return resource_; }
    
    Resource& operator*() { return resource_; }
    const Resource& operator*() const { return resource_; }
    
    Resource* operator->() { return &resource_; }
    const Resource* operator->() const { return &resource_; }
};

void demonstrateBasicRAII() {
    std::cout << "=== Basic RAII Examples ===\n\n";
    
    // 1. File RAII
    std::cout << "1. File RAII:\n";
    {
        try {
            FileHandler file("test.txt");
            file.write("Hello, RAII!");
            file.write("This file will be automatically closed.");
            // File automatically closed when leaving scope
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    std::cout << std::endl;
    
    // 2. Smart Pointer RAII
    std::cout << "2. Smart Pointer RAII:\n";
    {
        auto resource1 = std::make_unique<Resource>("UniqueResource");
        resource1->process();
        
        auto resource2 = std::make_shared<Resource>("SharedResource");
        {
            auto resource3 = resource2;  // Shared ownership
            resource3->process();
            std::cout << "resource2 use count: " << resource2.use_count() << std::endl;
        }
        std::cout << "resource2 use count after scope: " << resource2.use_count() << std::endl;
        
        // Resources automatically destroyed when leaving scope
    }
    std::cout << std::endl;
}

void demonstrateAdvancedRAII() {
    std::cout << "3. Lock RAII:\n";
    {
        ThreadSafeCounter counter;
        counter.increment();
        counter.increment();
        counter.decrement();
        std::cout << "Final count: " << counter.getValue() << std::endl;
        // Mutex automatically unlocked with each operation
    }
    std::cout << std::endl;
    
    std::cout << "4. Timer RAII:\n";
    {
        ScopedTimer timer("Operation1");
        // Simulate some work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // Timer automatically measures and prints duration
    }
    std::cout << std::endl;
    
    std::cout << "5. Network Connection RAII:\n";
    {
        NetworkConnection conn("192.168.1.100:8080");
        conn.sendData("Hello, Server!");
        conn.sendData("This is a test message.");
        // Connection automatically closed when leaving scope
    }
    std::cout << std::endl;
    
    std::cout << "6. Database Transaction RAII:\n";
    
    // Successful transaction
    {
        DatabaseTransaction txn("MainDB");
        txn.execute("INSERT INTO users VALUES (1, 'John')");
        txn.execute("INSERT INTO users VALUES (2, 'Jane')");
        txn.commit();
        // Transaction committed successfully
    }
    std::cout << std::endl;
    
    // Failed transaction (automatic rollback)
    {
        DatabaseTransaction txn("MainDB");
        txn.execute("INSERT INTO users VALUES (3, 'Bob')");
        // Simulating an error - no commit called
        // Transaction automatically rolled back in destructor
    }
    std::cout << std::endl;
}

void demonstrateTemplateRAII() {
    std::cout << "7. Template RAII Wrapper:\n";
    {
        // Custom deleter for a hypothetical C resource
        auto customDeleter = [](int* ptr) {
            std::cout << "Custom deleter called for resource: " << *ptr << std::endl;
            delete ptr;
        };
        
        RAIIWrapper<int*, decltype(customDeleter)> wrapper(customDeleter, new int(42));
        std::cout << "Resource value: " << *wrapper << std::endl;
        // Custom deleter automatically called when leaving scope
    }
    std::cout << std::endl;
}

void demonstrateExceptionSafety() {
    std::cout << "8. Exception Safety with RAII:\n";
    {
        try {
            ScopedTimer timer("Exception Test");
            NetworkConnection conn("test.server.com:443");
            DatabaseTransaction txn("TestDB");
            
            // Simulate some operations
            conn.sendData("Starting operations...");
            txn.execute("CREATE TABLE test (id INT)");
            
            // Simulate an exception
            throw std::runtime_error("Simulated error");
            
            // This code won't execute
            txn.commit();
            conn.sendData("Operations completed");
            
        } catch (const std::exception& e) {
            std::cout << "Exception caught: " << e.what() << std::endl;
            std::cout << "All resources cleaned up automatically!" << std::endl;
        }
        // All destructors called automatically, ensuring cleanup
    }
    std::cout << std::endl;
}

void demonstrateRAII() {
    std::cout << "=== RAII Pattern Demonstration ===\n\n";
    
    demonstrateBasicRAII();
    demonstrateAdvancedRAII();
    demonstrateTemplateRAII();
    demonstrateExceptionSafety();
    
    std::cout << "=== RAII Best Practices Demonstrated ===\n";
    std::cout << "✓ Automatic resource cleanup\n";
    std::cout << "✓ Exception safety\n";
    std::cout << "✓ Move semantics support\n";
    std::cout << "✓ No manual resource management\n";
    std::cout << "✓ Deterministic destruction\n";
}

int main() {
    demonstrateRAII();
    return 0;
}
