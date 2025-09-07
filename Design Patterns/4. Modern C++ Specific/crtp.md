# CRTP (Curiously Recurring Template Pattern) Study Guide

## Overview
CRTP is a C++ idiom where a class inherits from a template instantiation of itself. The template parameter is the derived class, creating a form of "static polymorphism" that achieves polymorphic behavior without virtual functions.

## Basic Structure
```cpp
template<typename Derived>
class Base {
public:
    void interface() {
        static_cast<Derived*>(this)->implementation();
    }
};

class Derived : public Base<Derived> {
public:
    void implementation() {
        // Actual implementation
    }
};
```

## When to Use
✅ **Good for:**
- High-performance polymorphism (avoiding virtual function overhead)
- Compile-time polymorphism
- Code reuse with zero runtime cost
- Library design (e.g., iterator facades, expression templates)
- Mixin patterns
- Static interface checking
- Template metaprogramming

❌ **Avoid when:**
- Runtime polymorphism is needed
- Different types need to be stored in same container
- Simplicity is more important than performance
- Interface might change frequently

## Common CRTP Use Cases

### 1. Static Polymorphism
```cpp
template<typename Derived>
class Shape {
public:
    void draw() {
        static_cast<Derived*>(this)->drawImpl();
    }
    
    double area() {
        return static_cast<Derived*>(this)->areaImpl();
    }
};

class Circle : public Shape<Circle> {
public:
    void drawImpl() { /* circle drawing */ }
    double areaImpl() { return 3.14159 * radius_ * radius_; }
private:
    double radius_;
};
```

**Benefits:**
- No virtual function call overhead
- Compile-time method resolution
- Better inlining opportunities

### 2. Mixin Pattern
```cpp
template<typename Derived>
class Comparable {
public:
    bool operator!=(const Derived& other) const {
        return !static_cast<const Derived*>(this)->operator==(other);
    }
    
    bool operator<=(const Derived& other) const {
        const auto& self = static_cast<const Derived&>(*this);
        return self < other || self == other;
    }
    
    // ... other operators
};

class Number : public Comparable<Number> {
public:
    bool operator==(const Number& other) const;
    bool operator<(const Number& other) const;
    // Get other operators for free!
};
```

### 3. Singleton Pattern
```cpp
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

class Logger : public Singleton<Logger> {
    friend class Singleton<Logger>;
private:
    Logger() = default;
public:
    void log(const std::string& msg);
};
```

### 4. Fluent Interface (Method Chaining)
```cpp
template<typename Derived>
class Builder {
public:
    Derived& name(const std::string& name) {
        name_ = name;
        return static_cast<Derived&>(*this);
    }
    
    Derived& id(int id) {
        id_ = id;
        return static_cast<Derived&>(*this);
    }

protected:
    std::string name_;
    int id_;
};

class PersonBuilder : public Builder<PersonBuilder> {
public:
    PersonBuilder& age(int age) {
        age_ = age;
        return *this;
    }
    
    Person build();
    
private:
    int age_;
};

// Usage: PersonBuilder().name("Alice").id(123).age(25).build()
```

### 5. Instance Counting
```cpp
template<typename T>
class InstanceCounter {
private:
    static std::atomic<size_t> count_;

protected:
    InstanceCounter() { ++count_; }
    InstanceCounter(const InstanceCounter&) { ++count_; }
    ~InstanceCounter() { --count_; }

public:
    static size_t getInstanceCount() { return count_; }
};

template<typename T>
std::atomic<size_t> InstanceCounter<T>::count_{0};

class Widget : public InstanceCounter<Widget> {
    // Widget automatically tracks instance count
};
```

## Advanced CRTP Techniques

### 1. Compile-Time Interface Checking
```cpp
template<typename Derived>
class Drawable {
public:
    void render() {
        static_assert(std::is_same_v<
            decltype(std::declval<Derived>().draw()), void>,
            "Derived must have void draw() method");
        
        static_cast<Derived*>(this)->draw();
    }
};
```

### 2. Expression Templates
```cpp
template<typename Derived>
class VectorExpression {
public:
    const Derived& derived() const {
        return static_cast<const Derived&>(*this);
    }
    
    Derived& derived() {
        return static_cast<Derived&>(*this);
    }
};

template<typename LHS, typename RHS>
class VectorAdd : public VectorExpression<VectorAdd<LHS, RHS>> {
    const LHS& lhs_;
    const RHS& rhs_;
    
public:
    VectorAdd(const LHS& lhs, const RHS& rhs) : lhs_(lhs), rhs_(rhs) {}
    
    auto operator[](size_t i) const {
        return lhs_[i] + rhs_[i];
    }
    
    size_t size() const { return lhs_.size(); }
};

// Enables efficient chaining: v1 + v2 + v3 without temporaries
```

### 3. Policy-Based Design with CRTP
```cpp
template<typename Derived, typename AllocPolicy, typename ThreadPolicy>
class Container : public AllocPolicy, public ThreadPolicy {
public:
    void add(const typename AllocPolicy::value_type& item) {
        typename ThreadPolicy::Lock lock(this->getMutex());
        this->allocate_and_add(item);
    }
    
protected:
    Derived& derived() { return static_cast<Derived&>(*this); }
};

class MyContainer : public Container<MyContainer, PoolAllocator<int>, ThreadSafe> {
    // Inherits allocation and threading policies
};
```

### 4. Visitor Pattern with CRTP
```cpp
template<typename Derived>
class Visitable {
public:
    template<typename Visitor>
    auto accept(Visitor&& visitor) {
        return visitor.visit(static_cast<Derived&>(*this));
    }
};

class Circle : public Visitable<Circle> {
    // Automatically gets accept() method
};

class Rectangle : public Visitable<Rectangle> {
    // Automatically gets accept() method
};
```

## Performance Benefits

### Runtime Comparison
```cpp
// Virtual function approach
class VirtualBase {
public:
    virtual void operation() = 0;
};

class VirtualDerived : public VirtualBase {
public:
    void operation() override { /* implementation */ }
};

// CRTP approach
template<typename Derived>
class CRTPBase {
public:
    void operation() {
        static_cast<Derived*>(this)->operation();
    }
};

class CRTPDerived : public CRTPBase<CRTPDerived> {
public:
    void operation() { /* implementation */ }
};
```

**Performance characteristics:**
- **Virtual calls**: ~1-3 CPU cycles overhead per call
- **CRTP calls**: 0 overhead (inlined at compile time)
- **Memory**: Virtual functions require vtable pointer (8 bytes per object)
- **CRTP**: No runtime memory overhead

### Benchmark Example
```cpp
void benchmark() {
    constexpr size_t iterations = 10'000'000;
    
    // CRTP version
    CRTPDerived crtp_obj;
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        crtp_obj.operation();  // Inlined
    }
    auto crtp_time = std::chrono::high_resolution_clock::now() - start;
    
    // Virtual version
    std::unique_ptr<VirtualBase> virtual_obj = std::make_unique<VirtualDerived>();
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        virtual_obj->operation();  // Virtual call
    }
    auto virtual_time = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "CRTP: " << crtp_time.count() << "ns\n";
    std::cout << "Virtual: " << virtual_time.count() << "ns\n";
}
```

## Common Pitfalls and Solutions

### 1. **Slicing Problem**
```cpp
// Bad - slicing occurs
template<typename Derived>
class Base {
public:
    void print() {
        static_cast<Derived*>(this)->printImpl();
    }
};

Base<Derived> obj = derivedObj;  // Slicing! Lost derived part
obj.print();  // Undefined behavior

// Solution - make base class non-copyable or use references
template<typename Derived>
class Base {
public:
    Base(const Base&) = delete;
    Base& operator=(const Base&) = delete;
};
```

### 2. **Incorrect Template Parameter**
```cpp
// Bad - wrong template parameter
class Circle : public Shape<Rectangle> {  // Wrong!
    // ...
};

// Solution - use static_assert
template<typename Derived>
class Shape {
public:
    static_assert(std::is_base_of_v<Shape<Derived>, Derived>,
                  "Template parameter must be the derived class");
};
```

### 3. **Premature Static Cast**
```cpp
// Bad - casting before object is fully constructed
template<typename Derived>
class Base {
public:
    Base() {
        static_cast<Derived*>(this)->init();  // Dangerous!
    }
};

// Solution - use two-phase initialization
template<typename Derived>
class Base {
public:
    void initialize() {
        static_cast<Derived*>(this)->init();
    }
};
```

### 4. **Template Instantiation Issues**
```cpp
// Bad - incomplete type issues
template<typename Derived>
class Base {
public:
    void method() {
        sizeof(Derived);  // May fail if Derived is incomplete
    }
};

// Solution - delay instantiation
template<typename Derived>
class Base {
public:
    void method() {
        static_assert(sizeof(Derived) > 0);  // Triggers at instantiation
        // Implementation
    }
};
```

## Modern C++ Enhancements

### C++14 and Later Features
```cpp
// C++14: Auto return type deduction
template<typename Derived>
class Base {
public:
    auto getValue() {
        return static_cast<Derived*>(this)->getValueImpl();
    }
};

// C++17: if constexpr
template<typename Derived>
class Base {
public:
    void process() {
        if constexpr (has_preprocess_v<Derived>) {
            static_cast<Derived*>(this)->preprocess();
        }
        static_cast<Derived*>(this)->processImpl();
    }
};

// C++20: Concepts
template<typename T>
concept CRTPDerived = requires(T t) {
    t.implementation();
    std::is_base_of_v<Base<T>, T>;
};

template<CRTPDerived Derived>
class Base {
    // Concept ensures proper CRTP usage
};
```

### SFINAE with CRTP
```cpp
template<typename Derived>
class Base {
public:
    // Only enabled if Derived has serialize() method
    template<typename D = Derived>
    auto serialize() -> decltype(std::declval<D>().serialize()) {
        return static_cast<Derived*>(this)->serialize();
    }
    
    // Fallback implementation
    template<typename D = Derived>
    auto serialize() -> std::enable_if_t<!has_serialize_v<D>, std::string> {
        return "No serialization available";
    }
};
```

## Testing CRTP

### Unit Testing
```cpp
// Test CRTP functionality
TEST(CRTPTest, StaticPolymorphism) {
    Circle circle(5.0);
    EXPECT_DOUBLE_EQ(circle.area(), 3.14159 * 25.0);
    
    Rectangle rect(4.0, 6.0);
    EXPECT_DOUBLE_EQ(rect.area(), 24.0);
}

// Test mixin functionality
TEST(CRTPTest, ComparableOperators) {
    Number num1(10);
    Number num2(20);
    
    EXPECT_TRUE(num1 != num2);
    EXPECT_TRUE(num1 < num2);
    EXPECT_TRUE(num2 > num1);
}

// Test instance counting
TEST(CRTPTest, InstanceCounting) {
    EXPECT_EQ(Widget::getInstanceCount(), 0);
    
    {
        Widget w1("test1");
        Widget w2("test2");
        EXPECT_EQ(Widget::getInstanceCount(), 2);
    }
    
    EXPECT_EQ(Widget::getInstanceCount(), 0);
}
```

### Compile-Time Testing
```cpp
// Test interface compliance at compile time
static_assert(std::is_base_of_v<Shape<Circle>, Circle>);
static_assert(std::is_same_v<decltype(std::declval<Circle>().area()), double>);

// Test expression templates
static_assert(std::is_same_v<
    decltype(std::declval<Vector>() + std::declval<Vector>()),
    VectorAdd<Vector, Vector>
>);
```

## Real-World Examples

### Eigen Library (Linear Algebra)
```cpp
// Simplified Eigen-style CRTP
template<typename Derived>
class MatrixBase {
public:
    const Derived& derived() const {
        return static_cast<const Derived&>(*this);
    }
    
    template<typename OtherDerived>
    auto operator+(const MatrixBase<OtherDerived>& other) const {
        return MatrixSum<Derived, OtherDerived>(derived(), other.derived());
    }
};

class Matrix : public MatrixBase<Matrix> {
    // Matrix implementation
};

class Vector : public MatrixBase<Vector> {
    // Vector implementation
};
```

### Iterator Facades
```cpp
template<typename Derived, typename Value>
class IteratorFacade {
public:
    Value& operator*() {
        return static_cast<Derived&>(*this).dereference();
    }
    
    Derived& operator++() {
        static_cast<Derived&>(*this).increment();
        return static_cast<Derived&>(*this);
    }
    
    bool operator==(const Derived& other) const {
        return static_cast<const Derived&>(*this).equal(other);
    }
};

class MyIterator : public IteratorFacade<MyIterator, int> {
    // Only need to implement: dereference(), increment(), equal()
};
```

## Interview Questions

**Q: What is CRTP and how does it work?**
A: CRTP is a pattern where a class inherits from a template instantiated with itself as the template parameter. It enables static polymorphism by using static_cast to call derived class methods.

**Q: CRTP vs Virtual Functions - when to use which?**
A: Use CRTP for performance-critical code with compile-time polymorphism. Use virtual functions for runtime polymorphism where different types need to be stored together.

**Q: What are the main benefits of CRTP?**
A: Zero runtime overhead, better optimization opportunities, compile-time interface checking, and code reuse without virtual function costs.

**Q: What are common CRTP pitfalls?**
A: Object slicing, incorrect template parameters, premature static casting during construction, and incomplete type issues.

**Q: How does CRTP enable the mixin pattern?**
A: CRTP allows a base class template to provide functionality that works with the derived class interface, enabling composition of behaviors.

## Best Practices

1. **Always use static_assert for type checking**
2. **Be careful with constructor calls to derived methods**
3. **Document the required interface for derived classes**
4. **Use concepts (C++20) or SFINAE for interface validation**
5. **Consider the compilation cost vs runtime benefits**
6. **Make base classes non-copyable when appropriate**
7. **Provide clear error messages for misuse**

## Conclusion
CRTP is a powerful C++ idiom that provides:
- Zero-overhead polymorphism
- Compile-time interface checking
- Flexible mixin capabilities
- Expression template foundations

It's essential for high-performance C++ libraries and applications where runtime polymorphism overhead is unacceptable. Modern C++ features like concepts and constexpr make CRTP even more powerful and safer to use.
