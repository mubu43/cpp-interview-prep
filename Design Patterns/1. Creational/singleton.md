# Singleton Pattern Study Guide

## Overview
The Singleton pattern ensures that a class has only one instance and provides a global point of access to that instance.

## Intent
- Ensure a class has only one instance
- Provide global access to that instance
- Control object creation and access

## When to Use
✅ **Good for:**
- Logging systems
- Configuration managers
- Database connections
- Hardware interface access
- Cache systems
- Thread pools

❌ **Avoid when:**
- You need multiple instances
- Testing becomes difficult
- Creates tight coupling
- Violates Single Responsibility Principle

## Implementation Variants

### 1. Classic Thread-Safe Singleton
```cpp
class Singleton {
private:
    static std::unique_ptr<Singleton> instance;
    static std::mutex mutex_;
    Singleton() = default;

public:
    static Singleton* getInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance == nullptr) {
            instance = std::unique_ptr<Singleton>(new Singleton());
        }
        return instance.get();
    }
};
```

**Pros:** Explicit control, lazy initialization
**Cons:** Mutex overhead, potential memory leaks

### 2. Meyer's Singleton (Recommended)
```cpp
class Singleton {
public:
    static Singleton& getInstance() {
        static Singleton instance;  // Thread-safe since C++11
        return instance;
    }
private:
    Singleton() = default;
};
```

**Pros:** Thread-safe, no mutex overhead, automatic cleanup
**Cons:** Less control over initialization timing

### 3. Template Singleton
```cpp
template<typename T>
class Singleton {
public:
    static T& getInstance() {
        static T instance;
        return instance;
    }
protected:
    Singleton() = default;
};
```

**Pros:** Reusable, type-safe
**Cons:** Template complexity

## Key Design Considerations

### Thread Safety
- **C++11+**: Static local variables are thread-safe
- **Pre-C++11**: Use double-checked locking or call_once
- **Meyer's Singleton**: Preferred for modern C++

### Memory Management
- Use smart pointers for explicit control
- Static local variables handle cleanup automatically
- Be careful with destruction order

### Testability
```cpp
// Make singleton testable with dependency injection
class Logger {
private:
    static std::unique_ptr<Logger> instance;
public:
    static void setInstance(std::unique_ptr<Logger> newInstance) {
        instance = std::move(newInstance);
    }
};
```

## Common Pitfalls

### 1. **Destruction Order Problem**
Static objects destruction order is undefined across translation units.

**Solution:** Use Meyer's Singleton or implement careful cleanup.

### 2. **Testing Difficulties**
Singletons create global state that's hard to reset between tests.

**Solution:** Provide reset functionality or use dependency injection.

### 3. **Hidden Dependencies**
Code using singletons has hidden dependencies that aren't visible in interfaces.

**Solution:** Make dependencies explicit through constructor parameters.

### 4. **Scalability Issues**
Single instance can become a bottleneck in multi-threaded applications.

**Solution:** Consider object pooling or per-thread instances.

## Modern C++ Best Practices

### 1. Delete Copy Operations
```cpp
Singleton(const Singleton&) = delete;
Singleton& operator=(const Singleton&) = delete;
```

### 2. Use References Instead of Pointers
```cpp
// Preferred
Singleton& instance = Singleton::getInstance();

// Avoid
Singleton* instance = Singleton::getInstance();
```

### 3. Consider std::call_once for Complex Initialization
```cpp
class Singleton {
private:
    static std::unique_ptr<Singleton> instance;
    static std::once_flag initFlag;

public:
    static Singleton& getInstance() {
        std::call_once(initFlag, []() {
            instance = std::unique_ptr<Singleton>(new Singleton());
        });
        return *instance;
    }
};
```

## Alternatives to Consider

### 1. Dependency Injection
```cpp
class Service {
    Logger& logger;
public:
    Service(Logger& log) : logger(log) {}
};
```

### 2. Monostate Pattern
```cpp
class Monostate {
private:
    static int sharedData;
public:
    void setData(int data) { sharedData = data; }
    int getData() const { return sharedData; }
};
```

### 3. Namespace with Functions
```cpp
namespace Logger {
    void log(const std::string& message);
    void setLevel(LogLevel level);
}
```

## Interview Questions

**Q: What's the difference between Singleton and static class?**
A: Singleton is an object that can implement interfaces, be passed around, and support polymorphism. Static class is just a namespace with functions.

**Q: How do you make Singleton thread-safe?**
A: Use Meyer's Singleton (C++11+), std::call_once, or mutex with double-checked locking.

**Q: What are the problems with Singleton?**
A: Global state, testing difficulties, tight coupling, potential race conditions, destruction order issues.

**Q: When would you use Singleton?**
A: Logging, configuration, hardware access, connection pools - when you truly need exactly one instance.

## Performance Considerations
- Meyer's Singleton: ~0 overhead after first call
- Mutex-based: Lock overhead on every access
- Memory: Single instance saves memory vs multiple objects
- Cache: Single instance improves cache locality

## Conclusion
The Singleton pattern is powerful but should be used sparingly. Meyer's Singleton is the recommended approach in modern C++. Always consider if you truly need a singleton or if dependency injection would be better.
