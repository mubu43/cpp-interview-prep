#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Abstract Product
class Animal {
public:
    virtual ~Animal() = default;
    virtual void makeSound() const = 0;
    virtual std::string getType() const = 0;
};

// Concrete Products - Dogs
class Labrador : public Animal {
public:
    void makeSound() const override {
        std::cout << "Labrador: Woof! Woof! (friendly bark)" << std::endl;
    }
    
    std::string getType() const override {
        return "Labrador";
    }
};

class Bulldog : public Animal {
public:
    void makeSound() const override {
        std::cout << "Bulldog: Wroof! Wroof! (deep bark)" << std::endl;
    }
    
    std::string getType() const override {
        return "Bulldog";
    }
};

// Concrete Products - Cats
class Persian : public Animal {
public:
    void makeSound() const override {
        std::cout << "Persian: Meow~ (elegant purr)" << std::endl;
    }
    
    std::string getType() const override {
        return "Persian";
    }
};

class Siamese : public Animal {
public:
    void makeSound() const override {
        std::cout << "Siamese: Miaow! (loud meow)" << std::endl;
    }
    
    std::string getType() const override {
        return "Siamese";
    }
};

// Factory Method Pattern - Abstract Creator
class AnimalFactory {
public:
    virtual ~AnimalFactory() = default;
    
    // Factory method - subclasses will override this
    virtual std::unique_ptr<Animal> createAnimal(const std::string& breed) const = 0;
    
    // Template method using factory method
    void adoptAnimal(const std::string& breed) const {
        auto animal = createAnimal(breed);
        if (animal) {
            std::cout << "Adopting a " << animal->getType() << std::endl;
            animal->makeSound();
            std::cout << "Welcome to your new home!" << std::endl;
        } else {
            std::cout << "Sorry, " << breed << " is not available." << std::endl;
        }
    }
};

// Concrete Creator - Dog Factory
class DogFactory : public AnimalFactory {
public:
    std::unique_ptr<Animal> createAnimal(const std::string& breed) const override {
        if (breed == "Labrador") {
            return std::make_unique<Labrador>();
        } else if (breed == "Bulldog") {
            return std::make_unique<Bulldog>();
        }
        return nullptr;
    }
};

// Concrete Creator - Cat Factory
class CatFactory : public AnimalFactory {
public:
    std::unique_ptr<Animal> createAnimal(const std::string& breed) const override {
        if (breed == "Persian") {
            return std::make_unique<Persian>();
        } else if (breed == "Siamese") {
            return std::make_unique<Siamese>();
        }
        return nullptr;
    }
};

// Advanced: Factory with registration system
class AdvancedAnimalFactory {
private:
    using CreatorFunction = std::function<std::unique_ptr<Animal>()>;
    std::map<std::string, CreatorFunction> creators_;

public:
    // Register a creator function for a specific type
    void registerCreator(const std::string& type, CreatorFunction creator) {
        creators_[type] = creator;
    }
    
    // Create animal using registered creators
    std::unique_ptr<Animal> createAnimal(const std::string& type) const {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second();
        }
        return nullptr;
    }
    
    // Get list of available types
    std::vector<std::string> getAvailableTypes() const {
        std::vector<std::string> types;
        for (const auto& pair : creators_) {
            types.push_back(pair.first);
        }
        return types;
    }
};

// Modern C++ approach with template factory
template<typename BaseType>
class TemplateFactory {
private:
    using CreatorFunction = std::function<std::unique_ptr<BaseType>()>;
    std::map<std::string, CreatorFunction> creators_;

public:
    template<typename ConcreteType>
    void registerType(const std::string& name) {
        creators_[name] = []() {
            return std::make_unique<ConcreteType>();
        };
    }
    
    std::unique_ptr<BaseType> create(const std::string& name) const {
        auto it = creators_.find(name);
        return (it != creators_.end()) ? it->second() : nullptr;
    }
    
    std::vector<std::string> getRegisteredTypes() const {
        std::vector<std::string> types;
        for (const auto& pair : creators_) {
            types.push_back(pair.first);
        }
        return types;
    }
};

// Simple Factory (not Factory Method, but commonly used)
class SimpleAnimalFactory {
public:
    enum class AnimalType { LABRADOR, BULLDOG, PERSIAN, SIAMESE };
    
    static std::unique_ptr<Animal> createAnimal(AnimalType type) {
        switch (type) {
            case AnimalType::LABRADOR:
                return std::make_unique<Labrador>();
            case AnimalType::BULLDOG:
                return std::make_unique<Bulldog>();
            case AnimalType::PERSIAN:
                return std::make_unique<Persian>();
            case AnimalType::SIAMESE:
                return std::make_unique<Siamese>();
            default:
                return nullptr;
        }
    }
};

void demonstrateFactoryMethod() {
    std::cout << "=== Factory Method Pattern Demonstration ===\n\n";
    
    // 1. Basic Factory Method Pattern
    std::cout << "1. Basic Factory Method Pattern:\n";
    std::unique_ptr<AnimalFactory> dogFactory = std::make_unique<DogFactory>();
    std::unique_ptr<AnimalFactory> catFactory = std::make_unique<CatFactory>();
    
    dogFactory->adoptAnimal("Labrador");
    dogFactory->adoptAnimal("Bulldog");
    dogFactory->adoptAnimal("Persian");  // Not available in dog factory
    
    std::cout << std::endl;
    
    catFactory->adoptAnimal("Persian");
    catFactory->adoptAnimal("Siamese");
    catFactory->adoptAnimal("Labrador");  // Not available in cat factory
    
    std::cout << "\n2. Advanced Factory with Registration:\n";
    AdvancedAnimalFactory advancedFactory;
    
    // Register creators
    advancedFactory.registerCreator("Labrador", []() {
        return std::make_unique<Labrador>();
    });
    advancedFactory.registerCreator("Persian", []() {
        return std::make_unique<Persian>();
    });
    advancedFactory.registerCreator("Bulldog", []() {
        return std::make_unique<Bulldog>();
    });
    
    std::cout << "Available types: ";
    for (const auto& type : advancedFactory.getAvailableTypes()) {
        std::cout << type << " ";
    }
    std::cout << std::endl;
    
    auto labrador = advancedFactory.createAnimal("Labrador");
    if (labrador) {
        std::cout << "Created: " << labrador->getType() << " - ";
        labrador->makeSound();
    }
    
    std::cout << "\n3. Template Factory:\n";
    TemplateFactory<Animal> templateFactory;
    templateFactory.registerType<Labrador>("Labrador");
    templateFactory.registerType<Persian>("Persian");
    templateFactory.registerType<Siamese>("Siamese");
    
    std::cout << "Template factory registered types: ";
    for (const auto& type : templateFactory.getRegisteredTypes()) {
        std::cout << type << " ";
    }
    std::cout << std::endl;
    
    auto siamese = templateFactory.create("Siamese");
    if (siamese) {
        std::cout << "Template factory created: " << siamese->getType() << " - ";
        siamese->makeSound();
    }
    
    std::cout << "\n4. Simple Factory:\n";
    auto bulldog = SimpleAnimalFactory::createAnimal(SimpleAnimalFactory::AnimalType::BULLDOG);
    if (bulldog) {
        std::cout << "Simple factory created: " << bulldog->getType() << " - ";
        bulldog->makeSound();
    }
    
    std::cout << "\n5. Polymorphic usage:\n";
    std::vector<std::unique_ptr<AnimalFactory>> factories;
    factories.push_back(std::make_unique<DogFactory>());
    factories.push_back(std::make_unique<CatFactory>());
    
    std::vector<std::string> breeds = {"Labrador", "Persian", "Bulldog", "Siamese"};
    
    for (const auto& factory : factories) {
        for (const auto& breed : breeds) {
            auto animal = factory->createAnimal(breed);
            if (animal) {
                std::cout << "Factory created " << animal->getType() << ": ";
                animal->makeSound();
            }
        }
    }
}

int main() {
    demonstrateFactoryMethod();
    return 0;
}
