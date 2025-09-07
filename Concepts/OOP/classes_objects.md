# Classes and Objects in C++ - Complete Study Guide

## Table of Contents
1. [Introduction](#introduction)
2. [Class Definition](#class-definition)
3. [Encapsulation](#encapsulation)
4. [Constructors and Destructors](#constructors-and-destructors)
5. [Copy and Move Semantics](#copy-and-move-semantics)
6. [Operator Overloading](#operator-overloading)
7. [Static Members](#static-members)
8. [Const Correctness](#const-correctness)
9. [Friend Functions](#friend-functions)
10. [Composition](#composition)
11. [Best Practices](#best-practices)
12. [Interview Questions](#interview-questions)

## Introduction

Classes and objects are the foundation of object-oriented programming in C++. A class defines a blueprint for creating objects that encapsulate data (member variables) and behavior (member functions). This approach promotes code organization, reusability, and maintainability.

### Key Concepts
- **Class**: A user-defined type that groups data and functions
- **Object**: An instance of a class
- **Encapsulation**: Bundling data and methods together
- **Data Hiding**: Controlling access to class members
- **Abstraction**: Hiding implementation details

## Class Definition

### Basic Syntax
```cpp
class ClassName {
private:
    // Private members (data hiding)
    int private_data_;
    
protected:
    // Protected members (inheritance)
    int protected_data_;
    
public:
    // Public interface
    ClassName();  // Constructor
    ~ClassName(); // Destructor
    
    void public_method();
    int get_data() const;
};
```

### Access Specifiers

#### Private
- Accessible only within the class
- Default access level for class members
- Implements data hiding

#### Protected
- Accessible within the class and derived classes
- Used for inheritance hierarchies

#### Public
- Accessible from anywhere
- Forms the class interface

### Member Types

#### Data Members
```cpp
class Example {
private:
    int value_;                    // Instance variable
    static int count_;            // Class variable
    mutable int cache_;           // Can be modified in const methods
    const int constant_;          // Must be initialized in constructor
};
```

#### Member Functions
```cpp
class Example {
public:
    void regular_method();        // Regular member function
    void const_method() const;    // Cannot modify object state
    static void static_method();  // Class method (no 'this' pointer)
    virtual void virtual_method(); // Can be overridden in derived classes
};
```

## Encapsulation

### Data Hiding
```cpp
class BankAccount {
private:
    double balance_;  // Hidden from external access
    
public:
    // Controlled access through public interface
    double getBalance() const { return balance_; }
    
    bool deposit(double amount) {
        if (amount > 0) {
            balance_ += amount;
            return true;
        }
        return false;
    }
};
```

### Benefits
- **Data Integrity**: Validation in setter methods
- **Maintainability**: Internal implementation can change
- **Security**: Sensitive data protected
- **Debugging**: Controlled access points

### Getters and Setters
```cpp
class Person {
private:
    std::string name_;
    int age_;
    
public:
    // Getter methods (const)
    const std::string& getName() const { return name_; }
    int getAge() const { return age_; }
    
    // Setter methods (with validation)
    void setName(const std::string& name) {
        if (!name.empty()) {
            name_ = name;
        }
    }
    
    void setAge(int age) {
        if (age >= 0 && age <= 150) {
            age_ = age;
        }
    }
};
```

## Constructors and Destructors

### Default Constructor
```cpp
class MyClass {
public:
    MyClass() = default;  // Compiler-generated
    
    // Or custom implementation
    MyClass() : value_(0) {}
    
private:
    int value_;
};
```

### Parameterized Constructor
```cpp
class Rectangle {
private:
    double width_, height_;
    
public:
    Rectangle(double w, double h) : width_(w), height_(h) {
        // Member initializer list preferred
    }
    
    // Constructor with default parameters
    Rectangle(double side = 1.0) : width_(side), height_(side) {}
};
```

### Constructor Delegation (C++11)
```cpp
class Point {
private:
    int x_, y_;
    
public:
    Point(int x, int y) : x_(x), y_(y) {}
    Point(int val) : Point(val, val) {}  // Delegate to two-parameter constructor
    Point() : Point(0) {}                // Delegate to one-parameter constructor
};
```

### Destructor
```cpp
class ResourceManager {
private:
    int* data_;
    
public:
    ResourceManager(size_t size) : data_(new int[size]) {}
    
    ~ResourceManager() {
        delete[] data_;  // Clean up resources
        std::cout << "Resources cleaned up\n";
    }
};
```

### Special Member Functions (Rule of Five)
```cpp
class Resource {
private:
    int* data_;
    size_t size_;
    
public:
    // 1. Destructor
    ~Resource() { delete[] data_; }
    
    // 2. Copy constructor
    Resource(const Resource& other) 
        : size_(other.size_), data_(new int[size_]) {
        std::copy(other.data_, other.data_ + size_, data_);
    }
    
    // 3. Copy assignment operator
    Resource& operator=(const Resource& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = new int[size_];
            std::copy(other.data_, other.data_ + size_, data_);
        }
        return *this;
    }
    
    // 4. Move constructor
    Resource(Resource&& other) noexcept 
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }
    
    // 5. Move assignment operator
    Resource& operator=(Resource&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
};
```

## Copy and Move Semantics

### Copy Semantics
```cpp
class CopyExample {
private:
    std::string data_;
    
public:
    // Copy constructor - creates deep copy
    CopyExample(const CopyExample& other) : data_(other.data_) {
        std::cout << "Copy constructor called\n";
    }
    
    // Copy assignment - replaces existing object
    CopyExample& operator=(const CopyExample& other) {
        if (this != &other) {
            data_ = other.data_;
            std::cout << "Copy assignment called\n";
        }
        return *this;
    }
};
```

### Move Semantics (C++11)
```cpp
class MoveExample {
private:
    std::unique_ptr<int[]> data_;
    size_t size_;
    
public:
    // Move constructor - transfers ownership
    MoveExample(MoveExample&& other) noexcept 
        : data_(std::move(other.data_)), size_(other.size_) {
        other.size_ = 0;
        std::cout << "Move constructor called\n";
    }
    
    // Move assignment - transfers ownership
    MoveExample& operator=(MoveExample&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
            size_ = other.size_;
            other.size_ = 0;
            std::cout << "Move assignment called\n";
        }
        return *this;
    }
};
```

### When Copy/Move is Called
```cpp
void demonstrate_copy_move() {
    CopyExample obj1;
    CopyExample obj2 = obj1;        // Copy constructor
    CopyExample obj3;
    obj3 = obj1;                    // Copy assignment
    
    CopyExample obj4 = std::move(obj1);  // Move constructor
    obj2 = std::move(obj3);              // Move assignment
}
```

## Operator Overloading

### Arithmetic Operators
```cpp
class Complex {
private:
    double real_, imag_;
    
public:
    Complex(double r = 0, double i = 0) : real_(r), imag_(i) {}
    
    // Binary arithmetic operators
    Complex operator+(const Complex& other) const {
        return Complex(real_ + other.real_, imag_ + other.imag_);
    }
    
    Complex operator-(const Complex& other) const {
        return Complex(real_ - other.real_, imag_ - other.imag_);
    }
    
    // Compound assignment operators
    Complex& operator+=(const Complex& other) {
        real_ += other.real_;
        imag_ += other.imag_;
        return *this;
    }
    
    // Unary operators
    Complex operator-() const {
        return Complex(-real_, -imag_);
    }
    
    // Prefix increment
    Complex& operator++() {
        ++real_;
        return *this;
    }
    
    // Postfix increment
    Complex operator++(int) {
        Complex temp = *this;
        ++real_;
        return temp;
    }
};
```

### Comparison Operators
```cpp
class Point {
private:
    int x_, y_;
    
public:
    Point(int x, int y) : x_(x), y_(y) {}
    
    bool operator==(const Point& other) const {
        return x_ == other.x_ && y_ == other.y_;
    }
    
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
    
    bool operator<(const Point& other) const {
        return (x_ < other.x_) || (x_ == other.x_ && y_ < other.y_);
    }
    
    // C++20: Three-way comparison (spaceship operator)
    auto operator<=>(const Point& other) const = default;
};
```

### Stream Operators
```cpp
class Student {
private:
    std::string name_;
    int grade_;
    
public:
    // Friend functions for stream operators
    friend std::ostream& operator<<(std::ostream& os, const Student& s) {
        os << "Student{" << s.name_ << ", " << s.grade_ << "}";
        return os;
    }
    
    friend std::istream& operator>>(std::istream& is, Student& s) {
        is >> s.name_ >> s.grade_;
        return is;
    }
};
```

### Subscript and Function Call Operators
```cpp
class Matrix {
private:
    std::vector<std::vector<int>> data_;
    
public:
    // Subscript operator
    std::vector<int>& operator[](size_t row) {
        return data_[row];
    }
    
    const std::vector<int>& operator[](size_t row) const {
        return data_[row];
    }
    
    // Function call operator (functor)
    int& operator()(size_t row, size_t col) {
        return data_[row][col];
    }
    
    const int& operator()(size_t row, size_t col) const {
        return data_[row][col];
    }
};
```

## Static Members

### Static Data Members
```cpp
class Counter {
private:
    static int count_;  // Shared by all instances
    int id_;
    
public:
    Counter() : id_(++count_) {}
    
    int getId() const { return id_; }
    static int getCount() { return count_; }
};

// Definition required outside class
int Counter::count_ = 0;
```

### Static Member Functions
```cpp
class MathUtils {
public:
    static double pi() { return 3.14159; }
    
    static double circleArea(double radius) {
        return pi() * radius * radius;
    }
    
    // Cannot access non-static members
    // No 'this' pointer available
};

// Usage
double area = MathUtils::circleArea(5.0);
```

### Static Local Variables
```cpp
class Singleton {
private:
    Singleton() = default;
    
public:
    static Singleton& getInstance() {
        static Singleton instance;  // Created only once
        return instance;
    }
    
    // Delete copy operations
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};
```

## Const Correctness

### Const Member Functions
```cpp
class Rectangle {
private:
    double width_, height_;
    
public:
    Rectangle(double w, double h) : width_(w), height_(h) {}
    
    // Const member functions - cannot modify object state
    double getWidth() const { return width_; }
    double getHeight() const { return height_; }
    double area() const { return width_ * height_; }
    
    // Non-const member functions
    void setWidth(double w) { width_ = w; }
    void setHeight(double h) { height_ = h; }
};
```

### Const Objects
```cpp
void demonstrate_const() {
    const Rectangle rect(10, 5);
    
    // OK - calling const member functions
    double w = rect.getWidth();
    double area = rect.area();
    
    // ERROR - cannot call non-const member functions
    // rect.setWidth(20);
}
```

### Mutable Members
```cpp
class CachedCalculator {
private:
    mutable double cached_result_;  // Can be modified in const methods
    mutable bool cache_valid_;
    double value_;
    
public:
    double expensiveCalculation() const {
        if (!cache_valid_) {
            cached_result_ = /* expensive computation */ value_ * value_;
            cache_valid_ = true;
        }
        return cached_result_;
    }
};
```

### Const Overloading
```cpp
class Container {
private:
    std::vector<int> data_;
    
public:
    // Non-const version - returns modifiable reference
    int& at(size_t index) {
        return data_.at(index);
    }
    
    // Const version - returns const reference
    const int& at(size_t index) const {
        return data_.at(index);
    }
};
```

## Friend Functions

### Friend Function Declaration
```cpp
class Vector3D {
private:
    double x_, y_, z_;
    
public:
    Vector3D(double x, double y, double z) : x_(x), y_(y), z_(z) {}
    
    // Friend function can access private members
    friend double dot_product(const Vector3D& a, const Vector3D& b);
    friend std::ostream& operator<<(std::ostream& os, const Vector3D& v);
};

// Friend function definition
double dot_product(const Vector3D& a, const Vector3D& b) {
    return a.x_ * b.x_ + a.y_ * b.y_ + a.z_ * b.z_;
}

std::ostream& operator<<(std::ostream& os, const Vector3D& v) {
    os << "(" << v.x_ << ", " << v.y_ << ", " << v.z_ << ")";
    return os;
}
```

### Friend Classes
```cpp
class Database {
private:
    std::string connection_string_;
    
    friend class DatabaseManager;  // Can access private members
    
public:
    Database(const std::string& conn) : connection_string_(conn) {}
};

class DatabaseManager {
public:
    void configure(Database& db, const std::string& new_conn) {
        db.connection_string_ = new_conn;  // Access private member
    }
};
```

## Composition

### Has-A Relationship
```cpp
class Engine {
private:
    int horsepower_;
    
public:
    Engine(int hp) : horsepower_(hp) {}
    void start() { std::cout << "Engine started\n"; }
    int getHorsepower() const { return horsepower_; }
};

class Car {
private:
    Engine engine_;        // Composition: Car HAS-A Engine
    std::string model_;
    
public:
    Car(const std::string& model, int hp) 
        : model_(model), engine_(hp) {}
    
    void startCar() {
        std::cout << "Starting " << model_ << "\n";
        engine_.start();
    }
    
    int getEnginePower() const {
        return engine_.getHorsepower();
    }
};
```

### Aggregation vs Composition
```cpp
// Composition: Strong ownership (Car owns Engine)
class Car {
    Engine engine_;  // Engine lifetime tied to Car
};

// Aggregation: Weak ownership (University uses Professor)
class University {
    std::vector<Professor*> professors_;  // Professors exist independently
};
```

## Best Practices

### 1. Design Principles

#### Single Responsibility
```cpp
// GOOD: Each class has one responsibility
class FileReader {
public:
    std::string readFile(const std::string& filename);
};

class DataParser {
public:
    std::vector<Record> parseData(const std::string& data);
};

// BAD: Multiple responsibilities
class FileProcessor {
public:
    std::string readFile(const std::string& filename);
    std::vector<Record> parseData(const std::string& data);
    void saveToDatabase(const std::vector<Record>& records);
};
```

#### Encapsulation
```cpp
// GOOD: Proper encapsulation
class BankAccount {
private:
    double balance_;
    
public:
    bool withdraw(double amount) {
        if (amount > 0 && amount <= balance_) {
            balance_ -= amount;
            return true;
        }
        return false;
    }
};

// BAD: Exposed internal state
class BadAccount {
public:
    double balance_;  // Anyone can modify
};
```

### 2. Constructor Guidelines

#### Prefer Initialization Lists
```cpp
// GOOD: Member initializer list
class Point {
    int x_, y_;
public:
    Point(int x, int y) : x_(x), y_(y) {}  // Direct initialization
};

// LESS EFFICIENT: Assignment in body
class Point {
    int x_, y_;
public:
    Point(int x, int y) {
        x_ = x;  // Default construction + assignment
        y_ = y;
    }
};
```

#### Handle Resource Acquisition
```cpp
class ResourceManager {
private:
    std::unique_ptr<Resource> resource_;
    
public:
    ResourceManager() : resource_(std::make_unique<Resource>()) {
        if (!resource_) {
            throw std::runtime_error("Failed to acquire resource");
        }
    }
};
```

### 3. Const Correctness
```cpp
class Container {
public:
    // Always mark functions const when they don't modify state
    size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }
    
    // Provide const and non-const overloads for access methods
    T& operator[](size_t index) { return data_[index]; }
    const T& operator[](size_t index) const { return data_[index]; }
    
private:
    std::vector<T> data_;
};
```

### 4. Rule of Zero/Three/Five
```cpp
// Rule of Zero: Let compiler generate special members
class SimpleClass {
    std::string name_;
    std::vector<int> data_;
    // Compiler-generated destructor, copy/move operations are fine
};

// Rule of Five: If you define one, define all
class ResourceClass {
public:
    ~ResourceClass();                              // Destructor
    ResourceClass(const ResourceClass&);           // Copy constructor
    ResourceClass& operator=(const ResourceClass&); // Copy assignment
    ResourceClass(ResourceClass&&);                // Move constructor
    ResourceClass& operator=(ResourceClass&&);     // Move assignment
};
```

## Interview Questions

### Basic Level

1. **What is the difference between a class and an object?**
   - Class: Blueprint/template that defines structure and behavior
   - Object: Instance of a class with actual memory allocation
   - Class is a type definition; object is a variable of that type

2. **What are access specifiers in C++?**
   - private: Accessible only within the class
   - protected: Accessible within class and derived classes
   - public: Accessible from anywhere

3. **What is encapsulation?**
   - Bundling data and methods that operate on that data
   - Hiding internal implementation details
   - Controlling access through public interfaces

### Intermediate Level

4. **Explain the difference between copy constructor and assignment operator.**
   - Copy constructor: Creates new object from existing one
   - Assignment operator: Copies data to already existing object
   - Copy constructor called during initialization; assignment during assignment

5. **What is the Rule of Three/Five?**
   - If you define destructor, copy constructor, or copy assignment, define all three
   - Rule of Five adds move constructor and move assignment (C++11)
   - Ensures proper resource management

6. **What are static members and when to use them?**
   - Shared among all instances of the class
   - Static data: Class-level variables
   - Static methods: Can be called without object instance
   - Use for class-wide properties or utility functions

### Advanced Level

7. **Explain const correctness in C++.**
   - Practice of properly using const keyword
   - Const methods cannot modify object state
   - Const objects can only call const methods
   - Helps prevent accidental modifications and enables optimizations

8. **What is the difference between composition and aggregation?**
   - Composition: Strong ownership (part cannot exist without whole)
   - Aggregation: Weak ownership (parts can exist independently)
   - Example: Car has Engine (composition) vs University has Students (aggregation)

9. **How do you implement operator overloading properly?**
   - Return appropriate types (reference for assignment, value for arithmetic)
   - Maintain symmetry and expected behavior
   - Use friend functions for binary operators when needed
   - Follow conventions (+ returns by value, += returns by reference)

### Expert Level

10. **Explain move semantics and when they're beneficial.**
    - Transfer ownership instead of copying
    - Beneficial for expensive-to-copy objects
    - Automatic for temporary objects
    - Use std::move() to force move semantics

11. **What is RAII and how does it relate to destructors?**
    - Resource Acquisition Is Initialization
    - Acquire resources in constructor, release in destructor
    - Ensures automatic cleanup when object goes out of scope
    - Prevents resource leaks and simplifies exception safety

12. **How do you design exception-safe classes?**
    - Use RAII for automatic resource management
    - Prefer smart pointers over raw pointers
    - Implement strong exception safety guarantee
    - Use copy-and-swap idiom for assignment operators

## Summary

Classes and objects form the foundation of C++ object-oriented programming:

**Core Concepts:**
- Encapsulation through access specifiers
- Constructor/destructor lifecycle management
- Copy and move semantics for efficiency
- Operator overloading for natural syntax
- Static members for class-level functionality

**Best Practices:**
- Follow Rule of Zero/Three/Five
- Maintain const correctness
- Use RAII for resource management
- Prefer composition over inheritance
- Design clear, minimal public interfaces

**Key Benefits:**
- Code organization and modularity
- Data hiding and security
- Reusability and maintainability
- Type safety and abstraction

Understanding these concepts is essential for writing robust, efficient, and maintainable C++ code.
