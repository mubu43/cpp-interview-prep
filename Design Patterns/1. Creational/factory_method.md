# Factory Method Pattern Study Guide

## Overview
The Factory Method pattern provides an interface for creating objects, but lets subclasses decide which class to instantiate. It encapsulates object creation and promotes loose coupling.

## Intent
- Create objects without specifying their exact classes
- Delegate object creation to subclasses
- Provide a common interface for object creation
- Promote loose coupling between client and concrete classes

## Structure
```
Creator (Abstract)           Product (Abstract)
    |                            |
    +-- ConcreteCreator     +-- ConcreteProduct
    |   +-- factoryMethod() |   +-- operation()
    +-- ConcreteCreator2    +-- ConcreteProduct2
        +-- factoryMethod()     +-- operation()
```

## When to Use
✅ **Good for:**
- When you don't know beforehand the exact types of objects to create
- When you want to provide a library/framework for object creation
- When you need to extend object creation logic
- Plugin architectures
- Configuration-based object creation
- Testing (mock object creation)

❌ **Avoid when:**
- Object creation is simple and unlikely to change
- You have a small, fixed set of objects
- Performance is critical (adds indirection)

## Implementation Variants

### 1. Classic Factory Method
```cpp
class Creator {
public:
    virtual ~Creator() = default;
    virtual std::unique_ptr<Product> createProduct() = 0;
    
    void doSomething() {
        auto product = createProduct();
        product->operation();
    }
};

class ConcreteCreator : public Creator {
public:
    std::unique_ptr<Product> createProduct() override {
        return std::make_unique<ConcreteProduct>();
    }
};
```

### 2. Parameterized Factory Method
```cpp
class Factory {
public:
    virtual std::unique_ptr<Product> create(const std::string& type) = 0;
};

class ConcreteFactory : public Factory {
public:
    std::unique_ptr<Product> create(const std::string& type) override {
        if (type == "A") return std::make_unique<ProductA>();
        if (type == "B") return std::make_unique<ProductB>();
        return nullptr;
    }
};
```

### 3. Registration-Based Factory
```cpp
class Factory {
private:
    std::map<std::string, std::function<std::unique_ptr<Product>()>> creators_;

public:
    void registerCreator(const std::string& type, 
                        std::function<std::unique_ptr<Product>()> creator) {
        creators_[type] = creator;
    }
    
    std::unique_ptr<Product> create(const std::string& type) {
        auto it = creators_.find(type);
        return (it != creators_.end()) ? it->second() : nullptr;
    }
};
```

### 4. Template Factory
```cpp
template<typename BaseType>
class TemplateFactory {
private:
    std::map<std::string, std::function<std::unique_ptr<BaseType>()>> creators_;

public:
    template<typename ConcreteType>
    void registerType(const std::string& name) {
        creators_[name] = []() {
            return std::make_unique<ConcreteType>();
        };
    }
    
    std::unique_ptr<BaseType> create(const std::string& name) {
        auto it = creators_.find(name);
        return (it != creators_.end()) ? it->second() : nullptr;
    }
};
```

## Related Patterns

### Simple Factory (Not GoF)
```cpp
class SimpleFactory {
public:
    static std::unique_ptr<Product> create(ProductType type) {
        switch (type) {
            case ProductType::A: return std::make_unique<ProductA>();
            case ProductType::B: return std::make_unique<ProductB>();
            default: return nullptr;
        }
    }
};
```

### Abstract Factory
Creates families of related objects, while Factory Method creates single objects.

### Builder
Constructs complex objects step by step, while Factory Method creates objects in one call.

## Modern C++ Best Practices

### 1. Use Smart Pointers
```cpp
// Preferred
std::unique_ptr<Product> createProduct();

// Avoid raw pointers
Product* createProduct();
```

### 2. Use enum class for Type Safety
```cpp
enum class ProductType {
    TYPE_A,
    TYPE_B,
    TYPE_C
};

std::unique_ptr<Product> create(ProductType type);
```

### 3. Use std::function for Flexibility
```cpp
using CreatorFunction = std::function<std::unique_ptr<Product>()>;
std::map<std::string, CreatorFunction> creators_;
```

### 4. Use Perfect Forwarding for Constructor Arguments
```cpp
template<typename T, typename... Args>
std::unique_ptr<T> create(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}
```

### 5. Use SFINAE for Type Constraints
```cpp
template<typename T>
std::enable_if_t<std::is_base_of_v<Product, T>, std::unique_ptr<T>>
create() {
    return std::make_unique<T>();
}
```

## Common Pitfalls

### 1. **Overuse**
Don't use Factory Method for simple object creation.

**Bad:**
```cpp
auto string = StringFactory::create("Hello");  // Overkill
```

**Good:**
```cpp
std::string str = "Hello";  // Direct creation is fine
```

### 2. **Not Using Virtual Destructors**
Always use virtual destructors in base classes.

```cpp
class Product {
public:
    virtual ~Product() = default;  // Essential!
};
```

### 3. **Returning Raw Pointers**
Always return smart pointers to manage ownership.

### 4. **Not Handling Unknown Types**
Always check for nullptr when creating objects.

```cpp
auto product = factory.create("unknown");
if (product) {
    product->operation();
} else {
    // Handle error
}
```

## Advanced Techniques

### 1. Self-Registering Factory
```cpp
template<typename BaseType>
class SelfRegisteringFactory {
private:
    static std::map<std::string, std::function<std::unique_ptr<BaseType>()>>& getCreators() {
        static std::map<std::string, std::function<std::unique_ptr<BaseType>()>> creators;
        return creators;
    }

public:
    template<typename ConcreteType>
    static bool registerType(const std::string& name) {
        getCreators()[name] = []() {
            return std::make_unique<ConcreteType>();
        };
        return true;
    }
    
    static std::unique_ptr<BaseType> create(const std::string& name) {
        auto& creators = getCreators();
        auto it = creators.find(name);
        return (it != creators.end()) ? it->second() : nullptr;
    }
};

// Auto-registration macro
#define REGISTER_TYPE(BaseType, ConcreteType, Name) \
    static bool registered_##ConcreteType = \
        SelfRegisteringFactory<BaseType>::registerType<ConcreteType>(Name);
```

### 2. Factory with Dependency Injection
```cpp
class Factory {
private:
    std::shared_ptr<Database> db_;
    std::shared_ptr<Logger> logger_;

public:
    Factory(std::shared_ptr<Database> db, std::shared_ptr<Logger> logger)
        : db_(db), logger_(logger) {}
        
    std::unique_ptr<Product> create(const std::string& type) {
        auto product = createBasedOnType(type);
        if (product) {
            product->setDatabase(db_);
            product->setLogger(logger_);
        }
        return product;
    }
};
```

## Performance Considerations

### Memory
- Smart pointers: Small overhead for automatic cleanup
- Registration map: Memory overhead for function objects
- Virtual calls: Minimal overhead in modern CPUs

### Speed
- Virtual function calls: ~1-2 CPU cycles overhead
- Map lookup: O(log n) for std::map, O(1) average for std::unordered_map
- Function object calls: Optimized by compiler

### Optimization Tips
```cpp
// Use unordered_map for O(1) lookup
std::unordered_map<std::string, CreatorFunction> creators_;

// Pre-allocate known types
factory.reserve(expectedTypeCount);

// Use string_view to avoid string copies
std::unique_ptr<Product> create(std::string_view type);
```

## Testing Strategies

### 1. Mock Factories
```cpp
class MockFactory : public Factory {
public:
    MOCK_METHOD(std::unique_ptr<Product>, create, (const std::string&), (override));
};
```

### 2. Test Factory Registration
```cpp
TEST(FactoryTest, RegisterAndCreate) {
    Factory factory;
    factory.registerType<ConcreteProduct>("test");
    
    auto product = factory.create("test");
    ASSERT_NE(product, nullptr);
    ASSERT_EQ(product->getType(), "ConcreteProduct");
}
```

## Interview Questions

**Q: Difference between Simple Factory and Factory Method?**
A: Simple Factory uses a static method with conditional logic. Factory Method uses inheritance and virtual methods for extensibility.

**Q: When would you use Factory Method over Constructor?**
A: When object creation logic is complex, when you need to create different types based on runtime conditions, or when you want to decouple client from concrete classes.

**Q: How does Factory Method promote Open/Closed Principle?**
A: You can add new product types by creating new concrete creators without modifying existing code.

**Q: What's the difference between Factory Method and Abstract Factory?**
A: Factory Method creates one product type, Abstract Factory creates families of related products.

## Real-World Examples
- **GUI Frameworks**: Creating platform-specific widgets
- **Database Drivers**: Creating connection objects for different databases
- **Game Development**: Creating different enemy types
- **Plugin Systems**: Loading and creating plugin instances
- **Serialization**: Creating serializers for different formats

## Conclusion
Factory Method is excellent for creating objects when you need flexibility and extensibility. Use it when object creation logic is complex or when you need to support different types determined at runtime. The registration-based approach is particularly powerful in modern C++.
