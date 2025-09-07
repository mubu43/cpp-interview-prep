#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <type_traits>

// CRTP - Curiously Recurring Template Pattern
// Base template class that takes derived class as template parameter

// Example 1: Static Polymorphism with CRTP
template<typename Derived>
class Shape {
public:
    void draw() {
        static_cast<Derived*>(this)->drawImpl();
    }
    
    double area() {
        return static_cast<Derived*>(this)->areaImpl();
    }
    
    void print() {
        std::cout << "Shape: " << static_cast<Derived*>(this)->getName() 
                  << ", Area: " << area() << std::endl;
    }
    
    // Static interface checking
    static_assert(std::is_base_of_v<Shape<Derived>, Derived>, 
                  "Derived must inherit from Shape<Derived>");
};

class Circle : public Shape<Circle> {
private:
    double radius_;

public:
    explicit Circle(double radius) : radius_(radius) {}
    
    void drawImpl() {
        std::cout << "Drawing a circle with radius " << radius_ << std::endl;
    }
    
    double areaImpl() {
        return 3.14159 * radius_ * radius_;
    }
    
    std::string getName() {
        return "Circle";
    }
    
    // Circle-specific methods
    double getRadius() const { return radius_; }
};

class Rectangle : public Shape<Rectangle> {
private:
    double width_, height_;

public:
    Rectangle(double width, double height) : width_(width), height_(height) {}
    
    void drawImpl() {
        std::cout << "Drawing a rectangle " << width_ << "x" << height_ << std::endl;
    }
    
    double areaImpl() {
        return width_ * height_;
    }
    
    std::string getName() {
        return "Rectangle";
    }
    
    // Rectangle-specific methods
    double getWidth() const { return width_; }
    double getHeight() const { return height_; }
};

// Example 2: CRTP for enabling functionality
template<typename Derived>
class Comparable {
public:
    bool operator!=(const Derived& other) const {
        return !static_cast<const Derived*>(this)->operator==(other);
    }
    
    bool operator<=(const Derived& other) const {
        const Derived& self = static_cast<const Derived&>(*this);
        return self < other || self == other;
    }
    
    bool operator>(const Derived& other) const {
        return !static_cast<const Derived*>(this)->operator<=(other);
    }
    
    bool operator>=(const Derived& other) const {
        return !static_cast<const Derived*>(this)->operator<(other);
    }
};

class Number : public Comparable<Number> {
private:
    int value_;

public:
    explicit Number(int value) : value_(value) {}
    
    bool operator==(const Number& other) const {
        return value_ == other.value_;
    }
    
    bool operator<(const Number& other) const {
        return value_ < other.value_;
    }
    
    int getValue() const { return value_; }
    
    friend std::ostream& operator<<(std::ostream& os, const Number& num) {
        return os << num.value_;
    }
};

// Example 3: CRTP for Singleton Pattern
template<typename Derived>
class Singleton {
protected:
    Singleton() = default;
    ~Singleton() = default;

public:
    // Delete copy constructor and assignment operator
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    
    static Derived& getInstance() {
        static Derived instance;
        return instance;
    }
};

class Logger : public Singleton<Logger> {
    friend class Singleton<Logger>;
    
private:
    Logger() {
        std::cout << "Logger instance created" << std::endl;
    }

public:
    void log(const std::string& message) {
        std::cout << "[LOG]: " << message << std::endl;
    }
};

class ConfigManager : public Singleton<ConfigManager> {
    friend class Singleton<ConfigManager>;
    
private:
    std::string configPath_;
    
    ConfigManager() : configPath_("default.config") {
        std::cout << "ConfigManager instance created" << std::endl;
    }

public:
    void setConfigPath(const std::string& path) {
        configPath_ = path;
    }
    
    const std::string& getConfigPath() const {
        return configPath_;
    }
};

// Example 4: CRTP for Method Chaining (Fluent Interface)
template<typename Derived>
class FluentBuilder {
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
    int id_ = 0;
};

class PersonBuilder : public FluentBuilder<PersonBuilder> {
private:
    int age_ = 0;
    std::string email_;

public:
    PersonBuilder& age(int age) {
        age_ = age;
        return *this;
    }
    
    PersonBuilder& email(const std::string& email) {
        email_ = email;
        return *this;
    }
    
    void build() {
        std::cout << "Person created: " << name_ << " (ID: " << id_ 
                  << ", Age: " << age_ << ", Email: " << email_ << ")" << std::endl;
    }
};

class ProductBuilder : public FluentBuilder<ProductBuilder> {
private:
    double price_ = 0.0;
    std::string category_;

public:
    ProductBuilder& price(double price) {
        price_ = price;
        return *this;
    }
    
    ProductBuilder& category(const std::string& category) {
        category_ = category;
        return *this;
    }
    
    void build() {
        std::cout << "Product created: " << name_ << " (ID: " << id_ 
                  << ", Price: $" << price_ << ", Category: " << category_ << ")" << std::endl;
    }
};

// Example 5: CRTP for Counting Instances
template<typename Derived>
class InstanceCounter {
private:
    static size_t count_;

protected:
    InstanceCounter() {
        ++count_;
    }
    
    InstanceCounter(const InstanceCounter&) {
        ++count_;
    }
    
    ~InstanceCounter() {
        --count_;
    }

public:
    static size_t getInstanceCount() {
        return count_;
    }
};

// Static member definition
template<typename Derived>
size_t InstanceCounter<Derived>::count_ = 0;

class Widget : public InstanceCounter<Widget> {
private:
    std::string name_;

public:
    explicit Widget(const std::string& name) : name_(name) {
        std::cout << "Widget '" << name_ << "' created. Total widgets: " 
                  << getInstanceCount() << std::endl;
    }
    
    ~Widget() {
        std::cout << "Widget '" << name_ << "' destroyed. Remaining widgets: " 
                  << getInstanceCount() - 1 << std::endl;
    }
    
    const std::string& getName() const { return name_; }
};

class Button : public InstanceCounter<Button> {
private:
    std::string label_;

public:
    explicit Button(const std::string& label) : label_(label) {
        std::cout << "Button '" << label_ << "' created. Total buttons: " 
                  << getInstanceCount() << std::endl;
    }
    
    ~Button() {
        std::cout << "Button '" << label_ << "' destroyed. Remaining buttons: " 
                  << getInstanceCount() - 1 << std::endl;
    }
    
    const std::string& getLabel() const { return label_; }
};

// Example 6: CRTP for Mixin Pattern
template<typename Derived>
class Printable {
public:
    void print() const {
        std::cout << static_cast<const Derived*>(this)->toString() << std::endl;
    }
};

template<typename Derived>
class Serializable {
public:
    std::string serialize() const {
        return static_cast<const Derived*>(this)->toJson();
    }
};

class Person : public Printable<Person>, public Serializable<Person> {
private:
    std::string name_;
    int age_;

public:
    Person(const std::string& name, int age) : name_(name), age_(age) {}
    
    std::string toString() const {
        return "Person{name: " + name_ + ", age: " + std::to_string(age_) + "}";
    }
    
    std::string toJson() const {
        return R"({"name": ")" + name_ + R"(", "age": )" + std::to_string(age_) + "}";
    }
    
    const std::string& getName() const { return name_; }
    int getAge() const { return age_; }
};

// Example 7: CRTP for Template Specialization
template<typename Derived>
class Drawable {
public:
    void render() {
        std::cout << "Rendering: ";
        static_cast<Derived*>(this)->draw();
    }
};

// Template specialization using CRTP
template<>
class Shape<Circle> {
public:
    void draw() {
        static_cast<Circle*>(this)->drawImpl();
    }
    
    double area() {
        return static_cast<Circle*>(this)->areaImpl();
    }
    
    // Specialized behavior for circles
    void drawWithBorder() {
        std::cout << "Drawing circle with decorative border" << std::endl;
        static_cast<Circle*>(this)->drawImpl();
    }
};

// Example 8: CRTP for Expression Templates (Advanced)
template<typename Derived>
class Expression {
public:
    const Derived& derived() const {
        return static_cast<const Derived&>(*this);
    }
    
    Derived& derived() {
        return static_cast<Derived&>(*this);
    }
};

class Vector : public Expression<Vector> {
private:
    std::vector<double> data_;

public:
    explicit Vector(std::vector<double> data) : data_(std::move(data)) {}
    
    double operator[](size_t i) const { return data_[i]; }
    double& operator[](size_t i) { return data_[i]; }
    
    size_t size() const { return data_.size(); }
    
    void print() const {
        std::cout << "[";
        for (size_t i = 0; i < data_.size(); ++i) {
            std::cout << data_[i];
            if (i < data_.size() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
};

template<typename LHS, typename RHS>
class VectorAdd : public Expression<VectorAdd<LHS, RHS>> {
private:
    const LHS& lhs_;
    const RHS& rhs_;

public:
    VectorAdd(const LHS& lhs, const RHS& rhs) : lhs_(lhs), rhs_(rhs) {}
    
    double operator[](size_t i) const {
        return lhs_[i] + rhs_[i];
    }
    
    size_t size() const {
        return lhs_.size();
    }
};

template<typename LHS, typename RHS>
VectorAdd<LHS, RHS> operator+(const Expression<LHS>& lhs, const Expression<RHS>& rhs) {
    return VectorAdd<LHS, RHS>(lhs.derived(), rhs.derived());
}

void demonstrateBasicCRTP() {
    std::cout << "=== Basic CRTP - Static Polymorphism ===\n\n";
    
    Circle circle(5.0);
    Rectangle rectangle(4.0, 6.0);
    
    // Static polymorphism - no virtual function calls
    circle.draw();
    circle.print();
    std::cout << std::endl;
    
    rectangle.draw();
    rectangle.print();
    std::cout << std::endl;
    
    // Access derived-specific methods
    std::cout << "Circle radius: " << circle.getRadius() << std::endl;
    std::cout << "Rectangle dimensions: " << rectangle.getWidth() 
              << "x" << rectangle.getHeight() << std::endl;
    std::cout << std::endl;
}

void demonstrateComparable() {
    std::cout << "=== CRTP for Operator Generation ===\n\n";
    
    Number num1(10);
    Number num2(20);
    Number num3(10);
    
    std::cout << "num1 = " << num1 << ", num2 = " << num2 << ", num3 = " << num3 << std::endl;
    
    std::cout << "num1 == num3: " << (num1 == num3) << std::endl;
    std::cout << "num1 != num2: " << (num1 != num2) << std::endl;
    std::cout << "num1 < num2: " << (num1 < num2) << std::endl;
    std::cout << "num1 <= num3: " << (num1 <= num3) << std::endl;
    std::cout << "num2 > num1: " << (num2 > num1) << std::endl;
    std::cout << "num1 >= num3: " << (num1 >= num3) << std::endl;
    std::cout << std::endl;
}

void demonstrateSingleton() {
    std::cout << "=== CRTP Singleton Pattern ===\n\n";
    
    Logger& logger1 = Logger::getInstance();
    Logger& logger2 = Logger::getInstance();
    
    std::cout << "Logger instances same? " << (&logger1 == &logger2 ? "Yes" : "No") << std::endl;
    
    logger1.log("First message");
    logger2.log("Second message");
    
    ConfigManager& config1 = ConfigManager::getInstance();
    ConfigManager& config2 = ConfigManager::getInstance();
    
    std::cout << "ConfigManager instances same? " << (&config1 == &config2 ? "Yes" : "No") << std::endl;
    
    config1.setConfigPath("/etc/myapp.config");
    std::cout << "Config path from config2: " << config2.getConfigPath() << std::endl;
    std::cout << std::endl;
}

void demonstrateFluentInterface() {
    std::cout << "=== CRTP Fluent Interface ===\n\n";
    
    PersonBuilder personBuilder;
    personBuilder.name("Alice")
                 .id(1001)
                 .age(25)
                 .email("alice@example.com")
                 .build();
    
    ProductBuilder productBuilder;
    productBuilder.name("Laptop")
                  .id(2001)
                  .price(999.99)
                  .category("Electronics")
                  .build();
    std::cout << std::endl;
}

void demonstrateInstanceCounting() {
    std::cout << "=== CRTP Instance Counting ===\n\n";
    
    std::cout << "Initial widget count: " << Widget::getInstanceCount() << std::endl;
    std::cout << "Initial button count: " << Button::getInstanceCount() << std::endl;
    
    {
        Widget widget1("MainWindow");
        Widget widget2("Dialog");
        Button button1("OK");
        Button button2("Cancel");
        Button button3("Help");
        
        std::cout << "\nCurrent widget count: " << Widget::getInstanceCount() << std::endl;
        std::cout << "Current button count: " << Button::getInstanceCount() << std::endl;
    }
    
    std::cout << "\nFinal widget count: " << Widget::getInstanceCount() << std::endl;
    std::cout << "Final button count: " << Button::getInstanceCount() << std::endl;
    std::cout << std::endl;
}

void demonstrateMixins() {
    std::cout << "=== CRTP Mixin Pattern ===\n\n";
    
    Person person("Bob", 30);
    
    std::cout << "Printing person:" << std::endl;
    person.print();
    
    std::cout << "Serializing person:" << std::endl;
    std::cout << person.serialize() << std::endl;
    std::cout << std::endl;
}

void demonstrateExpressionTemplates() {
    std::cout << "=== CRTP Expression Templates ===\n\n";
    
    Vector v1({1.0, 2.0, 3.0});
    Vector v2({4.0, 5.0, 6.0});
    Vector v3({7.0, 8.0, 9.0});
    
    std::cout << "v1: ";
    v1.print();
    std::cout << "v2: ";
    v2.print();
    std::cout << "v3: ";
    v3.print();
    
    // Expression templates allow efficient chaining without temporary objects
    auto result = v1 + v2 + v3;
    
    std::cout << "v1 + v2 + v3: ";
    std::cout << "[";
    for (size_t i = 0; i < result.size(); ++i) {
        std::cout << result[i];
        if (i < result.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    std::cout << std::endl;
}

void demonstrateCRTPBenefits() {
    std::cout << "=== CRTP Benefits Demonstration ===\n\n";
    
    // Performance comparison example
    auto start = std::chrono::high_resolution_clock::now();
    
    Circle circle(1.0);
    for (int i = 0; i < 1000000; ++i) {
        circle.area();  // Static dispatch - no virtual function overhead
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "CRTP static dispatch (1M calls): " << duration.count() << " microseconds" << std::endl;
    
    std::cout << "\nCRTP provides:" << std::endl;
    std::cout << "✓ Static polymorphism (no virtual function overhead)" << std::endl;
    std::cout << "✓ Compile-time method resolution" << std::endl;
    std::cout << "✓ Type safety at compile time" << std::endl;
    std::cout << "✓ Code reuse without runtime cost" << std::endl;
    std::cout << "✓ Template specialization opportunities" << std::endl;
}

void demonstrateCRTP() {
    std::cout << "=== CRTP (Curiously Recurring Template Pattern) Demonstration ===\n\n";
    
    demonstrateBasicCRTP();
    demonstrateComparable();
    demonstrateSingleton();
    demonstrateFluentInterface();
    demonstrateInstanceCounting();
    demonstrateMixins();
    demonstrateExpressionTemplates();
    demonstrateCRTPBenefits();
}

int main() {
    demonstrateCRTP();
    return 0;
}
