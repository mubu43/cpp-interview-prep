# RAII (Resource Acquisition Is Initialization) Study Guide

## Overview
RAII is a C++ programming idiom where resource acquisition is tied to object lifetime. Resources are acquired in constructors and released in destructors, ensuring automatic cleanup and exception safety.

## Core Principle
**"Resource Acquisition Is Initialization"**
- Acquire resources in constructor
- Release resources in destructor
- Use object lifetime to manage resource lifetime

## Fundamental Concepts

### 1. Automatic Resource Management
```cpp
class FileHandler {
    std::ifstream file_;
public:
    FileHandler(const std::string& filename) : file_(filename) {
        if (!file_) throw std::runtime_error("Failed to open file");
    }
    ~FileHandler() {
        // file_ automatically closed
    }
};
```

### 2. Exception Safety
RAII provides strong exception safety guarantees:
- **Basic guarantee**: No resource leaks
- **Strong guarantee**: Operation succeeds completely or has no effect
- **No-throw guarantee**: Operation never throws

## When to Use RAII
✅ **Always use for:**
- File handles
- Memory allocation
- Mutex locks
- Network connections
- Database transactions
- Graphics contexts
- Any limited resource

❌ **Not applicable for:**
- Value types (int, float, etc.)
- Resources with unclear ownership
- Global resources

## Common RAII Applications

### 1. Memory Management
```cpp
// Manual memory management (BAD)
void badFunction() {
    int* ptr = new int(42);
    // If exception thrown here, memory leak!
    processData(ptr);
    delete ptr;  // May never execute
}

// RAII memory management (GOOD)
void goodFunction() {
    auto ptr = std::make_unique<int>(42);
    processData(ptr.get());
    // Automatic cleanup, even with exceptions
}
```

### 2. File Handling
```cpp
class FileRAII {
    FILE* file_;
public:
    FileRAII(const char* filename, const char* mode) 
        : file_(fopen(filename, mode)) {
        if (!file_) throw std::runtime_error("File open failed");
    }
    
    ~FileRAII() {
        if (file_) fclose(file_);
    }
    
    FILE* get() const { return file_; }
};
```

### 3. Lock Management
```cpp
void threadSafeFunction() {
    std::lock_guard<std::mutex> lock(mutex_);
    // Critical section
    // Mutex automatically unlocked at scope exit
}
```

### 4. Database Transactions
```cpp
class Transaction {
    Database& db_;
    bool committed_ = false;
    
public:
    explicit Transaction(Database& db) : db_(db) {
        db_.beginTransaction();
    }
    
    ~Transaction() {
        if (!committed_) {
            db_.rollback();  // Auto-rollback if not committed
        }
    }
    
    void commit() {
        db_.commit();
        committed_ = true;
    }
};
```

## Smart Pointers - RAII in Action

### unique_ptr
```cpp
std::unique_ptr<Resource> ptr = std::make_unique<Resource>();
// Exclusive ownership, automatic deletion
```

### shared_ptr
```cpp
std::shared_ptr<Resource> ptr = std::make_shared<Resource>();
// Shared ownership, reference counting
```

### Custom Deleters
```cpp
auto deleter = [](FILE* f) { if (f) fclose(f); };
std::unique_ptr<FILE, decltype(deleter)> file(fopen("test.txt", "r"), deleter);
```

## Rule of Five and RAII

When implementing RAII classes, consider the Rule of Five:

```cpp
class RAIIClass {
public:
    // Constructor
    RAIIClass(Resource resource);
    
    // Destructor
    ~RAIIClass();
    
    // Copy constructor
    RAIIClass(const RAIIClass& other);
    
    // Copy assignment
    RAIIClass& operator=(const RAIIClass& other);
    
    // Move constructor
    RAIIClass(RAIIClass&& other) noexcept;
    
    // Move assignment
    RAIIClass& operator=(RAIIClass&& other) noexcept;
};
```

## Modern RAII Best Practices

### 1. Prefer Stack Allocation
```cpp
// Good - automatic cleanup
{
    FileHandler file("data.txt");
    // Use file
} // Automatically closed

// Avoid - manual management
FileHandler* file = new FileHandler("data.txt");
// Use file
delete file; // Easy to forget!
```

### 2. Use Move Semantics
```cpp
class MoveableRAII {
    Resource* resource_;
    
public:
    // Move constructor
    MoveableRAII(MoveableRAII&& other) noexcept 
        : resource_(other.resource_) {
        other.resource_ = nullptr;  // Transfer ownership
    }
    
    // Move assignment
    MoveableRAII& operator=(MoveableRAII&& other) noexcept {
        if (this != &other) {
            cleanup();  // Clean up current resource
            resource_ = other.resource_;
            other.resource_ = nullptr;
        }
        return *this;
    }
};
```

### 3. Make Classes Non-Copyable When Appropriate
```cpp
class NonCopyableRAII {
public:
    NonCopyableRAII() = default;
    ~NonCopyableRAII() = default;
    
    // Delete copy operations
    NonCopyableRAII(const NonCopyableRAII&) = delete;
    NonCopyableRAII& operator=(const NonCopyableRAII&) = delete;
    
    // Allow move operations
    NonCopyableRAII(NonCopyableRAII&&) = default;
    NonCopyableRAII& operator=(NonCopyableRAII&&) = default;
};
```

### 4. Exception-Safe Constructors
```cpp
class SafeRAII {
    Resource1* r1_;
    Resource2* r2_;
    
public:
    SafeRAII() try : r1_(new Resource1()), r2_(new Resource2()) {
        // Constructor body
    } catch (...) {
        delete r1_;  // Clean up r1_ if r2_ construction fails
        throw;
    }
    
    ~SafeRAII() {
        delete r2_;
        delete r1_;
    }
};
```

## Advanced RAII Techniques

### 1. Scope Guards
```cpp
template<typename Func>
class ScopeGuard {
    Func func_;
    bool active_;
    
public:
    explicit ScopeGuard(Func func) 
        : func_(std::move(func)), active_(true) {}
    
    ~ScopeGuard() {
        if (active_) func_();
    }
    
    void dismiss() { active_ = false; }
    
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
};

// Usage
auto guard = ScopeGuard([&]() { cleanup(); });
```

### 2. Custom RAII Wrappers
```cpp
template<typename T, typename Deleter>
class RAIIWrapper {
    T resource_;
    Deleter deleter_;
    
public:
    template<typename... Args>
    RAIIWrapper(Deleter d, Args&&... args) 
        : resource_(std::forward<Args>(args)...), deleter_(d) {}
    
    ~RAIIWrapper() {
        deleter_(resource_);
    }
    
    T& get() { return resource_; }
    const T& get() const { return resource_; }
};
```

### 3. RAII with Perfect Forwarding
```cpp
template<typename T>
class ForwardingRAII {
    T resource_;
    
public:
    template<typename... Args>
    explicit ForwardingRAII(Args&&... args) 
        : resource_(std::forward<Args>(args)...) {}
    
    template<typename F>
    auto invoke(F&& func) -> decltype(func(resource_)) {
        return func(resource_);
    }
};
```

## Common Pitfalls

### 1. **Forgetting Virtual Destructors**
```cpp
class Base {
public:
    virtual ~Base() = default;  // Essential for proper cleanup
};
```

### 2. **Resource Leaks in Constructors**
```cpp
// Bad - potential leak
class BadRAII {
    Resource* r1_;
    Resource* r2_;
public:
    BadRAII() : r1_(new Resource()), r2_(new Resource()) {
        // If second allocation throws, r1_ leaks
    }
};

// Good - exception safe
class GoodRAII {
    std::unique_ptr<Resource> r1_;
    std::unique_ptr<Resource> r2_;
public:
    GoodRAII() : r1_(std::make_unique<Resource>()), 
                 r2_(std::make_unique<Resource>()) {
        // Exception safe - smart pointers clean up automatically
    }
};
```

### 3. **Accidental Copies**
```cpp
// Might accidentally copy expensive RAII objects
void processFile(FileHandler file);  // Pass by value - BAD

// Better - pass by reference or use move
void processFile(const FileHandler& file);  // Pass by reference
void processFile(FileHandler&& file);       // Pass by move
```

### 4. **Order of Destruction**
```cpp
class OrderMatters {
    Database db_;
    Connection conn_;  // Should be destroyed before db_
    
public:
    OrderMatters() : db_(), conn_(db_) {
        // conn_ depends on db_
    }
    // Destruction order: conn_ destroyed first, then db_
};
```

## Performance Considerations

### Memory Overhead
- Smart pointers: Minimal overhead (8-16 bytes)
- Custom RAII: Only what you add
- Stack allocation: No allocation overhead

### Performance Benefits
- Eliminates manual cleanup code
- Reduces bugs and memory leaks
- Enables compiler optimizations
- Exception safety without performance cost

### Benchmarking RAII vs Manual
```cpp
// Manual management
auto start = std::chrono::high_resolution_clock::now();
for (int i = 0; i < 1000000; ++i) {
    int* ptr = new int(i);
    delete ptr;
}
auto end = std::chrono::high_resolution_clock::now();

// RAII with unique_ptr
start = std::chrono::high_resolution_clock::now();
for (int i = 0; i < 1000000; ++i) {
    auto ptr = std::make_unique<int>(i);
}
end = std::chrono::high_resolution_clock::now();
```

## Testing RAII

### 1. Test Exception Safety
```cpp
TEST(RAIITest, ExceptionSafety) {
    EXPECT_NO_THROW({
        try {
            RAIIClass obj;
            throw std::runtime_error("test");
        } catch (...) {
            // Resource should still be cleaned up
        }
    });
}
```

### 2. Test Move Semantics
```cpp
TEST(RAIITest, MoveSemantics) {
    RAIIClass obj1;
    RAIIClass obj2 = std::move(obj1);
    
    EXPECT_TRUE(obj2.isValid());
    EXPECT_FALSE(obj1.isValid());  // Moved-from state
}
```

### 3. Memory Leak Testing
```cpp
// Use tools like Valgrind, AddressSanitizer, or Visual Studio diagnostics
TEST(RAIITest, NoMemoryLeaks) {
    // Create and destroy many RAII objects
    for (int i = 0; i < 1000; ++i) {
        RAIIClass obj;
        obj.doWork();
    }
    // Memory usage should return to baseline
}
```

## Real-World Examples

### Graphics Context
```cpp
class OpenGLContext {
    GLuint texture_;
    GLuint buffer_;
    
public:
    OpenGLContext() {
        glGenTextures(1, &texture_);
        glGenBuffers(1, &buffer_);
    }
    
    ~OpenGLContext() {
        glDeleteBuffers(1, &buffer_);
        glDeleteTextures(1, &texture_);
    }
};
```

### Network Socket
```cpp
class Socket {
    int socket_fd_;
    
public:
    Socket(const std::string& host, int port) {
        socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        // Connect to host:port
    }
    
    ~Socket() {
        if (socket_fd_ >= 0) {
            close(socket_fd_);
        }
    }
};
```

## Interview Questions

**Q: What is RAII and why is it important?**
A: RAII ties resource lifetime to object lifetime. It's important because it provides automatic cleanup, exception safety, and eliminates resource leaks.

**Q: How does RAII help with exception safety?**
A: Destructors are automatically called during stack unwinding, ensuring resources are cleaned up even when exceptions are thrown.

**Q: What's the relationship between RAII and smart pointers?**
A: Smart pointers are the most common implementation of RAII for memory management. They automatically delete memory when going out of scope.

**Q: When shouldn't you use RAII?**
A: For simple value types that don't manage resources, or when you need manual control over resource lifetime beyond object scope.

**Q: How do you implement RAII for a C library resource?**
A: Wrap the C resource in a C++ class, acquire it in the constructor, and release it in the destructor using the appropriate C cleanup function.

## Conclusion

RAII is fundamental to modern C++ programming. It provides:
- Automatic resource management
- Exception safety
- Reduced code complexity
- Prevention of resource leaks
- Clear resource ownership semantics

Every C++ programmer should understand and apply RAII principles. It's not just a pattern—it's a core language idiom that makes C++ code safer and more maintainable.
