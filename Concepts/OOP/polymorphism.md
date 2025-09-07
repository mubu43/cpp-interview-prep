# Polymorphism in C++ - Complete Study Guide

## Table of Contents
1. [Introduction](#introduction)
2. [Types of Polymorphism](#types-of-polymorphism)
3. [Runtime Polymorphism](#runtime-polymorphism)
4. [Compile-Time Polymorphism](#compile-time-polymorphism)
5. [Virtual Functions Deep Dive](#virtual-functions-deep-dive)
6. [Dynamic Casting](#dynamic-casting)
7. [Abstract Classes and Interfaces](#abstract-classes-and-interfaces)
8. [Performance Considerations](#performance-considerations)
9. [Design Patterns](#design-patterns)
10. [Best Practices](#best-practices)
11. [Interview Questions](#interview-questions)

## Introduction

Polymorphism is one of the core principles of object-oriented programming, allowing objects of different types to be treated as objects of a common base type. The word "polymorphism" comes from Greek, meaning "many forms." In C++, polymorphism enables a single interface to represent different underlying forms (data types).

### Key Benefits
- **Code Flexibility**: Write code that works with multiple types
- **Extensibility**: Add new types without modifying existing code
- **Maintainability**: Centralized interface management
- **Abstraction**: Hide implementation details behind common interfaces

## Types of Polymorphism

### 1. Runtime Polymorphism (Dynamic Polymorphism)
- Resolved at runtime through virtual functions
- Uses inheritance and virtual function tables
- Enables late binding

### 2. Compile-Time Polymorphism (Static Polymorphism)
- Resolved at compile time
- Includes function overloading, operator overloading, and templates
- Uses early binding

## Runtime Polymorphism

### Basic Virtual Functions
```cpp
class Animal {
public:
    virtual ~Animal() = default;
    
    virtual void makeSound() const = 0;  // Pure virtual
    virtual void move() const {          // Virtual with default
        std::cout << "Animal moves\n";
    }
    
    void breathe() const {               // Non-virtual
        std::cout << "Animal breathes\n";
    }
};

class Dog : public Animal {
public:
    void makeSound() const override {
        std::cout << "Woof!\n";
    }
    
    void move() const override {
        std::cout << "Dog runs\n";
    }
};

class Cat : public Animal {
public:
    void makeSound() const override {
        std::cout << "Meow!\n";
    }
    
    // Uses default move() implementation
};
```

### Polymorphic Usage
```cpp
void processAnimal(const Animal& animal) {
    animal.makeSound();  // Calls appropriate derived class method
    animal.move();       // Runtime polymorphism
    animal.breathe();    // Always calls Animal::breathe()
}

void demonstrate() {
    Dog dog;
    Cat cat;
    
    processAnimal(dog);  // Calls Dog methods
    processAnimal(cat);  // Calls Cat methods
    
    // Container of polymorphic objects
    std::vector<std::unique_ptr<Animal>> animals;
    animals.push_back(std::make_unique<Dog>());
    animals.push_back(std::make_unique<Cat>());
    
    for (const auto& animal : animals) {
        animal->makeSound();  // Polymorphic call
    }
}
```

### Virtual Function Table (vtable)
```cpp
// Conceptual vtable structure
class Base {
public:
    virtual void func1() { std::cout << "Base::func1\n"; }
    virtual void func2() { std::cout << "Base::func2\n"; }
    // vtable: [&Base::func1, &Base::func2]
};

class Derived : public Base {
public:
    void func1() override { std::cout << "Derived::func1\n"; }
    virtual void func3() { std::cout << "Derived::func3\n"; }
    // vtable: [&Derived::func1, &Base::func2, &Derived::func3]
};

// Each object has vptr pointing to its class's vtable
Base* ptr = new Derived();
ptr->func1();  // 1. Access vptr, 2. Lookup vtable[0], 3. Call function
```

### Pure Virtual Functions and Abstract Classes
```cpp
class Shape {  // Abstract class
public:
    virtual ~Shape() = default;
    
    // Pure virtual functions - must be implemented
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
    virtual void draw() const = 0;
    
    // Regular virtual function - can be overridden
    virtual void displayInfo() const {
        std::cout << "Area: " << area() << ", Perimeter: " << perimeter() << "\n";
    }
    
    // Non-virtual function - shared implementation
    void validate() const {
        if (area() < 0) throw std::invalid_argument("Invalid area");
    }
};

class Circle : public Shape {
private:
    double radius_;
    
public:
    Circle(double r) : radius_(r) {}
    
    double area() const override {
        return 3.14159 * radius_ * radius_;
    }
    
    double perimeter() const override {
        return 2 * 3.14159 * radius_;
    }
    
    void draw() const override {
        std::cout << "Drawing circle with radius " << radius_ << "\n";
    }
};

// Cannot instantiate abstract class
// Shape shape;  // ERROR
Circle circle(5.0);  // OK
```

## Compile-Time Polymorphism

### Function Overloading
```cpp
class Calculator {
public:
    int add(int a, int b) {
        return a + b;
    }
    
    double add(double a, double b) {
        return a + b;
    }
    
    std::string add(const std::string& a, const std::string& b) {
        return a + b;
    }
};

Calculator calc;
auto result1 = calc.add(5, 3);        // Calls int version
auto result2 = calc.add(5.5, 3.2);    // Calls double version
auto result3 = calc.add("Hello", "World"); // Calls string version
```

### Template Polymorphism
```cpp
template<typename T>
void processShape(const T& shape) {
    shape.draw();
    std::cout << "Area: " << shape.area() << "\n";
}

// Works with any type that has draw() and area() methods
Circle circle(3.0);
Rectangle rect(4.0, 5.0);

processShape(circle);  // T = Circle
processShape(rect);    // T = Rectangle
```

### CRTP (Curiously Recurring Template Pattern)
```cpp
template<typename Derived>
class Shape {
public:
    void draw() const {
        static_cast<const Derived*>(this)->drawImpl();
    }
    
    double area() const {
        return static_cast<const Derived*>(this)->areaImpl();
    }
};

class Circle : public Shape<Circle> {
public:
    void drawImpl() const {
        std::cout << "Drawing circle\n";
    }
    
    double areaImpl() const {
        return 3.14159 * radius_ * radius_;
    }
    
private:
    double radius_ = 1.0;
};

// No virtual function overhead, resolved at compile time
Circle circle;
circle.draw();  // Calls Circle::drawImpl() directly
```

## Virtual Functions Deep Dive

### Virtual Destructors
```cpp
class Base {
public:
    virtual ~Base() {  // Essential for proper cleanup
        std::cout << "Base destructor\n";
    }
    
    virtual void process() = 0;
};

class Derived : public Base {
private:
    int* data_;
    
public:
    Derived() : data_(new int[1000]) {
        std::cout << "Derived constructor\n";
    }
    
    ~Derived() override {
        delete[] data_;
        std::cout << "Derived destructor\n";
    }
    
    void process() override {
        std::cout << "Processing in derived\n";
    }
};

void demonstrate_virtual_destructor() {
    std::unique_ptr<Base> ptr = std::make_unique<Derived>();
    // When ptr is destroyed:
    // 1. Derived::~Derived() called (virtual destructor)
    // 2. Base::~Base() called
    // Proper cleanup guaranteed
}
```

### Override and Final Keywords (C++11)
```cpp
class Base {
public:
    virtual void func1() {}
    virtual void func2() {}
    virtual void func3() {}
};

class Derived : public Base {
public:
    void func1() override {}        // Explicit override
    void func2() final override {}  // Final override
    // void func4() override {}     // ERROR: No base function
};

class FurtherDerived : public Derived {
public:
    void func1() override {}  // OK
    // void func2() override {}  // ERROR: func2 is final
};
```

### Virtual Function Behavior in Constructors/Destructors
```cpp
class Base {
public:
    Base() {
        init();  // Calls Base::init(), not derived version!
    }
    
    virtual ~Base() {
        cleanup();  // Calls Base::cleanup(), not derived version!
    }
    
    virtual void init() {
        std::cout << "Base init\n";
    }
    
    virtual void cleanup() {
        std::cout << "Base cleanup\n";
    }
};

class Derived : public Base {
public:
    Derived() : Base() {
        init();  // Now calls Derived::init()
    }
    
    ~Derived() override {
        cleanup();  // Calls Derived::cleanup()
    }
    
    void init() override {
        std::cout << "Derived init\n";
    }
    
    void cleanup() override {
        std::cout << "Derived cleanup\n";
    }
};

// Output when creating Derived:
// Base init      (from Base constructor)
// Derived init   (from Derived constructor)
// Derived cleanup (from Derived destructor)
// Base cleanup   (from Base destructor)
```

## Dynamic Casting

### Safe Downcasting
```cpp
class Animal {
public:
    virtual ~Animal() = default;
    virtual void makeSound() const = 0;
};

class Dog : public Animal {
public:
    void makeSound() const override { std::cout << "Woof!\n"; }
    void wagTail() const { std::cout << "Wagging tail\n"; }
};

class Cat : public Animal {
public:
    void makeSound() const override { std::cout << "Meow!\n"; }
    void purr() const { std::cout << "Purring\n"; }
};

void processAnimal(Animal* animal) {
    animal->makeSound();  // Polymorphic call
    
    // Safe downcasting
    if (Dog* dog = dynamic_cast<Dog*>(animal)) {
        dog->wagTail();  // Dog-specific behavior
    }
    else if (Cat* cat = dynamic_cast<Cat*>(animal)) {
        cat->purr();     // Cat-specific behavior
    }
}
```

### Dynamic Cast with References
```cpp
void processAnimalRef(Animal& animal) {
    try {
        Dog& dog = dynamic_cast<Dog&>(animal);
        dog.wagTail();
    }
    catch (const std::bad_cast& e) {
        std::cout << "Not a dog: " << e.what() << "\n";
    }
}
```

### Type Information (RTTI)
```cpp
void analyzeType(const Animal& animal) {
    std::cout << "Type: " << typeid(animal).name() << "\n";
    
    if (typeid(animal) == typeid(Dog)) {
        std::cout << "This is a Dog\n";
    }
    else if (typeid(animal) == typeid(Cat)) {
        std::cout << "This is a Cat\n";
    }
}
```

## Abstract Classes and Interfaces

### Interface Design
```cpp
// Pure interface - all methods are pure virtual
class Drawable {
public:
    virtual ~Drawable() = default;
    virtual void draw() const = 0;
    virtual void erase() const = 0;
    virtual void move(int x, int y) = 0;
};

class Resizable {
public:
    virtual ~Resizable() = default;
    virtual void resize(double factor) = 0;
    virtual void setSize(int width, int height) = 0;
};

// Multiple interface inheritance
class GraphicsObject : public Drawable, public Resizable {
protected:
    int x_, y_, width_, height_;
    
public:
    GraphicsObject(int x, int y, int w, int h) 
        : x_(x), y_(y), width_(w), height_(h) {}
    
    void move(int x, int y) override {
        x_ = x;
        y_ = y;
    }
    
    void setSize(int width, int height) override {
        width_ = width;
        height_ = height;
    }
    
    void resize(double factor) override {
        width_ = static_cast<int>(width_ * factor);
        height_ = static_cast<int>(height_ * factor);
    }
    
    // draw() and erase() remain pure virtual
};
```

### Template Method Pattern
```cpp
class DataProcessor {
public:
    void process() {  // Template method
        loadData();
        validateData();
        processData();
        saveResults();
    }
    
protected:
    virtual void loadData() = 0;       // Must implement
    virtual void processData() = 0;    // Must implement
    
    virtual void validateData() {      // Optional override
        std::cout << "Default validation\n";
    }
    
    virtual void saveResults() {       // Optional override
        std::cout << "Default save\n";
    }
};

class CSVProcessor : public DataProcessor {
protected:
    void loadData() override {
        std::cout << "Loading CSV data\n";
    }
    
    void processData() override {
        std::cout << "Processing CSV data\n";
    }
    
    void validateData() override {
        std::cout << "CSV-specific validation\n";
    }
};
```

## Performance Considerations

### Virtual Function Overhead
```cpp
// Performance comparison
class Direct {
public:
    void method() { /* implementation */ }
};

class Virtual {
public:
    virtual void method() { /* implementation */ }
};

void benchmark() {
    const int iterations = 10000000;
    
    Direct direct;
    Virtual virtual_obj;
    Virtual* virtual_ptr = &virtual_obj;
    
    // Direct call - fastest
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        direct.method();
    }
    auto direct_time = std::chrono::high_resolution_clock::now() - start;
    
    // Virtual call - slight overhead
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        virtual_ptr->method();
    }
    auto virtual_time = std::chrono::high_resolution_clock::now() - start;
    
    // Overhead is typically 10-20% for simple functions
}
```

### When to Use Virtual Functions
- **Use virtual functions when**:
  - You need runtime polymorphism
  - Objects will be used through base class pointers/references
  - Different behavior needed for different derived classes

- **Avoid virtual functions when**:
  - Performance is critical and polymorphism not needed
  - Class hierarchy is shallow and simple
  - Compile-time polymorphism (templates) can be used instead

### Memory Layout
```cpp
class NonVirtual {
    int data1_;
    int data2_;
    // Size: 8 bytes (2 ints)
};

class WithVirtual {
    int data1_;
    int data2_;
    virtual void func() {}
    // Size: 16 bytes (2 ints + vptr, assuming 64-bit)
};
```

## Design Patterns

### Factory Pattern
```cpp
class Shape {
public:
    virtual ~Shape() = default;
    virtual void draw() const = 0;
    virtual double area() const = 0;
};

class ShapeFactory {
public:
    enum class Type { CIRCLE, RECTANGLE, TRIANGLE };
    
    static std::unique_ptr<Shape> create(Type type) {
        switch (type) {
            case Type::CIRCLE:
                return std::make_unique<Circle>(1.0);
            case Type::RECTANGLE:
                return std::make_unique<Rectangle>(2.0, 3.0);
            case Type::TRIANGLE:
                return std::make_unique<Triangle>(3.0, 4.0, 5.0);
        }
        return nullptr;
    }
};

// Usage
auto shape = ShapeFactory::create(ShapeFactory::Type::CIRCLE);
shape->draw();  // Polymorphic call
```

### Strategy Pattern
```cpp
class SortStrategy {
public:
    virtual ~SortStrategy() = default;
    virtual void sort(std::vector<int>& data) = 0;
};

class BubbleSort : public SortStrategy {
public:
    void sort(std::vector<int>& data) override {
        // Bubble sort implementation
    }
};

class QuickSort : public SortStrategy {
public:
    void sort(std::vector<int>& data) override {
        // Quick sort implementation
    }
};

class Sorter {
private:
    std::unique_ptr<SortStrategy> strategy_;
    
public:
    void setStrategy(std::unique_ptr<SortStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    void sort(std::vector<int>& data) {
        if (strategy_) {
            strategy_->sort(data);
        }
    }
};
```

### Observer Pattern
```cpp
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const std::string& message) = 0;
};

class Subject {
private:
    std::vector<Observer*> observers_;
    
public:
    void addObserver(Observer* observer) {
        observers_.push_back(observer);
    }
    
    void removeObserver(Observer* observer) {
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end()
        );
    }
    
    void notify(const std::string& message) {
        for (auto observer : observers_) {
            observer->update(message);  // Polymorphic call
        }
    }
};
```

## Best Practices

### 1. Interface Design
```cpp
// Good: Minimal, focused interface
class Serializable {
public:
    virtual ~Serializable() = default;
    virtual std::string serialize() const = 0;
    virtual void deserialize(const std::string& data) = 0;
};

// Bad: Too many responsibilities
class Multipurpose {
public:
    virtual void draw() = 0;
    virtual void serialize() = 0;
    virtual void handleInput() = 0;
    virtual void update() = 0;
    virtual void render() = 0;
    // Too many unrelated responsibilities
};
```

### 2. Virtual Destructor Rules
```cpp
// Always virtual destructor in base classes
class Base {
public:
    virtual ~Base() = default;  // Virtual destructor
    virtual void func() = 0;
};

// Exception: Final classes don't need virtual destructors
class FinalClass final {
public:
    ~FinalClass() = default;  // Non-virtual is OK
};
```

### 3. Prefer Composition Over Inheritance
```cpp
// Instead of inheritance for code reuse
class Bird : public Flyable {  // Questionable IS-A relationship
    // ...
};

// Use composition
class Bird {
private:
    std::unique_ptr<FlightBehavior> flight_behavior_;
    
public:
    void fly() {
        if (flight_behavior_) {
            flight_behavior_->fly();
        }
    }
    
    void setFlightBehavior(std::unique_ptr<FlightBehavior> behavior) {
        flight_behavior_ = std::move(behavior);
    }
};
```

### 4. Use Smart Pointers
```cpp
// Good: Automatic memory management
std::vector<std::unique_ptr<Shape>> shapes;
shapes.push_back(std::make_unique<Circle>(5.0));

// Bad: Manual memory management
std::vector<Shape*> shapes;
shapes.push_back(new Circle(5.0));  // Memory leak risk
```

## Interview Questions

### Basic Level

1. **What is polymorphism?**
   - Ability of objects of different types to be treated as objects of common base type
   - "Many forms" - same interface, different implementations
   - Enables code to work with objects of different types uniformly

2. **What's the difference between compile-time and runtime polymorphism?**
   - Compile-time: Resolved during compilation (templates, overloading)
   - Runtime: Resolved during execution (virtual functions)
   - Compile-time is faster; runtime is more flexible

3. **What is a virtual function?**
   - Function that can be overridden in derived classes
   - Enables dynamic dispatch based on object's actual type
   - Declared with 'virtual' keyword in base class

### Intermediate Level

4. **How do virtual functions work internally?**
   - Each class with virtual functions has virtual table (vtable)
   - Objects contain virtual pointer (vptr) to class's vtable
   - Virtual function calls resolved through vtable lookup

5. **What is dynamic_cast and when to use it?**
   - Safe downcasting operator for polymorphic types
   - Returns nullptr (pointer) or throws bad_cast (reference) on failure
   - Use when you need to access derived class specific members

6. **What's the difference between pure virtual and virtual functions?**
   - Pure virtual: No implementation in base class (= 0), must be overridden
   - Virtual: Has implementation, can be overridden or used as-is
   - Pure virtual functions make class abstract

### Advanced Level

7. **Explain the performance implications of virtual functions.**
   - Small overhead: vtable lookup, indirect function call
   - Memory overhead: vptr in each object, vtable per class
   - Prevents some optimizations (inlining)
   - Usually negligible compared to benefits

8. **What happens when you call virtual functions in constructors/destructors?**
   - Virtual mechanism disabled during construction/destruction
   - Always calls base class version, not derived
   - Derived object not fully constructed/already destructed

9. **How would you implement polymorphism without virtual functions?**
   - Function pointers or std::function members
   - Type erasure with templates
   - Variant types with visitor pattern
   - Manual dispatch based on type tags

### Expert Level

10. **Compare virtual functions vs templates for polymorphism.**
    - Virtual: Runtime polymorphism, single compiled code, inheritance required
    - Templates: Compile-time polymorphism, code generation, duck typing
    - Virtual for heterogeneous containers; templates for performance

11. **What is object slicing and how to prevent it?**
    - When derived object copied to base class object
    - Derived parts "sliced off", polymorphism lost
    - Prevent: Use references/pointers, deleted copy operations

12. **How do you design exception-safe polymorphic hierarchies?**
    - Use RAII and smart pointers
    - Virtual destructors for proper cleanup
    - Strong exception safety in virtual functions
    - Consider PIMPL for ABI stability

## Summary

Polymorphism is a fundamental concept that enables flexible and extensible C++ designs:

**Key Benefits:**
- Code reusability and maintainability
- Flexible interfaces and extensible designs
- Clean separation of interface and implementation
- Support for design patterns and frameworks

**Implementation Mechanisms:**
- Virtual functions for runtime polymorphism
- Templates for compile-time polymorphism
- Function overloading for interface flexibility
- Abstract classes for interface definition

**Best Practices:**
- Use virtual destructors in base classes
- Prefer smart pointers for memory management
- Design minimal, focused interfaces
- Consider performance implications
- Use appropriate type of polymorphism for the use case

Understanding polymorphism is essential for designing robust, flexible, and maintainable C++ applications.
