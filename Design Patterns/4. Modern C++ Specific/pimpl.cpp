#include <iostream>
#include <string>
#include <memory>
#include <vector>

// Forward declarations to reduce compilation dependencies
class DatabaseImpl;
class NetworkManagerImpl;
class LoggerImpl;

// ===========================================
// Example 1: Database Connection Class
// ===========================================

// Header file would only contain this:
class Database {
public:
    Database(const std::string& connectionString);
    ~Database(); // Destructor needed for unique_ptr with incomplete type
    
    // Copy operations - need special handling with pimpl
    Database(const Database& other);
    Database& operator=(const Database& other);
    
    // Move operations (compiler-generated would work, but explicit is clearer)
    Database(Database&& other) noexcept;
    Database& operator=(Database&& other) noexcept;
    
    // Public interface
    bool connect();
    void disconnect();
    bool executeQuery(const std::string& query);
    std::string getLastError() const;
    bool isConnected() const;

private:
    std::unique_ptr<DatabaseImpl> pImpl; // Pointer to implementation
};

// Implementation class (would be in .cpp file)
class DatabaseImpl {
public:
    DatabaseImpl(const std::string& connectionString) 
        : connectionString_(connectionString), connected_(false) {}
    
    bool connect() {
        std::cout << "Connecting to database: " << connectionString_ << std::endl;
        // Simulate connection logic
        connected_ = true;
        return true;
    }
    
    void disconnect() {
        if (connected_) {
            std::cout << "Disconnecting from database" << std::endl;
            connected_ = false;
        }
    }
    
    bool executeQuery(const std::string& query) {
        if (!connected_) {
            lastError_ = "Not connected to database";
            return false;
        }
        std::cout << "Executing query: " << query << std::endl;
        lastError_.clear();
        return true;
    }
    
    std::string getLastError() const { return lastError_; }
    bool isConnected() const { return connected_; }

private:
    std::string connectionString_;
    bool connected_;
    std::string lastError_;
    // Many more private members would go here...
    // Database drivers, connection pools, etc.
};

// Implementation of Database methods (would be in .cpp file)
Database::Database(const std::string& connectionString) 
    : pImpl(std::make_unique<DatabaseImpl>(connectionString)) {}

Database::~Database() = default; // Explicit destructor needed

Database::Database(const Database& other) 
    : pImpl(std::make_unique<DatabaseImpl>(*other.pImpl)) {}

Database& Database::operator=(const Database& other) {
    if (this != &other) {
        pImpl = std::make_unique<DatabaseImpl>(*other.pImpl);
    }
    return *this;
}

Database::Database(Database&& other) noexcept = default;
Database& Database::operator=(Database&& other) noexcept = default;

bool Database::connect() { return pImpl->connect(); }
void Database::disconnect() { pImpl->disconnect(); }
bool Database::executeQuery(const std::string& query) { return pImpl->executeQuery(query); }
std::string Database::getLastError() const { return pImpl->getLastError(); }
bool Database::isConnected() const { return pImpl->isConnected(); }

// ===========================================
// Example 2: Network Manager with Complex Dependencies
// ===========================================

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();
    
    // Rule of 5 for pimpl
    NetworkManager(const NetworkManager& other);
    NetworkManager& operator=(const NetworkManager& other);
    NetworkManager(NetworkManager&& other) noexcept;
    NetworkManager& operator=(NetworkManager&& other) noexcept;
    
    bool sendRequest(const std::string& url, const std::string& data);
    std::string getResponse() const;
    void setTimeout(int seconds);
    bool isConnected() const;

private:
    std::unique_ptr<NetworkManagerImpl> pImpl;
};

class NetworkManagerImpl {
public:
    NetworkManagerImpl() : timeout_(30), connected_(false) {
        std::cout << "NetworkManager implementation created" << std::endl;
    }
    
    bool sendRequest(const std::string& url, const std::string& data) {
        std::cout << "Sending request to: " << url << std::endl;
        std::cout << "Data: " << data << std::endl;
        // Simulate network operations
        lastResponse_ = "Response from " + url;
        connected_ = true;
        return true;
    }
    
    std::string getResponse() const { return lastResponse_; }
    void setTimeout(int seconds) { timeout_ = seconds; }
    bool isConnected() const { return connected_; }

private:
    int timeout_;
    bool connected_;
    std::string lastResponse_;
    // Complex networking libraries, SSL contexts, etc. would go here
};

NetworkManager::NetworkManager() : pImpl(std::make_unique<NetworkManagerImpl>()) {}
NetworkManager::~NetworkManager() = default;

NetworkManager::NetworkManager(const NetworkManager& other) 
    : pImpl(std::make_unique<NetworkManagerImpl>(*other.pImpl)) {}

NetworkManager& NetworkManager::operator=(const NetworkManager& other) {
    if (this != &other) {
        pImpl = std::make_unique<NetworkManagerImpl>(*other.pImpl);
    }
    return *this;
}

NetworkManager::NetworkManager(NetworkManager&& other) noexcept = default;
NetworkManager& NetworkManager::operator=(NetworkManager&& other) noexcept = default;

bool NetworkManager::sendRequest(const std::string& url, const std::string& data) {
    return pImpl->sendRequest(url, data);
}

std::string NetworkManager::getResponse() const { return pImpl->getResponse(); }
void NetworkManager::setTimeout(int seconds) { pImpl->setTimeout(seconds); }
bool NetworkManager::isConnected() const { return pImpl->isConnected(); }

// ===========================================
// Example 3: Template-based Pimpl (Advanced)
// ===========================================

template<typename Impl>
class PimplBase {
protected:
    std::unique_ptr<Impl> pImpl;

public:
    PimplBase() : pImpl(std::make_unique<Impl>()) {}
    ~PimplBase() = default;
    
    // Copy semantics
    PimplBase(const PimplBase& other) 
        : pImpl(std::make_unique<Impl>(*other.pImpl)) {}
    
    PimplBase& operator=(const PimplBase& other) {
        if (this != &other) {
            pImpl = std::make_unique<Impl>(*other.pImpl);
        }
        return *this;
    }
    
    // Move semantics
    PimplBase(PimplBase&& other) noexcept = default;
    PimplBase& operator=(PimplBase&& other) noexcept = default;
};

class LoggerImpl {
public:
    void log(const std::string& message) {
        std::cout << "[LOG]: " << message << std::endl;
    }
    
    void setLevel(int level) { level_ = level; }
    int getLevel() const { return level_; }

private:
    int level_ = 1;
};

class Logger : public PimplBase<LoggerImpl> {
public:
    void log(const std::string& message) { pImpl->log(message); }
    void setLevel(int level) { pImpl->setLevel(level); }
    int getLevel() const { return pImpl->getLevel(); }
};

// ===========================================
// Demonstration
// ===========================================

void demonstratePimpl() {
    std::cout << "=== Pimpl Pattern Demonstration ===\n\n";
    
    std::cout << "1. Database Example:\n";
    {
        Database db("postgresql://localhost:5432/mydb");
        db.connect();
        db.executeQuery("SELECT * FROM users");
        std::cout << "Connected: " << (db.isConnected() ? "Yes" : "No") << std::endl;
        
        // Copy constructor test
        Database db2 = db;
        db2.executeQuery("SELECT * FROM products");
        
        db.disconnect();
    }
    
    std::cout << "\n2. Network Manager Example:\n";
    {
        NetworkManager nm;
        nm.setTimeout(60);
        nm.sendRequest("https://api.example.com/data", "{'user': 'test'}");
        std::cout << "Response: " << nm.getResponse() << std::endl;
        
        // Move constructor test
        NetworkManager nm2 = std::move(nm);
        std::cout << "Moved NetworkManager connected: " 
                  << (nm2.isConnected() ? "Yes" : "No") << std::endl;
    }
    
    std::cout << "\n3. Template-based Pimpl (Logger):\n";
    {
        Logger logger;
        logger.setLevel(2);
        logger.log("This is a test message");
        std::cout << "Logger level: " << logger.getLevel() << std::endl;
        
        // Copy test
        Logger logger2 = logger;
        logger2.log("Message from copied logger");
    }
    
    std::cout << "\n4. Benefits Demonstration:\n";
    std::cout << "- Compilation time: Headers are clean and minimal\n";
    std::cout << "- Binary compatibility: Implementation can change without recompiling clients\n";
    std::cout << "- Encapsulation: Private members are truly private\n";
    std::cout << "- Dependency hiding: Complex dependencies are hidden from headers\n";
}

int main() {
    demonstratePimpl();
    return 0;
}
