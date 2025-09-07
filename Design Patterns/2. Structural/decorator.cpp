#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Base Component interface
class Coffee {
public:
    virtual ~Coffee() = default;
    virtual std::string getDescription() const = 0;
    virtual double getCost() const = 0;
};

// Concrete Component - Basic coffee
class SimpleCoffee : public Coffee {
public:
    std::string getDescription() const override {
        return "Simple Coffee";
    }
    
    double getCost() const override {
        return 2.0;
    }
};

// Base Decorator - maintains reference to component
class CoffeeDecorator : public Coffee {
protected:
    std::unique_ptr<Coffee> coffee_;

public:
    CoffeeDecorator(std::unique_ptr<Coffee> coffee) 
        : coffee_(std::move(coffee)) {}
    
    std::string getDescription() const override {
        return coffee_->getDescription();
    }
    
    double getCost() const override {
        return coffee_->getCost();
    }
};

// Concrete Decorators
class MilkDecorator : public CoffeeDecorator {
public:
    MilkDecorator(std::unique_ptr<Coffee> coffee) 
        : CoffeeDecorator(std::move(coffee)) {}
    
    std::string getDescription() const override {
        return coffee_->getDescription() + ", Milk";
    }
    
    double getCost() const override {
        return coffee_->getCost() + 0.5;
    }
};

class SugarDecorator : public CoffeeDecorator {
public:
    SugarDecorator(std::unique_ptr<Coffee> coffee) 
        : CoffeeDecorator(std::move(coffee)) {}
    
    std::string getDescription() const override {
        return coffee_->getDescription() + ", Sugar";
    }
    
    double getCost() const override {
        return coffee_->getCost() + 0.2;
    }
};

class WhipDecorator : public CoffeeDecorator {
public:
    WhipDecorator(std::unique_ptr<Coffee> coffee) 
        : CoffeeDecorator(std::move(coffee)) {}
    
    std::string getDescription() const override {
        return coffee_->getDescription() + ", Whipped Cream";
    }
    
    double getCost() const override {
        return coffee_->getCost() + 0.7;
    }
};

class VanillaDecorator : public CoffeeDecorator {
public:
    VanillaDecorator(std::unique_ptr<Coffee> coffee) 
        : CoffeeDecorator(std::move(coffee)) {}
    
    std::string getDescription() const override {
        return coffee_->getDescription() + ", Vanilla";
    }
    
    double getCost() const override {
        return coffee_->getCost() + 0.6;
    }
};

// Alternative example: Text formatting
class Text {
public:
    virtual ~Text() = default;
    virtual std::string render() const = 0;
};

class PlainText : public Text {
private:
    std::string content_;

public:
    PlainText(const std::string& content) : content_(content) {}
    
    std::string render() const override {
        return content_;
    }
};

class TextDecorator : public Text {
protected:
    std::unique_ptr<Text> text_;

public:
    TextDecorator(std::unique_ptr<Text> text) : text_(std::move(text)) {}
    
    std::string render() const override {
        return text_->render();
    }
};

class BoldDecorator : public TextDecorator {
public:
    BoldDecorator(std::unique_ptr<Text> text) : TextDecorator(std::move(text)) {}
    
    std::string render() const override {
        return "<b>" + text_->render() + "</b>";
    }
};

class ItalicDecorator : public TextDecorator {
public:
    ItalicDecorator(std::unique_ptr<Text> text) : TextDecorator(std::move(text)) {}
    
    std::string render() const override {
        return "<i>" + text_->render() + "</i>";
    }
};

class UnderlineDecorator : public TextDecorator {
public:
    UnderlineDecorator(std::unique_ptr<Text> text) : TextDecorator(std::move(text)) {}
    
    std::string render() const override {
        return "<u>" + text_->render() + "</u>";
    }
};

// Function template to make decoration easier
template<typename DecoratorType, typename... Args>
std::unique_ptr<Coffee> addTopping(std::unique_ptr<Coffee> coffee, Args&&... args) {
    return std::make_unique<DecoratorType>(std::move(coffee), std::forward<Args>(args)...);
}

// Utility function to print coffee details
void printCoffee(const Coffee& coffee) {
    std::cout << "Coffee: " << coffee.getDescription() 
              << " | Cost: $" << coffee.getCost() << std::endl;
}

void demonstrateDecorator() {
    std::cout << "=== Decorator Pattern Demonstration ===\n\n";
    
    std::cout << "1. Coffee Shop Example:\n";
    
    // Basic coffee
    auto coffee1 = std::make_unique<SimpleCoffee>();
    printCoffee(*coffee1);
    
    // Coffee with milk
    auto coffee2 = std::make_unique<MilkDecorator>(std::make_unique<SimpleCoffee>());
    printCoffee(*coffee2);
    
    // Coffee with milk and sugar
    auto coffee3 = std::make_unique<SugarDecorator>(
        std::make_unique<MilkDecorator>(std::make_unique<SimpleCoffee>())
    );
    printCoffee(*coffee3);
    
    // Fully loaded coffee
    auto coffee4 = std::make_unique<VanillaDecorator>(
        std::make_unique<WhipDecorator>(
            std::make_unique<SugarDecorator>(
                std::make_unique<MilkDecorator>(std::make_unique<SimpleCoffee>())
            )
        )
    );
    printCoffee(*coffee4);
    
    std::cout << "\n2. Text Formatting Example:\n";
    
    // Plain text
    auto text1 = std::make_unique<PlainText>("Hello World");
    std::cout << "Plain: " << text1->render() << std::endl;
    
    // Bold text
    auto text2 = std::make_unique<BoldDecorator>(
        std::make_unique<PlainText>("Hello World")
    );
    std::cout << "Bold: " << text2->render() << std::endl;
    
    // Bold and italic
    auto text3 = std::make_unique<ItalicDecorator>(
        std::make_unique<BoldDecorator>(
            std::make_unique<PlainText>("Hello World")
        )
    );
    std::cout << "Bold + Italic: " << text3->render() << std::endl;
    
    // All formatting
    auto text4 = std::make_unique<UnderlineDecorator>(
        std::make_unique<ItalicDecorator>(
            std::make_unique<BoldDecorator>(
                std::make_unique<PlainText>("Hello World")
            )
        )
    );
    std::cout << "All formatting: " << text4->render() << std::endl;
    
    std::cout << "\n3. Dynamic Decoration:\n";
    
    // Building coffee dynamically
    std::unique_ptr<Coffee> dynamicCoffee = std::make_unique<SimpleCoffee>();
    std::vector<std::string> toppings = {"milk", "sugar", "whip"};
    
    for (const auto& topping : toppings) {
        if (topping == "milk") {
            dynamicCoffee = std::make_unique<MilkDecorator>(std::move(dynamicCoffee));
        } else if (topping == "sugar") {
            dynamicCoffee = std::make_unique<SugarDecorator>(std::move(dynamicCoffee));
        } else if (topping == "whip") {
            dynamicCoffee = std::make_unique<WhipDecorator>(std::move(dynamicCoffee));
        }
    }
    
    std::cout << "Dynamically built coffee:\n";
    printCoffee(*dynamicCoffee);
}

int main() {
    demonstrateDecorator();
    return 0;
}
