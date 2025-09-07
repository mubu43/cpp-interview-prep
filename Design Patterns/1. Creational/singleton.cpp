#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

// Classic Singleton with thread safety
class Logger {
private:
    static std::unique_ptr<Logger> instance;
    static std::mutex mutex_;
    
    // Private constructor to prevent instantiation
    Logger() {
        std::cout << "Logger instance created\n";
    }

public:
    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // Thread-safe getInstance method
    static Logger* getInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance == nullptr) {
            instance = std::unique_ptr<Logger>(new Logger());
        }
        return instance.get();
    }
    
    void log(const std::string& message) {
        std::cout << "[LOG]: " << message << std::endl;
    }
};

// Static member definitions
std::unique_ptr<Logger> Logger::instance = nullptr;
std::mutex Logger::mutex_;

// Modern C++11 Singleton (Meyer's Singleton) - Thread-safe and lazy
class ConfigManager {
private:
    ConfigManager() {
        std::cout << "ConfigManager instance created\n";
    }

public:
    // Delete copy constructor and assignment operator
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    // Thread-safe since C++11 - static local variables are initialized once
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }
    
    void setConfig(const std::string& key, const std::string& value) {
        config_[key] = value;
        std::cout << "Config set: " << key << " = " << value << std::endl;
    }
    
    std::string getConfig(const std::string& key) {
        auto it = config_.find(key);
        return (it != config_.end()) ? it->second : "Not found";
    }

private:
    std::map<std::string, std::string> config_;
};

// Template Singleton for reusable singleton behavior
template<typename T>
class Singleton {
protected:
    Singleton() = default;
    ~Singleton() = default;

public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    
    static T& getInstance() {
        static T instance;
        return instance;
    }
};

// Example usage of template singleton
class DatabaseConnection : public Singleton<DatabaseConnection> {
    friend class Singleton<DatabaseConnection>;
    
private:
    DatabaseConnection() {
        std::cout << "Database connection established\n";
    }

public:
    void query(const std::string& sql) {
        std::cout << "Executing query: " << sql << std::endl;
    }
};

// Demonstration function
void demonstrateSingleton() {
    std::cout << "=== Singleton Pattern Demonstration ===\n\n";
    
    // Classic Singleton
    std::cout << "1. Classic Singleton (Logger):\n";
    Logger* logger1 = Logger::getInstance();
    Logger* logger2 = Logger::getInstance();
    
    std::cout << "logger1 address: " << logger1 << std::endl;
    std::cout << "logger2 address: " << logger2 << std::endl;
    std::cout << "Are they the same? " << (logger1 == logger2 ? "Yes" : "No") << std::endl;
    
    logger1->log("First message");
    logger2->log("Second message");
    
    std::cout << "\n2. Meyer's Singleton (ConfigManager):\n";
    ConfigManager& config1 = ConfigManager::getInstance();
    ConfigManager& config2 = ConfigManager::getInstance();
    
    std::cout << "config1 address: " << &config1 << std::endl;
    std::cout << "config2 address: " << &config2 << std::endl;
    std::cout << "Are they the same? " << (&config1 == &config2 ? "Yes" : "No") << std::endl;
    
    config1.setConfig("database_url", "localhost:5432");
    std::cout << "Config from config2: " << config2.getConfig("database_url") << std::endl;
    
    std::cout << "\n3. Template Singleton (DatabaseConnection):\n";
    DatabaseConnection& db1 = DatabaseConnection::getInstance();
    DatabaseConnection& db2 = DatabaseConnection::getInstance();
    
    std::cout << "db1 address: " << &db1 << std::endl;
    std::cout << "db2 address: " << &db2 << std::endl;
    std::cout << "Are they the same? " << (&db1 == &db2 ? "Yes" : "No") << std::endl;
    
    db1.query("SELECT * FROM users");
    
    // Thread safety test
    std::cout << "\n4. Thread Safety Test:\n";
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([]() {
            Logger* logger = Logger::getInstance();
            logger->log("Message from thread " + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())));
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}

int main() {
    demonstrateSingleton();
    return 0;
}
