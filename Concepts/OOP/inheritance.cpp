#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>

// Base class demonstrating basic inheritance
class Animal {
protected:
    std::string name_;
    int age_;
    
public:
    Animal(const std::string& name, int age) : name_(name), age_(age) {
        std::cout << "Animal constructor: " << name_ << std::endl;
    }
    
    virtual ~Animal() {
        std::cout << "Animal destructor: " << name_ << std::endl;
    }
    
    // Virtual function - can be overridden
    virtual void makeSound() const {
        std::cout << name_ << " makes a generic animal sound" << std::endl;
    }
    
    // Virtual function with default implementation
    virtual void sleep() const {
        std::cout << name_ << " is sleeping" << std::endl;
    }
    
    // Non-virtual function - cannot be overridden (but can be hidden)
    void displayInfo() const {
        std::cout << "Name: " << name_ << ", Age: " << age_ << std::endl;
    }
    
    // Pure virtual function - makes class abstract
    virtual void move() const = 0;
    
    // Protected methods accessible to derived classes
protected:
    void setAge(int age) { age_ = age; }
    
    // Getters
public:
    const std::string& getName() const { return name_; }
    int getAge() const { return age_; }
};

// Derived class - public inheritance
class Dog : public Animal {
private:
    std::string breed_;
    
public:
    Dog(const std::string& name, int age, const std::string& breed)
        : Animal(name, age), breed_(breed) {
        std::cout << "Dog constructor: " << name_ << " (" << breed_ << ")" << std::endl;
    }
    
    ~Dog() override {
        std::cout << "Dog destructor: " << name_ << std::endl;
    }
    
    // Override virtual function
    void makeSound() const override {
        std::cout << name_ << " barks: Woof! Woof!" << std::endl;
    }
    
    // Implement pure virtual function
    void move() const override {
        std::cout << name_ << " runs on four legs" << std::endl;
    }
    
    // Additional behavior specific to Dog
    void wagTail() const {
        std::cout << name_ << " wags tail happily" << std::endl;
    }
    
    void fetch() const {
        std::cout << name_ << " fetches the ball" << std::endl;
    }
    
    const std::string& getBreed() const { return breed_; }
};

// Another derived class
class Cat : public Animal {
private:
    bool isIndoor_;
    
public:
    Cat(const std::string& name, int age, bool isIndoor = true)
        : Animal(name, age), isIndoor_(isIndoor) {
        std::cout << "Cat constructor: " << name_ << std::endl;
    }
    
    ~Cat() override {
        std::cout << "Cat destructor: " << name_ << std::endl;
    }
    
    void makeSound() const override {
        std::cout << name_ << " meows: Meow! Meow!" << std::endl;
    }
    
    void move() const override {
        std::cout << name_ << " walks gracefully and climbs" << std::endl;
    }
    
    // Override with different behavior
    void sleep() const override {
        std::cout << name_ << " sleeps 16 hours a day" << std::endl;
    }
    
    void purr() const {
        std::cout << name_ << " purrs contentedly" << std::endl;
    }
    
    bool isIndoor() const { return isIndoor_; }
};

// Bird class showing different inheritance
class Bird : public Animal {
protected:
    bool canFly_;
    
public:
    Bird(const std::string& name, int age, bool canFly = true)
        : Animal(name, age), canFly_(canFly) {
        std::cout << "Bird constructor: " << name_ << std::endl;
    }
    
    ~Bird() override {
        std::cout << "Bird destructor: " << name_ << std::endl;
    }
    
    void makeSound() const override {
        std::cout << name_ << " chirps melodiously" << std::endl;
    }
    
    void move() const override {
        if (canFly_) {
            std::cout << name_ << " flies through the air" << std::endl;
        } else {
            std::cout << name_ << " walks and hops" << std::endl;
        }
    }
    
    virtual void fly() const {
        if (canFly_) {
            std::cout << name_ << " soars high in the sky" << std::endl;
        } else {
            std::cout << name_ << " cannot fly" << std::endl;
        }
    }
};

// Multi-level inheritance
class Eagle : public Bird {
private:
    double wingspan_;
    
public:
    Eagle(const std::string& name, int age, double wingspan)
        : Bird(name, age, true), wingspan_(wingspan) {
        std::cout << "Eagle constructor: " << name_ << std::endl;
    }
    
    ~Eagle() override {
        std::cout << "Eagle destructor: " << name_ << std::endl;
    }
    
    void makeSound() const override {
        std::cout << name_ << " screeches: SCREECH!" << std::endl;
    }
    
    void fly() const override {
        std::cout << name_ << " soars majestically with " << wingspan_ 
                  << " meter wingspan" << std::endl;
    }
    
    void hunt() const {
        std::cout << name_ << " hunts with sharp talons" << std::endl;
    }
    
    double getWingspan() const { return wingspan_; }
};

// Example of private inheritance (IS-IMPLEMENTED-IN-TERMS-OF)
class Timer {
protected:
    int seconds_;
    
public:
    Timer(int seconds = 0) : seconds_(seconds) {}
    virtual ~Timer() = default;
    
    void tick() { ++seconds_; }
    int getTime() const { return seconds_; }
    void reset() { seconds_ = 0; }
};

class StopWatch : private Timer {  // Private inheritance
public:
    StopWatch() : Timer(0) {}
    
    void start() { 
        reset();
        std::cout << "Stopwatch started" << std::endl;
    }
    
    void stop() {
        std::cout << "Stopwatch stopped at " << getTime() << " seconds" << std::endl;
    }
    
    // Expose only specific functionality
    using Timer::tick;        // Make tick() public
    using Timer::getTime;     // Make getTime() public
    
    // Timer::reset() remains private
};

// Protected inheritance example (rarely used)
class Vehicle {
protected:
    std::string engine_type_;
    
public:
    Vehicle(const std::string& engine) : engine_type_(engine) {}
    virtual ~Vehicle() = default;
    
    void startEngine() {
        std::cout << "Starting " << engine_type_ << " engine" << std::endl;
    }
};

class Car : protected Vehicle {  // Protected inheritance
public:
    Car(const std::string& engine) : Vehicle(engine) {}
    
    void drive() {
        startEngine();  // Available through protected inheritance
        std::cout << "Car is driving" << std::endl;
    }
    
    // Expose startEngine to public
    using Vehicle::startEngine;
};

// Abstract base class for demonstration
class Shape {
public:
    virtual ~Shape() = default;
    
    // Pure virtual functions
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
    virtual void draw() const = 0;
    
    // Virtual function with default implementation
    virtual void displayInfo() const {
        std::cout << "Area: " << area() << ", Perimeter: " << perimeter() << std::endl;
    }
};

class Rectangle : public Shape {
private:
    double width_, height_;
    
public:
    Rectangle(double width, double height) : width_(width), height_(height) {}
    
    double area() const override {
        return width_ * height_;
    }
    
    double perimeter() const override {
        return 2 * (width_ + height_);
    }
    
    void draw() const override {
        std::cout << "Drawing rectangle: " << width_ << " x " << height_ << std::endl;
    }
};

class Circle : public Shape {
private:
    double radius_;
    static constexpr double PI = 3.14159;
    
public:
    Circle(double radius) : radius_(radius) {}
    
    double area() const override {
        return PI * radius_ * radius_;
    }
    
    double perimeter() const override {
        return 2 * PI * radius_;
    }
    
    void draw() const override {
        std::cout << "Drawing circle with radius: " << radius_ << std::endl;
    }
};

// Demonstrating virtual destructors
class Base {
public:
    Base() { std::cout << "Base constructor" << std::endl; }
    
    // Virtual destructor ensures proper cleanup
    virtual ~Base() { std::cout << "Base destructor" << std::endl; }
    
    virtual void process() { std::cout << "Base processing" << std::endl; }
};

class Derived : public Base {
private:
    int* data_;
    
public:
    Derived() : data_(new int[100]) {
        std::cout << "Derived constructor" << std::endl;
    }
    
    ~Derived() override {
        delete[] data_;
        std::cout << "Derived destructor" << std::endl;
    }
    
    void process() override {
        std::cout << "Derived processing" << std::endl;
    }
};

// Function overriding vs function hiding
class BaseClass {
public:
    virtual void virtualFunc() {
        std::cout << "BaseClass::virtualFunc()" << std::endl;
    }
    
    void nonVirtualFunc() {
        std::cout << "BaseClass::nonVirtualFunc()" << std::endl;
    }
    
    virtual void overloadedFunc(int x) {
        std::cout << "BaseClass::overloadedFunc(int): " << x << std::endl;
    }
    
    virtual void overloadedFunc(double x) {
        std::cout << "BaseClass::overloadedFunc(double): " << x << std::endl;
    }
};

class DerivedClass : public BaseClass {
public:
    void virtualFunc() override {
        std::cout << "DerivedClass::virtualFunc()" << std::endl;
    }
    
    void nonVirtualFunc() {  // Hides base class function
        std::cout << "DerivedClass::nonVirtualFunc()" << std::endl;
    }
    
    void overloadedFunc(int x) override {
        std::cout << "DerivedClass::overloadedFunc(int): " << x << std::endl;
    }
    
    // If we don't override all overloads, they get hidden
    // To bring back base class overloads:
    using BaseClass::overloadedFunc;
};

void demonstrateBasicInheritance() {
    std::cout << "=== Basic Inheritance Demonstration ===\n\n";
    
    // Cannot instantiate abstract class
    // Animal animal("Generic", 5);  // ERROR: Animal is abstract
    
    std::cout << "Creating objects:\n";
    Dog dog("Buddy", 3, "Golden Retriever");
    Cat cat("Whiskers", 2, true);
    Eagle eagle("Liberty", 5, 2.3);
    
    std::cout << "\nCalling virtual functions:\n";
    
    // Polymorphism through base class pointers
    std::vector<std::unique_ptr<Animal>> animals;
    animals.push_back(std::make_unique<Dog>("Rex", 4, "German Shepherd"));
    animals.push_back(std::make_unique<Cat>("Mittens", 1));
    animals.push_back(std::make_unique<Eagle>("Storm", 7, 2.1));
    
    for (const auto& animal : animals) {
        animal->makeSound();  // Calls overridden version
        animal->move();       // Calls overridden version
        animal->sleep();      // May call overridden version
        animal->displayInfo(); // Calls base class version
        std::cout << std::endl;
    }
    
    std::cout << "Derived class specific functionality:\n";
    dog.wagTail();
    dog.fetch();
    cat.purr();
    eagle.hunt();
    
    std::cout << std::endl;
}

void demonstrateInheritanceTypes() {
    std::cout << "=== Different Inheritance Types ===\n\n";
    
    std::cout << "1. Public Inheritance (IS-A relationship):\n";
    Dog dog("Buddy", 3, "Labrador");
    Animal* animal_ptr = &dog;  // OK: Dog IS-A Animal
    animal_ptr->makeSound();
    
    std::cout << "\n2. Private Inheritance (IS-IMPLEMENTED-IN-TERMS-OF):\n";
    StopWatch stopwatch;
    stopwatch.start();
    stopwatch.tick();
    stopwatch.tick();
    stopwatch.stop();
    // stopwatch.reset();  // ERROR: reset() is private
    
    std::cout << "\n3. Protected Inheritance:\n";
    Car car("V8");
    car.drive();
    car.startEngine();  // Made public through 'using' declaration
    
    std::cout << std::endl;
}

void demonstratePolymorphism() {
    std::cout << "=== Polymorphism Demonstration ===\n\n";
    
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Rectangle>(5.0, 3.0));
    shapes.push_back(std::make_unique<Circle>(2.5));
    shapes.push_back(std::make_unique<Rectangle>(4.0, 4.0));
    shapes.push_back(std::make_unique<Circle>(1.0));
    
    std::cout << "Processing shapes polymorphically:\n";
    for (const auto& shape : shapes) {
        shape->draw();
        shape->displayInfo();
        std::cout << std::endl;
    }
}

void demonstrateVirtualDestructors() {
    std::cout << "=== Virtual Destructors Demonstration ===\n\n";
    
    std::cout << "With virtual destructor:\n";
    {
        std::unique_ptr<Base> ptr = std::make_unique<Derived>();
        ptr->process();
    }  // Proper destruction: Derived destructor called first
    
    std::cout << "\nWithout virtual destructor (problematic):\n";
    // If Base destructor wasn't virtual:
    // Base* ptr = new Derived();
    // delete ptr;  // Would only call Base destructor, memory leak!
}

void demonstrateFunctionOverriding() {
    std::cout << "=== Function Overriding vs Hiding ===\n\n";
    
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    std::cout << "Virtual function overriding:\n";
    base_ptr->virtualFunc();     // Calls DerivedClass version
    derived.virtualFunc();       // Calls DerivedClass version
    
    std::cout << "\nFunction hiding (non-virtual):\n";
    base_ptr->nonVirtualFunc();  // Calls BaseClass version
    derived.nonVirtualFunc();    // Calls DerivedClass version
    
    std::cout << "\nFunction overloading with inheritance:\n";
    derived.overloadedFunc(42);      // DerivedClass version
    derived.overloadedFunc(3.14);    // BaseClass version (via using declaration)
    
    std::cout << std::endl;
}

void demonstrateConstructorDestructorOrder() {
    std::cout << "=== Constructor/Destructor Order ===\n\n";
    
    std::cout << "Creating Eagle (multi-level inheritance):\n";
    {
        Eagle eagle("Swift", 3, 1.8);
        std::cout << "\nEagle object exists here\n";
    }
    std::cout << "Eagle object destroyed\n\n";
}

void demonstrateSlicing() {
    std::cout << "=== Object Slicing Demonstration ===\n\n";
    
    Dog dog("Buddy", 3, "Bulldog");
    
    std::cout << "No slicing (using references/pointers):\n";
    Animal& animal_ref = dog;
    animal_ref.makeSound();  // Calls Dog::makeSound()
    
    std::cout << "\nObject slicing (copying to base class):\n";
    Animal animal_copy = dog;  // Slicing occurs! Only Animal part copied
    animal_copy.makeSound();   // Calls Animal::makeSound()
    
    std::cout << std::endl;
}

int main() {
    demonstrateBasicInheritance();
    demonstrateInheritanceTypes();
    demonstratePolymorphism();
    demonstrateVirtualDestructors();
    demonstrateFunctionOverriding();
    demonstrateConstructorDestructorOrder();
    demonstrateSlicing();
    
    std::cout << "=== Key Inheritance Concepts ===\n";
    std::cout << "1. IS-A relationship through public inheritance\n";
    std::cout << "2. Virtual functions enable polymorphism\n";
    std::cout << "3. Pure virtual functions create abstract classes\n";
    std::cout << "4. Virtual destructors ensure proper cleanup\n";
    std::cout << "5. Function overriding vs function hiding\n";
    std::cout << "6. Constructor/destructor call order\n";
    std::cout << "7. Object slicing with value semantics\n";
    std::cout << "8. Different inheritance types (public, private, protected)\n";
    
    return 0;
}
