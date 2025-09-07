#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <typeinfo>
#include <stdexcept>

// Abstract base class for polymorphism demonstration
class Shape {
protected:
    std::string name_;
    
public:
    Shape(const std::string& name) : name_(name) {}
    virtual ~Shape() = default;
    
    // Pure virtual functions - interface
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
    virtual void draw() const = 0;
    
    // Virtual function with default implementation
    virtual void displayInfo() const {
        std::cout << "Shape: " << name_ 
                  << ", Area: " << area() 
                  << ", Perimeter: " << perimeter() << std::endl;
    }
    
    // Non-virtual function
    std::string getName() const { return name_; }
    
    // Virtual function for cloning (virtual constructor idiom)
    virtual std::unique_ptr<Shape> clone() const = 0;
};

// Concrete derived classes
class Circle : public Shape {
private:
    double radius_;
    static constexpr double PI = 3.14159;
    
public:
    Circle(double radius) : Shape("Circle"), radius_(radius) {}
    
    double area() const override {
        return PI * radius_ * radius_;
    }
    
    double perimeter() const override {
        return 2 * PI * radius_;
    }
    
    void draw() const override {
        std::cout << "Drawing circle with radius " << radius_ << std::endl;
    }
    
    std::unique_ptr<Shape> clone() const override {
        return std::make_unique<Circle>(*this);
    }
    
    double getRadius() const { return radius_; }
};

class Rectangle : public Shape {
private:
    double width_, height_;
    
public:
    Rectangle(double width, double height) 
        : Shape("Rectangle"), width_(width), height_(height) {}
    
    double area() const override {
        return width_ * height_;
    }
    
    double perimeter() const override {
        return 2 * (width_ + height_);
    }
    
    void draw() const override {
        std::cout << "Drawing rectangle " << width_ << "x" << height_ << std::endl;
    }
    
    std::unique_ptr<Shape> clone() const override {
        return std::make_unique<Rectangle>(*this);
    }
    
    double getWidth() const { return width_; }
    double getHeight() const { return height_; }
};

class Triangle : public Shape {
private:
    double side1_, side2_, side3_;
    
public:
    Triangle(double s1, double s2, double s3) 
        : Shape("Triangle"), side1_(s1), side2_(s2), side3_(s3) {}
    
    double area() const override {
        // Using Heron's formula
        double s = perimeter() / 2;
        return sqrt(s * (s - side1_) * (s - side2_) * (s - side3_));
    }
    
    double perimeter() const override {
        return side1_ + side2_ + side3_;
    }
    
    void draw() const override {
        std::cout << "Drawing triangle with sides " 
                  << side1_ << ", " << side2_ << ", " << side3_ << std::endl;
    }
    
    std::unique_ptr<Shape> clone() const override {
        return std::make_unique<Triangle>(*this);
    }
};

// Animal hierarchy for another polymorphism example
class Animal {
protected:
    std::string species_;
    int age_;
    
public:
    Animal(const std::string& species, int age) : species_(species), age_(age) {}
    virtual ~Animal() = default;
    
    virtual void makeSound() const = 0;
    virtual void move() const = 0;
    virtual void eat() const {
        std::cout << species_ << " is eating" << std::endl;
    }
    
    virtual std::string getType() const { return "Animal"; }
    
    std::string getSpecies() const { return species_; }
    int getAge() const { return age_; }
};

class Mammal : public Animal {
public:
    Mammal(const std::string& species, int age) : Animal(species, age) {}
    
    virtual void nurse() const {
        std::cout << species_ << " is nursing offspring" << std::endl;
    }
    
    std::string getType() const override { return "Mammal"; }
};

class Bird : public Animal {
protected:
    bool canFly_;
    
public:
    Bird(const std::string& species, int age, bool canFly = true) 
        : Animal(species, age), canFly_(canFly) {}
    
    virtual void fly() const {
        if (canFly_) {
            std::cout << species_ << " is flying" << std::endl;
        } else {
            std::cout << species_ << " cannot fly" << std::endl;
        }
    }
    
    std::string getType() const override { return "Bird"; }
};

// Concrete animal classes
class Dog : public Mammal {
private:
    std::string breed_;
    
public:
    Dog(const std::string& breed, int age) : Mammal("Dog", age), breed_(breed) {}
    
    void makeSound() const override {
        std::cout << "Woof! Woof!" << std::endl;
    }
    
    void move() const override {
        std::cout << "Running on four legs" << std::endl;
    }
    
    void wagTail() const {
        std::cout << "Wagging tail happily" << std::endl;
    }
    
    std::string getBreed() const { return breed_; }
};

class Cat : public Mammal {
public:
    Cat(int age) : Mammal("Cat", age) {}
    
    void makeSound() const override {
        std::cout << "Meow! Meow!" << std::endl;
    }
    
    void move() const override {
        std::cout << "Walking gracefully" << std::endl;
    }
    
    void purr() const {
        std::cout << "Purring contentedly" << std::endl;
    }
};

class Eagle : public Bird {
private:
    double wingspan_;
    
public:
    Eagle(int age, double wingspan) : Bird("Eagle", age, true), wingspan_(wingspan) {}
    
    void makeSound() const override {
        std::cout << "SCREECH!" << std::endl;
    }
    
    void move() const override {
        std::cout << "Soaring high in the sky" << std::endl;
    }
    
    void hunt() const {
        std::cout << "Hunting with sharp talons" << std::endl;
    }
    
    double getWingspan() const { return wingspan_; }
};

class Penguin : public Bird {
public:
    Penguin(int age) : Bird("Penguin", age, false) {}  // Cannot fly
    
    void makeSound() const override {
        std::cout << "Squawk! Squawk!" << std::endl;
    }
    
    void move() const override {
        std::cout << "Waddling and swimming" << std::endl;
    }
    
    void swim() const {
        std::cout << "Swimming gracefully underwater" << std::endl;
    }
};

// Function overloading vs function overriding demonstration
class Base {
public:
    virtual void display() const {
        std::cout << "Base::display()" << std::endl;
    }
    
    virtual void display(int x) const {
        std::cout << "Base::display(int): " << x << std::endl;
    }
    
    virtual void virtualFunc() const {
        std::cout << "Base::virtualFunc()" << std::endl;
    }
    
    void nonVirtualFunc() const {
        std::cout << "Base::nonVirtualFunc()" << std::endl;
    }
};

class Derived : public Base {
public:
    void display() const override {
        std::cout << "Derived::display()" << std::endl;
    }
    
    // If we override one overload, we hide all base overloads
    // To bring back base overloads:
    using Base::display;
    
    void virtualFunc() const override {
        std::cout << "Derived::virtualFunc()" << std::endl;
    }
    
    void nonVirtualFunc() const {  // Hides base version
        std::cout << "Derived::nonVirtualFunc()" << std::endl;
    }
};

// Function templates with polymorphism
template<typename T>
void processAnimal(const T& animal) {
    std::cout << "Processing " << animal.getSpecies() << ":" << std::endl;
    animal.makeSound();
    animal.move();
    animal.eat();
    std::cout << std::endl;
}

// Runtime polymorphism with virtual functions
void processAnimalsPolymorphically(const std::vector<std::unique_ptr<Animal>>& animals) {
    std::cout << "=== Polymorphic Processing ===\n";
    
    for (const auto& animal : animals) {
        std::cout << "Type: " << animal->getType() 
                  << ", Species: " << animal->getSpecies() << std::endl;
        animal->makeSound();
        animal->move();
        animal->eat();
        
        // Dynamic casting for specific behavior
        if (auto mammal = dynamic_cast<const Mammal*>(animal.get())) {
            mammal->nurse();
        }
        
        if (auto bird = dynamic_cast<const Bird*>(animal.get())) {
            bird->fly();
        }
        
        std::cout << std::endl;
    }
}

void demonstrateBasicPolymorphism() {
    std::cout << "=== Basic Polymorphism with Shapes ===\n\n";
    
    // Create shapes polymorphically
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(5.0));
    shapes.push_back(std::make_unique<Rectangle>(4.0, 6.0));
    shapes.push_back(std::make_unique<Triangle>(3.0, 4.0, 5.0));
    shapes.push_back(std::make_unique<Circle>(2.5));
    
    // Process shapes polymorphically
    std::cout << "Processing shapes:\n";
    for (const auto& shape : shapes) {
        shape->draw();           // Calls overridden version
        shape->displayInfo();    // Calls virtual function
        std::cout << std::endl;
    }
    
    // Demonstrate virtual constructor (cloning)
    std::cout << "Cloning shapes:\n";
    std::vector<std::unique_ptr<Shape>> cloned_shapes;
    for (const auto& shape : shapes) {
        cloned_shapes.push_back(shape->clone());
    }
    
    for (const auto& shape : cloned_shapes) {
        std::cout << "Cloned: ";
        shape->displayInfo();
    }
    
    std::cout << std::endl;
}

void demonstrateAnimalPolymorphism() {
    std::cout << "=== Animal Hierarchy Polymorphism ===\n\n";
    
    // Create animals
    std::vector<std::unique_ptr<Animal>> animals;
    animals.push_back(std::make_unique<Dog>("Golden Retriever", 5));
    animals.push_back(std::make_unique<Cat>(3));
    animals.push_back(std::make_unique<Eagle>(7, 2.3));
    animals.push_back(std::make_unique<Penguin>(4));
    
    processAnimalsPolymorphically(animals);
}

void demonstrateStaticVsDynamicBinding() {
    std::cout << "=== Static vs Dynamic Binding ===\n\n";
    
    Derived derived;
    Base* base_ptr = &derived;
    Base& base_ref = derived;
    
    std::cout << "Virtual function calls (dynamic binding):\n";
    base_ptr->virtualFunc();    // Calls Derived::virtualFunc()
    base_ref.virtualFunc();     // Calls Derived::virtualFunc()
    derived.virtualFunc();      // Calls Derived::virtualFunc()
    
    std::cout << "\nNon-virtual function calls (static binding):\n";
    base_ptr->nonVirtualFunc(); // Calls Base::nonVirtualFunc()
    base_ref.nonVirtualFunc();  // Calls Base::nonVirtualFunc()
    derived.nonVirtualFunc();   // Calls Derived::nonVirtualFunc()
    
    std::cout << "\nFunction overloading with inheritance:\n";
    derived.display();          // Derived version
    derived.display(42);        // Base version (via using declaration)
    
    std::cout << std::endl;
}

void demonstrateDynamicCasting() {
    std::cout << "=== Dynamic Casting ===\n\n";
    
    std::vector<std::unique_ptr<Animal>> animals;
    animals.push_back(std::make_unique<Dog>("Labrador", 4));
    animals.push_back(std::make_unique<Eagle>(6, 2.1));
    animals.push_back(std::make_unique<Cat>(2));
    animals.push_back(std::make_unique<Penguin>(3));
    
    for (const auto& animal : animals) {
        std::cout << "Animal: " << animal->getSpecies() << std::endl;
        
        // Safe downcasting with dynamic_cast
        if (auto dog = dynamic_cast<Dog*>(animal.get())) {
            std::cout << "  - It's a dog! Breed: " << dog->getBreed() << std::endl;
            dog->wagTail();
        }
        else if (auto cat = dynamic_cast<Cat*>(animal.get())) {
            std::cout << "  - It's a cat!" << std::endl;
            cat->purr();
        }
        else if (auto eagle = dynamic_cast<Eagle*>(animal.get())) {
            std::cout << "  - It's an eagle! Wingspan: " << eagle->getWingspan() << "m" << std::endl;
            eagle->hunt();
        }
        else if (auto penguin = dynamic_cast<Penguin*>(animal.get())) {
            std::cout << "  - It's a penguin!" << std::endl;
            penguin->swim();
        }
        
        // Type checking with typeid
        std::cout << "  - Type info: " << typeid(*animal).name() << std::endl;
        
        std::cout << std::endl;
    }
}

void demonstrateVirtualFunctionPerformance() {
    std::cout << "=== Virtual Function Performance Demo ===\n\n";
    
    const int iterations = 1000000;
    
    // Direct function call
    Circle circle(1.0);
    auto start = std::chrono::high_resolution_clock::now();
    
    double total_area = 0.0;
    for (int i = 0; i < iterations; ++i) {
        total_area += circle.area();  // Direct call
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto direct_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Virtual function call
    Shape* shape_ptr = &circle;
    start = std::chrono::high_resolution_clock::now();
    
    total_area = 0.0;
    for (int i = 0; i < iterations; ++i) {
        total_area += shape_ptr->area();  // Virtual call
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto virtual_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Performance comparison (" << iterations << " iterations):\n";
    std::cout << "Direct calls: " << direct_time.count() << " microseconds\n";
    std::cout << "Virtual calls: " << virtual_time.count() << " microseconds\n";
    std::cout << "Overhead: " << static_cast<double>(virtual_time.count()) / direct_time.count() 
              << "x\n\n";
}

// Template-based polymorphism (compile-time)
template<typename ShapeType>
void processShapeTemplate(const ShapeType& shape) {
    shape.draw();
    std::cout << "Area: " << shape.area() << std::endl;
}

void demonstrateCompileTimePolymorphism() {
    std::cout << "=== Compile-Time Polymorphism (Templates) ===\n\n";
    
    Circle circle(3.0);
    Rectangle rectangle(4.0, 5.0);
    
    std::cout << "Template-based processing:\n";
    processShapeTemplate(circle);     // No virtual function overhead
    processShapeTemplate(rectangle);  // Type resolved at compile time
    
    std::cout << std::endl;
}

// Polymorphic factory pattern
class ShapeFactory {
public:
    enum class ShapeType { CIRCLE, RECTANGLE, TRIANGLE };
    
    static std::unique_ptr<Shape> createShape(ShapeType type) {
        switch (type) {
            case ShapeType::CIRCLE:
                return std::make_unique<Circle>(1.0);
            case ShapeType::RECTANGLE:
                return std::make_unique<Rectangle>(2.0, 3.0);
            case ShapeType::TRIANGLE:
                return std::make_unique<Triangle>(3.0, 4.0, 5.0);
            default:
                throw std::invalid_argument("Unknown shape type");
        }
    }
};

void demonstratePolymorphicFactory() {
    std::cout << "=== Polymorphic Factory Pattern ===\n\n";
    
    auto shapes = {
        ShapeFactory::ShapeType::CIRCLE,
        ShapeFactory::ShapeType::RECTANGLE,
        ShapeFactory::ShapeType::TRIANGLE
    };
    
    std::cout << "Creating shapes through factory:\n";
    for (auto shape_type : shapes) {
        auto shape = ShapeFactory::createShape(shape_type);
        shape->draw();
        shape->displayInfo();
        std::cout << std::endl;
    }
}

int main() {
    demonstrateBasicPolymorphism();
    demonstrateAnimalPolymorphism();
    demonstrateStaticVsDynamicBinding();
    demonstrateDynamicCasting();
    demonstrateVirtualFunctionPerformance();
    demonstrateCompileTimePolymorphism();
    demonstratePolymorphicFactory();
    
    std::cout << "=== Key Polymorphism Concepts ===\n";
    std::cout << "1. Runtime polymorphism through virtual functions\n";
    std::cout << "2. Dynamic binding based on object's actual type\n";
    std::cout << "3. Abstract classes define interfaces\n";
    std::cout << "4. Dynamic casting for safe downcasting\n";
    std::cout << "5. Virtual function table mechanism\n";
    std::cout << "6. Compile-time polymorphism through templates\n";
    std::cout << "7. Factory pattern for polymorphic object creation\n";
    std::cout << "8. Performance considerations of virtual calls\n";
    
    return 0;
}
