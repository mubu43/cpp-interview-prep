#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <type_traits>

// ===== ABSTRACTION DEMONSTRATIONS =====

// 1. INTERFACE ABSTRACTION
// Abstract base class defines interface without exposing implementation details
class DatabaseConnection {
public:
    virtual ~DatabaseConnection() = default;
    
    // Pure virtual interface - clients don't need to know implementation
    virtual bool connect(const std::string& connectionString) = 0;
    virtual void disconnect() = 0;
    virtual bool execute(const std::string& query) = 0;
    virtual std::string getLastError() const = 0;
    virtual bool isConnected() const = 0;
    
    // Template method - defines algorithm structure
    bool executeTransaction(const std::vector<std::string>& queries) {
        if (!isConnected()) {
            return false;
        }
        
        beginTransaction();
        
        for (const auto& query : queries) {
            if (!execute(query)) {
                rollback();
                return false;
            }
        }
        
        return commit();
    }
    
protected:
    // Abstract methods for transaction management
    virtual bool beginTransaction() = 0;
    virtual bool commit() = 0;
    virtual bool rollback() = 0;
};

// Concrete implementation 1 - PostgreSQL
class PostgreSQLConnection : public DatabaseConnection {
private:
    bool connected_;
    std::string lastError_;
    std::string connectionString_;
    
public:
    PostgreSQLConnection() : connected_(false) {}
    
    bool connect(const std::string& connectionString) override {
        connectionString_ = connectionString;
        // Simulate PostgreSQL connection logic
        std::cout << "Connecting to PostgreSQL: " << connectionString << "\n";
        connected_ = true;
        lastError_.clear();
        return true;
    }
    
    void disconnect() override {
        if (connected_) {
            std::cout << "Disconnecting from PostgreSQL\n";
            connected_ = false;
        }
    }
    
    bool execute(const std::string& query) override {
        if (!connected_) {
            lastError_ = "Not connected to database";
            return false;
        }
        
        std::cout << "PostgreSQL executing: " << query << "\n";
        // Simulate query execution
        return true;
    }
    
    std::string getLastError() const override {
        return lastError_;
    }
    
    bool isConnected() const override {
        return connected_;
    }
    
protected:
    bool beginTransaction() override {
        std::cout << "PostgreSQL: BEGIN TRANSACTION\n";
        return true;
    }
    
    bool commit() override {
        std::cout << "PostgreSQL: COMMIT\n";
        return true;
    }
    
    bool rollback() override {
        std::cout << "PostgreSQL: ROLLBACK\n";
        return true;
    }
};

// Concrete implementation 2 - MySQL
class MySQLConnection : public DatabaseConnection {
private:
    bool connected_;
    std::string lastError_;
    
public:
    MySQLConnection() : connected_(false) {}
    
    bool connect(const std::string& connectionString) override {
        std::cout << "Connecting to MySQL: " << connectionString << "\n";
        connected_ = true;
        lastError_.clear();
        return true;
    }
    
    void disconnect() override {
        if (connected_) {
            std::cout << "Disconnecting from MySQL\n";
            connected_ = false;
        }
    }
    
    bool execute(const std::string& query) override {
        if (!connected_) {
            lastError_ = "Not connected to database";
            return false;
        }
        
        std::cout << "MySQL executing: " << query << "\n";
        return true;
    }
    
    std::string getLastError() const override {
        return lastError_;
    }
    
    bool isConnected() const override {
        return connected_;
    }
    
protected:
    bool beginTransaction() override {
        std::cout << "MySQL: START TRANSACTION\n";
        return true;
    }
    
    bool commit() override {
        std::cout << "MySQL: COMMIT\n";
        return true;
    }
    
    bool rollback() override {
        std::cout << "MySQL: ROLLBACK\n";
        return true;
    }
};

// 2. DATA ABSTRACTION WITH ENCAPSULATION
// Stack abstract data type - users don't know if it's array or linked list based
template<typename T>
class Stack {
public:
    virtual ~Stack() = default;
    
    // Abstract interface
    virtual void push(const T& item) = 0;
    virtual T pop() = 0;
    virtual const T& top() const = 0;
    virtual bool empty() const = 0;
    virtual size_t size() const = 0;
    
    // High-level operations built on abstract interface
    void pushMultiple(const std::vector<T>& items) {
        for (const auto& item : items) {
            push(item);
        }
    }
    
    std::vector<T> popMultiple(size_t count) {
        std::vector<T> result;
        for (size_t i = 0; i < count && !empty(); ++i) {
            result.push_back(pop());
        }
        return result;
    }
};

// Array-based implementation
template<typename T>
class ArrayStack : public Stack<T> {
private:
    static const size_t MAX_SIZE = 1000;
    T data_[MAX_SIZE];
    size_t top_index_;
    
public:
    ArrayStack() : top_index_(0) {}
    
    void push(const T& item) override {
        if (top_index_ >= MAX_SIZE) {
            throw std::overflow_error("Stack overflow");
        }
        data_[top_index_++] = item;
    }
    
    T pop() override {
        if (empty()) {
            throw std::underflow_error("Stack underflow");
        }
        return data_[--top_index_];
    }
    
    const T& top() const override {
        if (empty()) {
            throw std::underflow_error("Stack is empty");
        }
        return data_[top_index_ - 1];
    }
    
    bool empty() const override {
        return top_index_ == 0;
    }
    
    size_t size() const override {
        return top_index_;
    }
};

// Vector-based implementation
template<typename T>
class VectorStack : public Stack<T> {
private:
    std::vector<T> data_;
    
public:
    void push(const T& item) override {
        data_.push_back(item);
    }
    
    T pop() override {
        if (empty()) {
            throw std::underflow_error("Stack underflow");
        }
        T item = data_.back();
        data_.pop_back();
        return item;
    }
    
    const T& top() const override {
        if (empty()) {
            throw std::underflow_error("Stack is empty");
        }
        return data_.back();
    }
    
    bool empty() const override {
        return data_.empty();
    }
    
    size_t size() const override {
        return data_.size();
    }
};

// 3. LAYERED ABSTRACTION
// Network abstraction layers - each layer abstracts the complexity below

// Physical layer abstraction
class NetworkDevice {
public:
    virtual ~NetworkDevice() = default;
    virtual bool sendRawData(const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> receiveRawData() = 0;
    virtual bool isConnected() const = 0;
};

class EthernetDevice : public NetworkDevice {
public:
    bool sendRawData(const std::vector<uint8_t>& data) override {
        std::cout << "Ethernet: Sending " << data.size() << " bytes\n";
        return true;
    }
    
    std::vector<uint8_t> receiveRawData() override {
        std::cout << "Ethernet: Receiving data\n";
        return {0x01, 0x02, 0x03, 0x04};
    }
    
    bool isConnected() const override {
        return true;
    }
};

// Network layer abstraction
class NetworkProtocol {
protected:
    std::unique_ptr<NetworkDevice> device_;
    
public:
    NetworkProtocol(std::unique_ptr<NetworkDevice> device) 
        : device_(std::move(device)) {}
    
    virtual ~NetworkProtocol() = default;
    virtual bool sendPacket(const std::string& destination, const std::string& data) = 0;
    virtual std::string receivePacket() = 0;
};

class TCPProtocol : public NetworkProtocol {
public:
    TCPProtocol(std::unique_ptr<NetworkDevice> device) 
        : NetworkProtocol(std::move(device)) {}
    
    bool sendPacket(const std::string& destination, const std::string& data) override {
        std::cout << "TCP: Sending to " << destination << ": " << data << "\n";
        
        // Abstract away the complexity of TCP packet creation
        std::vector<uint8_t> tcpPacket = createTCPPacket(destination, data);
        return device_->sendRawData(tcpPacket);
    }
    
    std::string receivePacket() override {
        auto rawData = device_->receiveRawData();
        return parseTCPPacket(rawData);
    }
    
private:
    std::vector<uint8_t> createTCPPacket(const std::string& dest, const std::string& data) {
        // Complex TCP packet creation logic hidden from users
        std::vector<uint8_t> packet;
        // Add TCP headers, checksums, etc.
        for (char c : data) {
            packet.push_back(static_cast<uint8_t>(c));
        }
        return packet;
    }
    
    std::string parseTCPPacket(const std::vector<uint8_t>& rawData) {
        // Complex TCP packet parsing logic hidden from users
        std::string result;
        for (uint8_t byte : rawData) {
            result += static_cast<char>(byte);
        }
        return result;
    }
};

// Application layer abstraction
class HttpClient {
private:
    std::unique_ptr<NetworkProtocol> protocol_;
    
public:
    HttpClient(std::unique_ptr<NetworkProtocol> protocol) 
        : protocol_(std::move(protocol)) {}
    
    // High-level HTTP operations abstract away network complexity
    bool get(const std::string& url) {
        std::string httpRequest = "GET " + url + " HTTP/1.1\r\n\r\n";
        std::cout << "HTTP GET: " << url << "\n";
        return protocol_->sendPacket("server.com", httpRequest);
    }
    
    bool post(const std::string& url, const std::string& body) {
        std::string httpRequest = "POST " + url + " HTTP/1.1\r\n"
                                "Content-Length: " + std::to_string(body.length()) + "\r\n\r\n" + body;
        std::cout << "HTTP POST: " << url << "\n";
        return protocol_->sendPacket("server.com", httpRequest);
    }
    
    std::string getResponse() {
        return protocol_->receivePacket();
    }
};

// 4. ABSTRACTION THROUGH PIMPL (POINTER TO IMPLEMENTATION)
// Interface remains stable even when implementation changes

// Public interface - stable across versions
class FileManager {
private:
    class Impl;  // Forward declaration
    std::unique_ptr<Impl> pImpl_;  // Pointer to implementation
    
public:
    FileManager();
    ~FileManager();
    
    // Move constructor and assignment
    FileManager(FileManager&& other) noexcept;
    FileManager& operator=(FileManager&& other) noexcept;
    
    // Delete copy operations to keep example simple
    FileManager(const FileManager&) = delete;
    FileManager& operator=(const FileManager&) = delete;
    
    // Public interface - implementation details hidden
    bool createFile(const std::string& filename);
    bool deleteFile(const std::string& filename);
    bool copyFile(const std::string& source, const std::string& destination);
    std::vector<std::string> listFiles(const std::string& directory);
    bool fileExists(const std::string& filename);
    size_t getFileSize(const std::string& filename);
};

// Implementation hidden in source file
class FileManager::Impl {
private:
    std::string currentDirectory_;
    std::vector<std::string> recentFiles_;
    
public:
    Impl() : currentDirectory_(".") {}
    
    bool createFile(const std::string& filename) {
        std::cout << "Creating file: " << filename << "\n";
        recentFiles_.push_back(filename);
        return true;
    }
    
    bool deleteFile(const std::string& filename) {
        std::cout << "Deleting file: " << filename << "\n";
        auto it = std::find(recentFiles_.begin(), recentFiles_.end(), filename);
        if (it != recentFiles_.end()) {
            recentFiles_.erase(it);
        }
        return true;
    }
    
    bool copyFile(const std::string& source, const std::string& destination) {
        std::cout << "Copying " << source << " to " << destination << "\n";
        return true;
    }
    
    std::vector<std::string> listFiles(const std::string& directory) {
        std::cout << "Listing files in: " << directory << "\n";
        return {"file1.txt", "file2.txt", "file3.txt"};
    }
    
    bool fileExists(const std::string& filename) {
        return std::find(recentFiles_.begin(), recentFiles_.end(), filename) != recentFiles_.end();
    }
    
    size_t getFileSize(const std::string& filename) {
        return filename.length() * 100;  // Simulated size
    }
};

// Implementation of FileManager methods
FileManager::FileManager() : pImpl_(std::make_unique<Impl>()) {}
FileManager::~FileManager() = default;

FileManager::FileManager(FileManager&& other) noexcept : pImpl_(std::move(other.pImpl_)) {}
FileManager& FileManager::operator=(FileManager&& other) noexcept {
    pImpl_ = std::move(other.pImpl_);
    return *this;
}

bool FileManager::createFile(const std::string& filename) {
    return pImpl_->createFile(filename);
}

bool FileManager::deleteFile(const std::string& filename) {
    return pImpl_->deleteFile(filename);
}

bool FileManager::copyFile(const std::string& source, const std::string& destination) {
    return pImpl_->copyFile(source, destination);
}

std::vector<std::string> FileManager::listFiles(const std::string& directory) {
    return pImpl_->listFiles(directory);
}

bool FileManager::fileExists(const std::string& filename) {
    return pImpl_->fileExists(filename);
}

size_t FileManager::getFileSize(const std::string& filename) {
    return pImpl_->getFileSize(filename);
}

// 5. TEMPLATE ABSTRACTIONS
// Type-erased abstraction for different callable types

class Function {
private:
    struct CallableBase {
        virtual ~CallableBase() = default;
        virtual void call() = 0;
        virtual std::unique_ptr<CallableBase> clone() const = 0;
    };
    
    template<typename F>
    struct CallableImpl : public CallableBase {
        F func_;
        
        CallableImpl(F func) : func_(std::move(func)) {}
        
        void call() override {
            func_();
        }
        
        std::unique_ptr<CallableBase> clone() const override {
            return std::make_unique<CallableImpl>(func_);
        }
    };
    
    std::unique_ptr<CallableBase> callable_;
    
public:
    template<typename F>
    Function(F&& func) : callable_(std::make_unique<CallableImpl<F>>(std::forward<F>(func))) {}
    
    Function(const Function& other) : callable_(other.callable_->clone()) {}
    
    Function& operator=(const Function& other) {
        callable_ = other.callable_->clone();
        return *this;
    }
    
    Function(Function&&) = default;
    Function& operator=(Function&&) = default;
    
    void operator()() {
        if (callable_) {
            callable_->call();
        }
    }
};

// ===== DEMONSTRATION FUNCTIONS =====

void demonstrateInterfaceAbstraction() {
    std::cout << "\n=== INTERFACE ABSTRACTION ===\n";
    
    // Client code works with abstract interface
    auto testDatabase = [](std::unique_ptr<DatabaseConnection> db) {
        if (db->connect("localhost:5432/testdb")) {
            std::cout << "Connected successfully\n";
            
            // Execute simple query
            db->execute("SELECT * FROM users");
            
            // Execute transaction
            std::vector<std::string> queries = {
                "INSERT INTO users (name) VALUES ('Alice')",
                "INSERT INTO users (name) VALUES ('Bob')",
                "UPDATE users SET active = true"
            };
            
            if (db->executeTransaction(queries)) {
                std::cout << "Transaction completed successfully\n";
            }
            
            db->disconnect();
        }
    };
    
    std::cout << "\nTesting with PostgreSQL:\n";
    testDatabase(std::make_unique<PostgreSQLConnection>());
    
    std::cout << "\nTesting with MySQL:\n";
    testDatabase(std::make_unique<MySQLConnection>());
}

void demonstrateDataAbstraction() {
    std::cout << "\n=== DATA ABSTRACTION ===\n";
    
    // Client code works with abstract Stack interface
    auto testStack = [](std::unique_ptr<Stack<int>> stack) {
        std::cout << "Testing stack implementation:\n";
        
        // Use high-level operations
        stack->pushMultiple({1, 2, 3, 4, 5});
        std::cout << "Stack size: " << stack->size() << "\n";
        std::cout << "Top element: " << stack->top() << "\n";
        
        auto popped = stack->popMultiple(3);
        std::cout << "Popped elements: ";
        for (int val : popped) {
            std::cout << val << " ";
        }
        std::cout << "\nRemaining size: " << stack->size() << "\n";
    };
    
    std::cout << "\nTesting Array-based stack:\n";
    testStack(std::make_unique<ArrayStack<int>>());
    
    std::cout << "\nTesting Vector-based stack:\n";
    testStack(std::make_unique<VectorStack<int>>());
}

void demonstrateLayeredAbstraction() {
    std::cout << "\n=== LAYERED ABSTRACTION ===\n";
    
    // Each layer abstracts the complexity of layers below
    auto device = std::make_unique<EthernetDevice>();
    auto protocol = std::make_unique<TCPProtocol>(std::move(device));
    HttpClient client(std::move(protocol));
    
    // High-level HTTP operations hide network complexity
    client.get("/api/users");
    client.post("/api/users", "{\"name\": \"Alice\"}");
    std::string response = client.getResponse();
    std::cout << "Response: " << response << "\n";
}

void demonstratePimplAbstraction() {
    std::cout << "\n=== PIMPL ABSTRACTION ===\n";
    
    FileManager fileManager;
    
    // Simple interface hides implementation complexity
    fileManager.createFile("test.txt");
    fileManager.createFile("data.txt");
    
    std::cout << "File exists: " << fileManager.fileExists("test.txt") << "\n";
    std::cout << "File size: " << fileManager.getFileSize("test.txt") << "\n";
    
    fileManager.copyFile("test.txt", "backup.txt");
    
    auto files = fileManager.listFiles(".");
    std::cout << "Files: ";
    for (const auto& file : files) {
        std::cout << file << " ";
    }
    std::cout << "\n";
    
    fileManager.deleteFile("test.txt");
}

void demonstrateTypeErasure() {
    std::cout << "\n=== TYPE ERASURE ABSTRACTION ===\n";
    
    std::vector<Function> functions;
    
    // Different callable types abstracted to same interface
    functions.emplace_back([]() { std::cout << "Lambda function\n"; });
    
    struct Functor {
        void operator()() { std::cout << "Function object\n"; }
    };
    functions.emplace_back(Functor{});
    
    auto regularFunction = []() { std::cout << "Regular function\n"; };
    functions.emplace_back(regularFunction);
    
    std::cout << "Calling abstracted functions:\n";
    for (auto& func : functions) {
        func();  // Uniform interface despite different types
    }
}

// Utility function to demonstrate abstraction benefits
void demonstrateAbstractionBenefits() {
    std::cout << "\n=== ABSTRACTION BENEFITS ===\n";
    
    std::cout << "1. Implementation Independence: ";
    std::cout << "Clients don't need to know internal details\n";
    
    std::cout << "2. Interchangeability: ";
    std::cout << "Different implementations can be swapped\n";
    
    std::cout << "3. Complexity Management: ";
    std::cout << "Complex systems broken into manageable layers\n";
    
    std::cout << "4. Code Reusability: ";
    std::cout << "Abstract interfaces enable generic programming\n";
    
    std::cout << "5. Maintainability: ";
    std::cout << "Changes in implementation don't affect clients\n";
}

int main() {
    try {
        std::cout << "C++ ABSTRACTION DEMONSTRATIONS\n";
        std::cout << "=============================\n";
        
        demonstrateInterfaceAbstraction();
        demonstrateDataAbstraction();
        demonstrateLayeredAbstraction();
        demonstratePimplAbstraction();
        demonstrateTypeErasure();
        demonstrateAbstractionBenefits();
        
        std::cout << "\n=== COMPILATION AND EXECUTION SUCCESSFUL ===\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

/*
Expected Output:
================

C++ ABSTRACTION DEMONSTRATIONS
=============================

=== INTERFACE ABSTRACTION ===

Testing with PostgreSQL:
Connecting to PostgreSQL: localhost:5432/testdb
Connected successfully
PostgreSQL executing: SELECT * FROM users
PostgreSQL: BEGIN TRANSACTION
PostgreSQL executing: INSERT INTO users (name) VALUES ('Alice')
PostgreSQL executing: INSERT INTO users (name) VALUES ('Bob')
PostgreSQL executing: UPDATE users SET active = true
PostgreSQL: COMMIT
Transaction completed successfully
Disconnecting from PostgreSQL

Testing with MySQL:
Connecting to MySQL: localhost:5432/testdb
Connected successfully
MySQL executing: SELECT * FROM users
MySQL: START TRANSACTION
MySQL executing: INSERT INTO users (name) VALUES ('Alice')
MySQL executing: INSERT INTO users (name) VALUES ('Bob')
MySQL executing: UPDATE users SET active = true
MySQL: COMMIT
Transaction completed successfully
Disconnecting from MySQL

=== DATA ABSTRACTION ===

Testing Array-based stack:
Testing stack implementation:
Stack size: 5
Top element: 5
Popped elements: 5 4 3
Remaining size: 2

Testing Vector-based stack:
Testing stack implementation:
Stack size: 5
Top element: 5
Popped elements: 5 4 3
Remaining size: 2

=== LAYERED ABSTRACTION ===
TCP: Sending to server.com: GET /api/users HTTP/1.1

Ethernet: Sending 26 bytes
TCP: Sending to server.com: POST /api/users HTTP/1.1
Content-Length: 17

{"name": "Alice"}
Ethernet: Sending 63 bytes
Ethernet: Receiving data
Response: ♠☻♥♦

=== PIMPL ABSTRACTION ===
Creating file: test.txt
Creating file: data.txt
File exists: 1
File size: 800
Copying test.txt to backup.txt
Listing files in: .
Files: file1.txt file2.txt file3.txt
Deleting file: test.txt

=== TYPE ERASURE ABSTRACTION ===
Calling abstracted functions:
Lambda function
Function object
Regular function

=== ABSTRACTION BENEFITS ===
1. Implementation Independence: Clients don't need to know internal details
2. Interchangeability: Different implementations can be swapped
3. Complexity Management: Complex systems broken into manageable layers
4. Code Reusability: Abstract interfaces enable generic programming
5. Maintainability: Changes in implementation don't affect clients

=== COMPILATION AND EXECUTION SUCCESSFUL ===

Compilation command:
g++ -std=c++17 -Wall -Wextra -O2 abstraction.cpp -o abstraction

Key Learning Points:
===================
1. Interface Abstraction: Hide implementation details behind pure virtual interfaces
2. Data Abstraction: Abstract data types hide internal representation
3. Layered Abstraction: Complex systems built in layers of abstraction
4. PIMPL: Separate interface from implementation for ABI stability
5. Type Erasure: Abstract different types behind uniform interface
6. Template Abstraction: Generic programming through type abstraction
7. Proper abstraction reduces coupling and increases maintainability
8. Abstraction enables polymorphism and code reusability
9. Each abstraction layer should have a clear, focused responsibility
10. Good abstraction makes complex systems manageable and extensible
*/
