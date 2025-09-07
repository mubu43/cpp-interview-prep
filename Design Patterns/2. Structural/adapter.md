# Adapter Pattern Study Guide

## Overview
The Adapter pattern allows objects with incompatible interfaces to work together. It acts as a bridge between two incompatible interfaces by wrapping an existing class with a new interface.

## Intent
- Make incompatible interfaces compatible
- Allow existing classes to work with others without modifying their source code
- Provide a unified interface to a set of classes with different interfaces
- Enable integration of third-party libraries

## Structure
```
Client  -->  Target Interface  -->  Adapter  -->  Adaptee
                    |                  |              |
               (what client       (converts)    (existing
                expects)                        incompatible
                                                interface)
```

## When to Use
✅ **Good for:**
- Integrating third-party libraries with incompatible interfaces
- Legacy system integration
- Making incompatible classes work together
- Standardizing interfaces across different implementations
- Gradual system migration and refactoring
- Creating unified APIs from multiple data sources

❌ **Avoid when:**
- You can modify the existing classes directly
- The interfaces are already compatible
- The adaptation adds significant complexity
- Performance overhead is unacceptable

## Implementation Types

### 1. Object Adapter (Composition)
```cpp
class Adapter : public Target {
private:
    std::unique_ptr<Adaptee> adaptee_;

public:
    Adapter() : adaptee_(std::make_unique<Adaptee>()) {}
    
    void request() override {
        // Translate target interface to adaptee interface
        adaptee_->specificRequest();
    }
};
```

**Pros:**
- Uses composition (preferred)
- Can adapt multiple adaptees
- Runtime flexibility
- Follows composition over inheritance

**Cons:**
- Slight performance overhead
- More complex object relationships

### 2. Class Adapter (Inheritance)
```cpp
class Adapter : public Target, private Adaptee {
public:
    void request() override {
        // Direct access to adaptee methods
        specificRequest();
    }
};
```

**Pros:**
- Direct method access
- Better performance
- Simpler object structure

**Cons:**
- Multiple inheritance complexity
- Less flexible
- Tight coupling

## Common Adapter Scenarios

### 1. Media Player Adapter
```cpp
// Target interface
class MediaPlayer {
public:
    virtual void play(const std::string& format, const std::string& file) = 0;
};

// Adaptees with different interfaces
class Mp4Player {
public:
    void playMp4(const std::string& file);
};

class VlcPlayer {
public:
    void playVlcMedia(const std::string& file);
};

// Adapter
class MediaAdapter : public MediaPlayer {
    std::unique_ptr<Mp4Player> mp4Player_;
    std::unique_ptr<VlcPlayer> vlcPlayer_;
    
public:
    void play(const std::string& format, const std::string& file) override {
        if (format == "mp4") {
            mp4Player_->playMp4(file);
        } else if (format == "vlc") {
            vlcPlayer_->playVlcMedia(file);
        }
    }
};
```

### 2. Database Adapter
```cpp
// Target interface
class Database {
public:
    virtual void connect(const std::string& connectionString) = 0;
    virtual std::string query(const std::string& sql) = 0;
};

// Legacy database with different interface
class LegacyDatabase {
public:
    void establishConnection(const char* host, int port, const char* db);
    char* executeQuery(const char* query);
};

// Adapter
class DatabaseAdapter : public Database {
    LegacyDatabase legacyDb_;
    
public:
    void connect(const std::string& connectionString) override {
        // Parse connection string and adapt
        auto [host, port, db] = parseConnectionString(connectionString);
        legacyDb_.establishConnection(host.c_str(), port, db.c_str());
    }
    
    std::string query(const std::string& sql) override {
        char* result = legacyDb_.executeQuery(sql.c_str());
        std::string stdResult(result);
        free(result);  // Clean up C-style memory
        return stdResult;
    }
};
```

### 3. Graphics Library Adapter
```cpp
// Modern graphics interface
class Shape {
public:
    virtual void draw() = 0;
    virtual void getBounds() = 0;
};

// Legacy graphics library
class LegacyLine {
public:
    void drawLine(int x1, int y1, int x2, int y2);
};

// Adapter
class LineAdapter : public Shape, private LegacyLine {
    int x1_, y1_, x2_, y2_;
    
public:
    LineAdapter(int x1, int y1, int x2, int y2) 
        : x1_(x1), y1_(y1), x2_(x2), y2_(y2) {}
    
    void draw() override {
        drawLine(x1_, y1_, x2_, y2_);
    }
    
    void getBounds() override {
        // Calculate and return bounds
    }
};
```

## Modern C++ Techniques

### 1. Template Adapters
```cpp
template<typename Adaptee>
class GenericAdapter {
    Adaptee adaptee_;
    
public:
    template<typename... Args>
    GenericAdapter(Args&&... args) : adaptee_(std::forward<Args>(args)...) {}
    
    void execute() {
        if constexpr (std::is_same_v<Adaptee, Mp4Player>) {
            adaptee_.playMp4("default.mp4");
        } else if constexpr (std::is_same_v<Adaptee, VlcPlayer>) {
            adaptee_.playVlcMedia("default.vlc");
        }
    }
};
```

### 2. Function Adapters with std::function
```cpp
class FunctionAdapter {
    std::function<void(const std::string&)> playFunction_;
    
public:
    template<typename Player>
    FunctionAdapter(Player& player) {
        if constexpr (std::is_same_v<Player, Mp3Player>) {
            playFunction_ = [&](const std::string& file) {
                player.playMp3(file);
            };
        }
        // ... other player types
    }
    
    void play(const std::string& file) {
        if (playFunction_) playFunction_(file);
    }
};
```

### 3. SFINAE-based Adapters
```cpp
template<typename T>
class SmartAdapter {
private:
    T adaptee_;
    
    template<typename U = T>
    auto callPlay(const std::string& file) 
        -> decltype(std::declval<U>().play(file), void()) {
        adaptee_.play(file);
    }
    
    template<typename U = T>
    auto callPlay(const std::string& file) 
        -> decltype(std::declval<U>().playFile(file), void()) {
        adaptee_.playFile(file);
    }
    
public:
    void play(const std::string& file) {
        callPlay(file);
    }
};
```

### 4. Concept-based Adapters (C++20)
```cpp
template<typename T>
concept Playable = requires(T t, const std::string& file) {
    t.play(file);
} || requires(T t, const std::string& file) {
    t.playFile(file);
};

template<Playable T>
class ConceptAdapter {
    T adaptee_;
    
public:
    void play(const std::string& file) {
        if constexpr (requires { adaptee_.play(file); }) {
            adaptee_.play(file);
        } else {
            adaptee_.playFile(file);
        }
    }
};
```

## Advanced Adapter Patterns

### 1. Two-Way Adapter
```cpp
class TwoWayAdapter : public ModernInterface, public LegacyInterface {
    ModernImpl modern_;
    LegacyImpl legacy_;
    
public:
    // Modern interface implementation
    void modernMethod() override {
        modern_.doModernThing();
    }
    
    // Legacy interface implementation
    void legacyMethod() override {
        legacy_.doLegacyThing();
    }
    
    // Adapter methods
    void adaptModernToLegacy() {
        // Convert modern call to legacy
        legacy_.doLegacyThing();
    }
    
    void adaptLegacyToModern() {
        // Convert legacy call to modern
        modern_.doModernThing();
    }
};
```

### 2. Pluggable Adapter
```cpp
class PluggableAdapter {
private:
    std::map<std::string, std::function<void(const std::string&)>> adapters_;
    
public:
    template<typename T>
    void registerAdapter(const std::string& type, T& adaptee) {
        adapters_[type] = [&adaptee](const std::string& data) {
            if constexpr (requires { adaptee.process(data); }) {
                adaptee.process(data);
            } else if constexpr (requires { adaptee.handle(data); }) {
                adaptee.handle(data);
            }
        };
    }
    
    void execute(const std::string& type, const std::string& data) {
        auto it = adapters_.find(type);
        if (it != adapters_.end()) {
            it->second(data);
        }
    }
};
```

### 3. Decorator-Adapter Combination
```cpp
template<typename Component>
class LoggingAdapter : public Component {
private:
    std::string logPrefix_;
    
public:
    template<typename... Args>
    LoggingAdapter(const std::string& prefix, Args&&... args)
        : Component(std::forward<Args>(args)...), logPrefix_(prefix) {}
    
    void operation() override {
        std::cout << logPrefix_ << ": Starting operation" << std::endl;
        Component::operation();
        std::cout << logPrefix_ << ": Operation completed" << std::endl;
    }
};
```

## Performance Considerations

### Object vs Class Adapter Performance
```cpp
// Object Adapter - indirect call through pointer
class ObjectAdapter : public Target {
    std::unique_ptr<Adaptee> adaptee_;
public:
    void request() override {
        adaptee_->specificRequest();  // Indirect call
    }
};

// Class Adapter - direct call
class ClassAdapter : public Target, private Adaptee {
public:
    void request() override {
        specificRequest();  // Direct call
    }
};
```

### Memory Overhead
- **Object Adapter**: Additional pointer storage (8 bytes)
- **Class Adapter**: No additional memory overhead
- **Template Adapter**: Zero overhead if inlined

### Call Overhead
- **Virtual calls**: ~1-2 CPU cycles
- **Function object calls**: Often inlined
- **Template calls**: Usually zero overhead

## Common Pitfalls

### 1. **Over-adaptation**
```cpp
// Bad - unnecessary adapter
class SimpleAdapter : public Target {
    Adaptee adaptee_;
public:
    void request() override {
        adaptee_.request();  // Same method name - no adaptation needed!
    }
};

// Good - direct usage
Target* target = &adaptee;  // If interfaces are compatible
```

### 2. **Leaky Abstractions**
```cpp
// Bad - exposes adaptee details
class LeakyAdapter : public Target {
public:
    void request() override { /* ... */ }
    Adaptee& getAdaptee() { return adaptee_; }  // Breaks encapsulation
};

// Good - proper encapsulation
class ProperAdapter : public Target {
public:
    void request() override { /* ... */ }
    // No direct access to adaptee
};
```

### 3. **Inefficient Conversions**
```cpp
// Bad - expensive conversions on every call
class IneffientAdapter : public Target {
public:
    void request(const std::string& data) override {
        // Converting string to char* on every call
        std::vector<char> buffer(data.begin(), data.end());
        buffer.push_back('\0');
        adaptee_.legacyRequest(buffer.data());
    }
};

// Good - cache or optimize conversions
class EfficientAdapter : public Target {
    mutable std::vector<char> buffer_;
public:
    void request(const std::string& data) override {
        buffer_.assign(data.begin(), data.end());
        buffer_.push_back('\0');
        adaptee_.legacyRequest(buffer_.data());
    }
};
```

### 4. **Resource Management Issues**
```cpp
// Bad - potential memory leaks
class BadAdapter : public Target {
    Adaptee* adaptee_;
public:
    BadAdapter() : adaptee_(new Adaptee()) {}
    // Missing destructor - memory leak!
};

// Good - RAII
class GoodAdapter : public Target {
    std::unique_ptr<Adaptee> adaptee_;
public:
    GoodAdapter() : adaptee_(std::make_unique<Adaptee>()) {}
    // Automatic cleanup
};
```

## Testing Strategies

### 1. Test Interface Compatibility
```cpp
TEST(AdapterTest, InterfaceCompatibility) {
    MockAdaptee mockAdaptee;
    Adapter adapter(mockAdaptee);
    
    EXPECT_CALL(mockAdaptee, specificRequest()).Times(1);
    adapter.request();
}
```

### 2. Test Data Conversion
```cpp
TEST(AdapterTest, DataConversion) {
    TestAdaptee adaptee;
    Adapter adapter(adaptee);
    
    std::string input = "test data";
    adapter.processData(input);
    
    EXPECT_EQ(adaptee.getLastProcessedData(), "test data");
}
```

### 3. Test Error Handling
```cpp
TEST(AdapterTest, ErrorHandling) {
    FailingAdaptee adaptee;
    Adapter adapter(adaptee);
    
    EXPECT_THROW(adapter.request(), AdapterException);
}
```

## Real-World Examples

### GUI Framework Adaptation
```cpp
// Adapting Win32 API to modern C++ GUI framework
class Win32WindowAdapter : public Window {
    HWND hwnd_;
    
public:
    void show() override {
        ShowWindow(hwnd_, SW_SHOW);
    }
    
    void hide() override {
        ShowWindow(hwnd_, SW_HIDE);
    }
    
    void setTitle(const std::string& title) override {
        SetWindowTextA(hwnd_, title.c_str());
    }
};
```

### Network Protocol Adaptation
```cpp
// Adapting different network protocols to common interface
class HttpAdapter : public NetworkClient {
    HttpLibrary httpLib_;
    
public:
    std::string request(const std::string& url) override {
        auto response = httpLib_.get(url);
        return response.body();
    }
};

class WebSocketAdapter : public NetworkClient {
    WebSocketLibrary wsLib_;
    
public:
    std::string request(const std::string& url) override {
        wsLib_.connect(url);
        auto message = wsLib_.receive();
        wsLib_.disconnect();
        return message;
    }
};
```

## Interview Questions

**Q: When would you use Adapter pattern?**
A: When you need to integrate classes with incompatible interfaces, especially third-party libraries, legacy systems, or when standardizing interfaces across different implementations.

**Q: Difference between Adapter and Facade?**
A: Adapter makes incompatible interfaces compatible (1-to-1 mapping), while Facade provides a simplified interface to a complex subsystem (1-to-many mapping).

**Q: Object Adapter vs Class Adapter?**
A: Object Adapter uses composition (flexible, preferred), Class Adapter uses inheritance (faster, but requires multiple inheritance in C++).

**Q: Can Adapter change the interface?**
A: Yes, Adapter can translate between different interfaces, including changing method signatures, parameter types, and calling conventions.

**Q: Performance implications of Adapter?**
A: Minimal overhead - one additional indirection for Object Adapter, zero overhead for Class Adapter if methods are inlined.

## Conclusion
The Adapter pattern is essential for system integration and legacy code maintenance. It enables:
- Seamless third-party library integration
- Legacy system modernization
- Interface standardization
- Code reuse without modification

Use Object Adapter (composition) as the default choice, and consider Class Adapter only when performance is critical and multiple inheritance is acceptable.
