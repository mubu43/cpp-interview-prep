# Inheritance in C++ - Complete Study Guide

## Table of Contents
1. [Introduction](#introduction)
2. [Basic Inheritance](#basic-inheritance)
3. [Types of Inheritance](#types-of-inheritance)
4. [Virtual Functions](#virtual-functions)
5. [Abstract Classes](#abstract-classes)
6. [Polymorphism](#polymorphism)
7. [Constructor and Destructor Order](#constructor-and-destructor-order)
8. [Multiple Inheritance](#multiple-inheritance)
9. [Common Pitfalls](#common-pitfalls)
10. [Best Practices](#best-practices)
11. [Interview Questions](#interview-questions)

## Introduction

Inheritance is a fundamental concept in object-oriented programming that allows classes to inherit properties and behaviors from other classes. It enables code reuse, establishes IS-A relationships, and forms the foundation for polymorphism.

### Key Concepts
- **Base Class (Parent)**: The class being inherited from
- **Derived Class (Child)**: The class that inherits from the base class
- **IS-A Relationship**: Derived class "is a" type of base class
- **Code Reuse**: Inherit existing functionality without rewriting
- **Extensibility**: Add new features while maintaining existing ones

## Basic Inheritance

### Syntax
```cpp
class Base {
protected:
    int protected_member_;
public:
    Base(int value) : protected_member_(value) {}
    virtual void virtualFunction() {}
    void nonVirtualFunction() {}
};

class Derived : public Base {
private:
    int derived_member_;
public:
    Derived(int base_val, int derived_val) 
        : Base(base_val), derived_member_(derived_val) {}
    
    void virtualFunction() override {}  // C++11 override keyword
    void newFunction() {}
};
```

### Access Control in Inheritance

#### Public Inheritance (IS-A Relationship)
```cpp
class Animal {
protected:
    std::string name_;
public:
    Animal(const std::string& name) : name_(name) {}
    virtual void makeSound() = 0;
};

class Dog : public Animal {  // Dog IS-A Animal
public:
    Dog(const std::string& name) : Animal(name) {}
    void makeSound() override { std::cout << "Woof!\n"; }
};

// Usage
Dog dog("Buddy");
Animal* animal = &dog;  // Valid: Dog IS-A Animal
animal->makeSound();    // Calls Dog::makeSound()
```

#### Private Inheritance (IS-IMPLEMENTED-IN-TERMS-OF)
```cpp
class Timer {
protected:
    int seconds_;
public:
    Timer() : seconds_(0) {}
    void tick() { ++seconds_; }
    int getTime() const { return seconds_; }
};

class StopWatch : private Timer {  // Not IS-A relationship
public:
    void start() { /* implementation */ }
    void stop() { /* implementation */ }
    
    // Selectively expose Timer functionality
    using Timer::getTime;
    using Timer::tick;
    
    // Timer's other methods remain private
};

// Timer* ptr = &stopwatch;  // ERROR: Not IS-A relationship
```

#### Protected Inheritance (Rare)
```cpp
class Vehicle : protected Engine {
    // Engine's public members become protected in Vehicle
    // Typically used for controlled inheritance hierarchies
};
```

### Member Access Rules

| Base Access | Public Inheritance | Protected Inheritance | Private Inheritance |
|-------------|-------------------|----------------------|-------------------|
| public      | public            | protected            | private           |
| protected   | protected         | protected            | private           |
| private     | not accessible    | not accessible       | not accessible    |

## Virtual Functions

### Basic Virtual Functions
```cpp
class Base {
public:
    virtual void virtualFunc() {
        std::cout << "Base version\n";
    }
    
    void nonVirtualFunc() {
        std::cout << "Base non-virtual\n";
    }
};

class Derived : public Base {
public:
    void virtualFunc() override {  // Overrides base version
        std::cout << "Derived version\n";
    }
    
    void nonVirtualFunc() {  // Hides base version
        std::cout << "Derived non-virtual\n";
    }
};

void demonstrate() {
    Derived d;
    Base* ptr = &d;
    
    ptr->virtualFunc();     // Calls Derived::virtualFunc()
    ptr->nonVirtualFunc();  // Calls Base::nonVirtualFunc()
}
```

### Virtual Function Mechanism
- **Virtual Table (vtable)**: Array of function pointers
- **Virtual Pointer (vptr)**: Points to object's vtable
- **Runtime Polymorphism**: Function call resolved at runtime

```cpp
// Conceptual vtable layout
class Base {
    virtual void func1() {}
    virtual void func2() {}
    // vptr points to Base's vtable
};

class Derived : public Base {
    void func1() override {}  // Overrides Base::func1
    virtual void func3() {}   // New virtual function
    // vptr points to Derived's vtable
};
```

### Virtual Destructors
```cpp
class Base {
public:
    virtual ~Base() {  // Virtual destructor is crucial
        std::cout << "Base destructor\n";
    }
};

class Derived : public Base {
private:
    int* data_;
public:
    Derived() : data_(new int[100]) {}
    
    ~Derived() override {
        delete[] data_;
        std::cout << "Derived destructor\n";
    }
};

void proper_cleanup() {
    std::unique_ptr<Base> ptr = std::make_unique<Derived>();
    // When ptr is destroyed:
    // 1. Derived destructor called (because virtual)
    // 2. Base destructor called
    // Proper cleanup guaranteed
}
```

### Final and Override Keywords (C++11)
```cpp
class Base {
public:
    virtual void func1() {}
    virtual void func2() {}
};

class Derived : public Base {
public:
    void func1() override {}        // Explicit override
    void func2() final override {}  // Cannot be overridden further
    
    // void func3() override {}     // ERROR: No base function to override
};

class FinalDerived final : public Derived {  // Cannot be inherited
    // void func2() override {}  // ERROR: func2 is final
};

// class CannotInherit : public FinalDerived {};  // ERROR: final class
```

## Abstract Classes

### Pure Virtual Functions
```cpp
class Shape {  // Abstract class
public:
    virtual ~Shape() = default;
    
    // Pure virtual functions
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
    virtual void draw() const = 0;
    
    // Non-pure virtual function (has default implementation)
    virtual void displayInfo() const {
        std::cout << "Area: " << area() << ", Perimeter: " << perimeter() << "\n";
    }
    
    // Non-virtual function
    void commonOperation() const {
        std::cout << "Common operation for all shapes\n";
    }
};

class Rectangle : public Shape {
private:
    double width_, height_;
    
public:
    Rectangle(double w, double h) : width_(w), height_(h) {}
    
    // Must implement all pure virtual functions
    double area() const override {
        return width_ * height_;
    }
    
    double perimeter() const override {
        return 2 * (width_ + height_);
    }
    
    void draw() const override {
        std::cout << "Drawing rectangle " << width_ << "x" << height_ << "\n";
    }
};
```

### Interface Classes
```cpp
// Pure interface (all methods are pure virtual)
class Drawable {
public:
    virtual ~Drawable() = default;
    virtual void draw() const = 0;
    virtual void erase() const = 0;
};

class Movable {
public:
    virtual ~Movable() = default;
    virtual void move(int x, int y) = 0;
};

// Multiple interface inheritance
class GameObject : public Drawable, public Movable {
private:
    int x_, y_;
    
public:
    GameObject(int x, int y) : x_(x), y_(y) {}
    
    void draw() const override {
        std::cout << "Drawing at (" << x_ << ", " << y_ << ")\n";
    }
    
    void erase() const override {
        std::cout << "Erasing at (" << x_ << ", " << y_ << ")\n";
    }
    
    void move(int x, int y) override {
        x_ = x;
        y_ = y;
    }
};
```

## Polymorphism

### Runtime Polymorphism
```cpp
void processShapes(const std::vector<std::unique_ptr<Shape>>& shapes) {
    for (const auto& shape : shapes) {
        shape->draw();        // Calls appropriate derived class method
        shape->displayInfo(); // Runtime polymorphism
        std::cout << "---\n";
    }
}

void demonstrate_polymorphism() {
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Rectangle>(5, 3));
    shapes.push_back(std::make_unique<Circle>(2.5));
    shapes.push_back(std::make_unique<Triangle>(4, 6, 8));
    
    processShapes(shapes);  // Each shape behaves differently
}
```

### Polymorphic Behavior
```cpp
class Animal {
public:
    virtual void makeSound() const = 0;
    virtual void move() const = 0;
    virtual ~Animal() = default;
};

class Dog : public Animal {
public:
    void makeSound() const override { std::cout << "Woof!\n"; }
    void move() const override { std::cout << "Runs on four legs\n"; }
};

class Bird : public Animal {
public:
    void makeSound() const override { std::cout << "Tweet!\n"; }
    void move() const override { std::cout << "Flies\n"; }
};

void animal_behavior(const Animal& animal) {
    animal.makeSound();  // Polymorphic call
    animal.move();       // Polymorphic call
}
```

## Constructor and Destructor Order

### Construction Order
```cpp
class Grandparent {
public:
    Grandparent() { std::cout << "Grandparent constructor\n"; }
    virtual ~Grandparent() { std::cout << "Grandparent destructor\n"; }
};

class Parent : public Grandparent {
public:
    Parent() { std::cout << "Parent constructor\n"; }
    ~Parent() override { std::cout << "Parent destructor\n"; }
};

class Child : public Parent {
public:
    Child() { std::cout << "Child constructor\n"; }
    ~Child() override { std::cout << "Child destructor\n"; }
};

void demonstrate_order() {
    Child c;
    // Construction order: Grandparent → Parent → Child
    // Destruction order: Child → Parent → Grandparent
}
```

### Constructor Chaining
```cpp
class Base {
protected:
    int value_;
    
public:
    Base(int value) : value_(value) {
        std::cout << "Base constructor: " << value_ << "\n";
    }
};

class Derived : public Base {
private:
    std::string name_;
    
public:
    // Must call base constructor explicitly
    Derived(int value, const std::string& name) 
        : Base(value), name_(name) {
        std::cout << "Derived constructor: " << name_ << "\n";
    }
    
    // If base has no default constructor, this would be an error:
    // Derived() {}  // ERROR: No matching Base constructor
};
```

## Multiple Inheritance

### Basic Multiple Inheritance
```cpp
class Flyable {
public:
    virtual void fly() { std::cout << "Flying\n"; }
    virtual ~Flyable() = default;
};

class Swimmable {
public:
    virtual void swim() { std::cout << "Swimming\n"; }
    virtual ~Swimmable() = default;
};

class Duck : public Animal, public Flyable, public Swimmable {
public:
    Duck(const std::string& name) : Animal(name) {}
    
    void makeSound() const override { std::cout << "Quack!\n"; }
    void move() const override { 
        std::cout << "Walks, flies, and swims\n"; 
    }
    
    void fly() override { std::cout << "Duck flies\n"; }
    void swim() override { std::cout << "Duck swims\n"; }
};
```

### Diamond Problem
```cpp
class A {
public:
    int value;
    A(int v) : value(v) {}
};

class B : public A {
public:
    B(int v) : A(v) {}
};

class C : public A {
public:
    C(int v) : A(v) {}
};

// Problem: D inherits two copies of A
class D : public B, public C {
public:
    D(int v) : B(v), C(v) {}
    
    void access_value() {
        // std::cout << value;     // ERROR: Ambiguous
        std::cout << B::value;     // OK: Specify which A
        std::cout << C::value;     // OK: Specify which A
    }
};
```

### Virtual Inheritance (Solution to Diamond Problem)
```cpp
class A {
public:
    int value;
    A(int v) : value(v) { std::cout << "A constructor\n"; }
};

class B : virtual public A {  // Virtual inheritance
public:
    B(int v) : A(v) { std::cout << "B constructor\n"; }
};

class C : virtual public A {  // Virtual inheritance
public:
    C(int v) : A(v) { std::cout << "C constructor\n"; }
};

class D : public B, public C {
public:
    // Most derived class calls virtual base constructor
    D(int v) : A(v), B(v), C(v) { 
        std::cout << "D constructor\n"; 
    }
    
    void access_value() {
        std::cout << value;  // OK: Only one copy of A
    }
};
```

## Common Pitfalls

### 1. Object Slicing
```cpp
class Base {
public:
    virtual void func() { std::cout << "Base\n"; }
    int base_data;
};

class Derived : public Base {
public:
    void func() override { std::cout << "Derived\n"; }
    int derived_data;
};

void demonstrate_slicing() {
    Derived d;
    d.derived_data = 42;
    
    Base b = d;  // SLICING: Only Base part copied
    b.func();    // Calls Base::func(), not Derived::func()
    
    // derived_data is lost!
    
    // Correct: Use references or pointers
    Base& ref = d;
    ref.func();  // Calls Derived::func()
}
```

### 2. Calling Virtual Functions in Constructors
```cpp
class Base {
public:
    Base() {
        init();  // Dangerous: calls Base::init(), not derived version
    }
    
    virtual void init() {
        std::cout << "Base init\n";
    }
    
    virtual ~Base() = default;
};

class Derived : public Base {
public:
    Derived() : Base() {
        // Base constructor already finished
        init();  // Now this calls Derived::init()
    }
    
    void init() override {
        std::cout << "Derived init\n";
    }
};
```

### 3. Assignment Operator Issues
```cpp
class Base {
public:
    Base& operator=(const Base& other) {
        if (this != &other) {
            // Copy base data
        }
        return *this;
    }
};

class Derived : public Base {
private:
    std::string derived_data_;
    
public:
    Derived& operator=(const Derived& other) {
        if (this != &other) {
            Base::operator=(other);  // Call base assignment
            derived_data_ = other.derived_data_;
        }
        return *this;
    }
    
    // Also need virtual assignment for polymorphic assignment
    virtual Derived& operator=(const Base& other) {
        const Derived* derived_other = dynamic_cast<const Derived*>(&other);
        if (derived_other) {
            return operator=(*derived_other);
        }
        return *this;
    }
};
```

## Best Practices

### 1. Design Guidelines

#### Prefer Composition Over Inheritance
```cpp
// Instead of inheritance for code reuse
class Bird : public Flyable {  // IS-A relationship unclear
    // ...
};

// Use composition
class Bird {
private:
    std::unique_ptr<FlightCapability> flight_;  // HAS-A relationship
public:
    void fly() {
        if (flight_) {
            flight_->fly();
        }
    }
};
```

#### Use Inheritance for IS-A Relationships
```cpp
// Good: Clear IS-A relationship
class Rectangle : public Shape {
    // Rectangle IS-A Shape
};

class Dog : public Animal {
    // Dog IS-A Animal
};

// Bad: No clear IS-A relationship
class Button : public Rectangle {
    // Button is not really a Rectangle
};
```

### 2. Virtual Function Guidelines

#### Always Make Base Destructors Virtual
```cpp
class Base {
public:
    virtual ~Base() = default;  // Virtual destructor
    virtual void func() = 0;
};
```

#### Use Override and Final Keywords
```cpp
class Derived : public Base {
public:
    void func() override final {  // Explicit override, cannot be overridden
        // Implementation
    }
};
```

#### Avoid Virtual Functions in Constructors/Destructors
```cpp
class Base {
public:
    Base() {
        // Don't call virtual functions here
        initialize();  // Call non-virtual initialization
    }
    
    virtual ~Base() {
        // Don't call virtual functions here
        cleanup();     // Call non-virtual cleanup
    }
    
private:
    void initialize() { /* non-virtual initialization */ }
    void cleanup() { /* non-virtual cleanup */ }
};
```

### 3. Abstract Class Design

#### Define Clear Interfaces
```cpp
// Good: Clear, minimal interface
class Drawable {
public:
    virtual ~Drawable() = default;
    virtual void draw(Canvas& canvas) const = 0;
    virtual BoundingBox getBounds() const = 0;
};

// Bad: Too many responsibilities
class GraphicsObject {
public:
    virtual void draw() = 0;
    virtual void serialize() = 0;
    virtual void handleInput() = 0;
    virtual void updateAnimation() = 0;
    // Too many different responsibilities
};
```

#### Provide Template Method Pattern
```cpp
class Algorithm {
public:
    void execute() {  // Template method
        initialize();
        processData();
        finalize();
    }
    
protected:
    virtual void initialize() {}           // Optional override
    virtual void processData() = 0;       // Must override
    virtual void finalize() {}            // Optional override
};
```

## Interview Questions

### Basic Level

1. **What is inheritance in C++?**
   - Mechanism for creating new classes based on existing classes
   - Enables code reuse and establishes IS-A relationships
   - Allows derived classes to inherit properties and behaviors from base classes

2. **What are the different types of inheritance?**
   - Public inheritance: IS-A relationship, base public members remain public
   - Private inheritance: IS-IMPLEMENTED-IN-TERMS-OF, base public members become private
   - Protected inheritance: Rarely used, base public members become protected

3. **What is a virtual function?**
   - Function that can be overridden in derived classes
   - Enables runtime polymorphism through dynamic dispatch
   - Resolved at runtime based on object's actual type

### Intermediate Level

4. **Why do we need virtual destructors?**
   - Ensure proper cleanup when deleting through base class pointer
   - Without virtual destructor, only base destructor called (resource leak)
   - Essential for proper polymorphic destruction

5. **What is the difference between function overriding and function hiding?**
   - Overriding: Virtual function replaced in derived class (polymorphic)
   - Hiding: Non-virtual function shadows base class function (static binding)
   - Override affects polymorphic calls; hiding affects static calls

6. **What is an abstract class?**
   - Class with at least one pure virtual function
   - Cannot be instantiated directly
   - Used to define interfaces and common base functionality

### Advanced Level

7. **Explain the diamond problem in multiple inheritance.**
   - Occurs when class inherits from two classes that share common base
   - Results in ambiguous member access and duplicate base class copies
   - Solved using virtual inheritance

8. **What is object slicing?**
   - When derived object copied to base class object
   - Derived class parts are "sliced off"
   - Results in loss of derived class data and polymorphic behavior

9. **How does virtual function mechanism work internally?**
   - Each class with virtual functions has virtual table (vtable)
   - Objects contain virtual pointer (vptr) to class's vtable
   - Virtual function calls resolved through vtable lookup at runtime

### Expert Level

10. **When should you use private inheritance?**
    - When implementing IS-IMPLEMENTED-IN-TERMS-OF relationship
    - For controlled access to base class functionality
    - Alternative to composition for performance-critical code

11. **How do you handle assignment in inheritance hierarchies?**
    - Implement assignment operators at each level
    - Call base class assignment operator from derived
    - Consider virtual assignment for polymorphic assignment
    - Be aware of object slicing issues

12. **What are the performance implications of virtual functions?**
    - Small overhead for virtual function calls (vtable lookup)
    - Prevents some compiler optimizations (inlining)
    - Memory overhead for vtable and vptr
    - Usually negligible compared to benefits

## Summary

Inheritance is a powerful C++ feature that enables:

**Core Benefits:**
- Code reuse through IS-A relationships
- Polymorphic behavior through virtual functions
- Extensible designs through abstract classes
- Interface definition through pure virtual functions

**Key Concepts:**
- Public inheritance for IS-A relationships
- Virtual functions for runtime polymorphism
- Abstract classes for interface definition
- Virtual destructors for proper cleanup

**Best Practices:**
- Use inheritance for clear IS-A relationships
- Prefer composition over inheritance for code reuse
- Always make base destructors virtual
- Use override and final keywords for clarity
- Design clear, minimal interfaces

Understanding inheritance is crucial for effective object-oriented design in C++.
