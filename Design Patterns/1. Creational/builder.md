# Builder Pattern Study Guide

## Overview
The Builder pattern constructs complex objects step by step. It separates the construction of a complex object from its representation, allowing the same construction process to create different representations.

## Intent
- Construct complex objects step by step
- Separate construction logic from representation
- Allow different representations of the same construction process
- Provide fine control over the construction process

## Structure
```
Director                    Builder (Abstract)
+-- construct()            +-- buildPartA()
                          +-- buildPartB()
                          +-- getResult()
                               |
                          ConcreteBuilder
                          +-- buildPartA()
                          +-- buildPartB()
                          +-- getResult()
                               |
                          Product
```

## When to Use
✅ **Good for:**
- Creating complex objects with many optional parameters
- Objects that require step-by-step construction
- When you need different representations of the same object
- Immutable objects with many fields
- Configuration objects
- SQL query builders
- GUI component construction

❌ **Avoid when:**
- Simple objects with few parameters
- Objects with fixed structure
- Performance is critical (adds overhead)
- Construction logic is unlikely to change

## Implementation Variants

### 1. Classic Builder with Director
```cpp
class ProductBuilder {
public:
    virtual ~ProductBuilder() = default;
    virtual void buildPartA() = 0;
    virtual void buildPartB() = 0;
    virtual std::unique_ptr<Product> getResult() = 0;
};

class Director {
    ProductBuilder* builder_;
public:
    void setBuilder(ProductBuilder* builder) { builder_ = builder; }
    
    std::unique_ptr<Product> construct() {
        builder_->buildPartA();
        builder_->buildPartB();
        return builder_->getResult();
    }
};
```

### 2. Fluent Builder (Method Chaining)
```cpp
class FluentBuilder {
    std::unique_ptr<Product> product_;
    
public:
    FluentBuilder() : product_(std::make_unique<Product>()) {}
    
    FluentBuilder& withPartA(const std::string& partA) {
        product_->setPartA(partA);
        return *this;
    }
    
    FluentBuilder& withPartB(int partB) {
        product_->setPartB(partB);
        return *this;
    }
    
    std::unique_ptr<Product> build() {
        return std::move(product_);
    }
};

// Usage
auto product = FluentBuilder()
    .withPartA("value")
    .withPartB(42)
    .build();
```

### 3. Telescoping Constructor Problem Solution
```cpp
// Before Builder (Telescoping Constructor Problem)
class BadConfig {
public:
    BadConfig(std::string host) { /*...*/ }
    BadConfig(std::string host, int port) { /*...*/ }
    BadConfig(std::string host, int port, bool ssl) { /*...*/ }
    BadConfig(std::string host, int port, bool ssl, int timeout) { /*...*/ }
    // ... many more constructors
};

// After Builder
class Config {
    // Many private members
public:
    class Builder {
        Config config_;
    public:
        Builder& host(const std::string& h) { config_.host_ = h; return *this; }
        Builder& port(int p) { config_.port_ = p; return *this; }
        Builder& ssl(bool s) { config_.ssl_ = s; return *this; }
        Builder& timeout(int t) { config_.timeout_ = t; return *this; }
        Config build() { return std::move(config_); }
    };
};
```

### 4. Template Builder
```cpp
template<typename T>
class TemplateBuilder {
    T object_;
    
public:
    template<typename U>
    TemplateBuilder& set(U T::*member, const U& value) {
        object_.*member = value;
        return *this;
    }
    
    T build() {
        return std::move(object_);
    }
};
```

## Modern C++ Enhancements

### 1. Move Semantics
```cpp
class Builder {
    std::unique_ptr<Product> product_;
    
public:
    Builder& withData(std::string data) {
        product_->setData(std::move(data));  // Move instead of copy
        return *this;
    }
    
    std::unique_ptr<Product> build() && {  // Ref-qualified for rvalue
        return std::move(product_);
    }
};
```

### 2. Perfect Forwarding
```cpp
template<typename... Args>
Builder& addComponent(Args&&... args) {
    product_->addComponent(std::forward<Args>(args)...);
    return *this;
}
```

### 3. SFINAE for Type Safety
```cpp
template<typename T>
std::enable_if_t<std::is_arithmetic_v<T>, Builder&>
setValue(T value) {
    product_->setValue(value);
    return *this;
}
```

### 4. Compile-time Validation
```cpp
template<bool HasRequiredField = false>
class TypedBuilder {
public:
    auto withRequired(const std::string& value) {
        return TypedBuilder<true>{product_.withRequired(value)};
    }
    
    // Only available when required field is set
    template<bool H = HasRequiredField>
    std::enable_if_t<H, Product> build() {
        return product_;
    }
};
```

## Advanced Techniques

### 1. Nested Builders
```cpp
class QueryBuilder {
public:
    class WhereBuilder {
        QueryBuilder& parent_;
        
    public:
        WhereBuilder(QueryBuilder& parent) : parent_(parent) {}
        
        WhereBuilder& equals(const std::string& field, const std::string& value) {
            parent_.addCondition(field + " = " + value);
            return *this;
        }
        
        QueryBuilder& endWhere() {
            return parent_;
        }
    };
    
    WhereBuilder where() {
        return WhereBuilder(*this);
    }
    
    std::string build() { return query_; }
};

// Usage
auto query = QueryBuilder()
    .select("name, age")
    .from("users")
    .where()
        .equals("status", "active")
        .equals("role", "admin")
    .endWhere()
    .build();
```

### 2. Builder Validation
```cpp
class ValidatingBuilder {
    bool hasRequired_ = false;
    
public:
    Builder& withRequired(const std::string& value) {
        hasRequired_ = true;
        return *this;
    }
    
    Product build() {
        if (!hasRequired_) {
            throw std::runtime_error("Required field not set");
        }
        return createProduct();
    }
};
```

### 3. Immutable Builder Results
```cpp
class ImmutableProduct {
    const std::string name_;
    const int value_;
    
public:
    ImmutableProduct(std::string name, int value)
        : name_(std::move(name)), value_(value) {}
        
    // Only getters, no setters
    const std::string& getName() const { return name_; }
    int getValue() const { return value_; }
};

class ImmutableBuilder {
    std::string name_;
    int value_ = 0;
    
public:
    ImmutableBuilder& withName(std::string name) {
        name_ = std::move(name);
        return *this;
    }
    
    ImmutableBuilder& withValue(int value) {
        value_ = value;
        return *this;
    }
    
    ImmutableProduct build() {
        return ImmutableProduct{std::move(name_), value_};
    }
};
```

## Common Pitfalls

### 1. **Forgetting to Reset Builder State**
```cpp
// Bad - reusing builder without reset
auto product1 = builder.withA("a").build();
auto product2 = builder.withB("b").build();  // Still has "a" from before!

// Good - reset between uses
builder.reset();
auto product2 = builder.withB("b").build();
```

### 2. **Not Making Build() Move-Only**
```cpp
// Bad - allows multiple calls to build()
Product build() { return product_; }

// Good - move-only build
std::unique_ptr<Product> build() && {
    return std::move(product_);
}
```

### 3. **Missing Validation**
```cpp
// Always validate required fields
Product build() {
    validate();  // Check required fields
    return std::move(product_);
}
```

### 4. **Thread Safety Issues**
Builders are typically not thread-safe. Don't share builder instances across threads.

## Performance Considerations

### Memory
- Each builder holds product state during construction
- Consider object pooling for frequently created objects
- Use move semantics to avoid copies

### Speed
- Method chaining: Minimal overhead
- Validation: Add overhead but prevents errors
- Complex builders: More overhead than simple constructors

### Optimization Tips
```cpp
// Reserve collections if size is known
builder.reserveComponents(expectedSize);

// Use string_view for temporary strings
Builder& withName(std::string_view name);

// Avoid unnecessary copies
Builder& withData(std::vector<int> data) {
    product_->setData(std::move(data));
    return *this;
}
```

## Testing Strategies

### 1. Test Builder Validation
```cpp
TEST(BuilderTest, ThrowsOnMissingRequired) {
    Builder builder;
    EXPECT_THROW(builder.build(), std::runtime_error);
}
```

### 2. Test Method Chaining
```cpp
TEST(BuilderTest, MethodChaining) {
    auto product = Builder()
        .withName("test")
        .withValue(42)
        .build();
    
    EXPECT_EQ(product->getName(), "test");
    EXPECT_EQ(product->getValue(), 42);
}
```

### 3. Test Builder Reuse
```cpp
TEST(BuilderTest, BuilderReuse) {
    Builder builder;
    
    auto product1 = builder.withName("first").build();
    builder.reset();
    auto product2 = builder.withName("second").build();
    
    EXPECT_NE(product1->getName(), product2->getName());
}
```

## Real-World Examples

### HTTP Request Builder
```cpp
auto request = HttpRequestBuilder()
    .method("POST")
    .url("https://api.example.com/users")
    .header("Content-Type", "application/json")
    .header("Authorization", "Bearer token")
    .body(R"({"name": "John", "age": 30})")
    .timeout(std::chrono::seconds(30))
    .build();
```

### Configuration Builder
```cpp
auto config = DatabaseConfigBuilder()
    .host("localhost")
    .port(5432)
    .database("myapp")
    .username("user")
    .password("pass")
    .poolSize(10)
    .enableSSL()
    .connectionTimeout(std::chrono::seconds(5))
    .build();
```

## Interview Questions

**Q: When would you use Builder over Constructor?**
A: When you have many optional parameters, when construction is complex, or when you need different representations of the same object.

**Q: Difference between Builder and Factory Method?**
A: Builder constructs objects step-by-step with fine control, Factory Method creates objects in one call with a focus on which type to create.

**Q: How do you ensure required fields in Builder?**
A: Use validation in build() method, template types for compile-time checks, or return different builder types after setting required fields.

**Q: Is Builder thread-safe?**
A: No, builders maintain state and are not thread-safe. Each thread should have its own builder instance.

## Conclusion
The Builder pattern is excellent for creating complex objects with many optional parameters. The fluent interface makes code readable and maintainable. Use it when constructors become unwieldy or when you need step-by-step object construction. Modern C++ features like move semantics and perfect forwarding make builders more efficient and expressive.
