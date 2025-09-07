#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <optional>

// Product to be built
class Computer {
private:
    std::string cpu_;
    std::string gpu_;
    std::string ram_;
    std::string storage_;
    std::string motherboard_;
    std::string powerSupply_;
    std::string coolingSystem_;
    std::vector<std::string> peripherals_;
    bool isGamingOptimized_ = false;
    bool hasRGBLighting_ = false;

public:
    // Setters
    void setCPU(const std::string& cpu) { cpu_ = cpu; }
    void setGPU(const std::string& gpu) { gpu_ = gpu; }
    void setRAM(const std::string& ram) { ram_ = ram; }
    void setStorage(const std::string& storage) { storage_ = storage; }
    void setMotherboard(const std::string& mb) { motherboard_ = mb; }
    void setPowerSupply(const std::string& psu) { powerSupply_ = psu; }
    void setCoolingSystem(const std::string& cooling) { coolingSystem_ = cooling; }
    void addPeripheral(const std::string& peripheral) { peripherals_.push_back(peripheral); }
    void setGamingOptimized(bool optimized) { isGamingOptimized_ = optimized; }
    void setRGBLighting(bool rgb) { hasRGBLighting_ = rgb; }
    
    // Display computer specs
    void displaySpecs() const {
        std::cout << "=== Computer Specifications ===" << std::endl;
        std::cout << "CPU: " << cpu_ << std::endl;
        std::cout << "GPU: " << gpu_ << std::endl;
        std::cout << "RAM: " << ram_ << std::endl;
        std::cout << "Storage: " << storage_ << std::endl;
        std::cout << "Motherboard: " << motherboard_ << std::endl;
        std::cout << "Power Supply: " << powerSupply_ << std::endl;
        std::cout << "Cooling: " << coolingSystem_ << std::endl;
        
        if (!peripherals_.empty()) {
            std::cout << "Peripherals: ";
            for (size_t i = 0; i < peripherals_.size(); ++i) {
                std::cout << peripherals_[i];
                if (i < peripherals_.size() - 1) std::cout << ", ";
            }
            std::cout << std::endl;
        }
        
        std::cout << "Gaming Optimized: " << (isGamingOptimized_ ? "Yes" : "No") << std::endl;
        std::cout << "RGB Lighting: " << (hasRGBLighting_ ? "Yes" : "No") << std::endl;
        std::cout << "==============================" << std::endl;
    }
};

// Abstract Builder Interface
class ComputerBuilder {
protected:
    std::unique_ptr<Computer> computer_;

public:
    ComputerBuilder() : computer_(std::make_unique<Computer>()) {}
    virtual ~ComputerBuilder() = default;
    
    // Pure virtual methods for building components
    virtual ComputerBuilder& buildCPU() = 0;
    virtual ComputerBuilder& buildGPU() = 0;
    virtual ComputerBuilder& buildRAM() = 0;
    virtual ComputerBuilder& buildStorage() = 0;
    virtual ComputerBuilder& buildMotherboard() = 0;
    virtual ComputerBuilder& buildPowerSupply() = 0;
    virtual ComputerBuilder& buildCoolingSystem() = 0;
    virtual ComputerBuilder& buildPeripherals() = 0;
    
    // Method to get the final product
    std::unique_ptr<Computer> getResult() {
        return std::move(computer_);
    }
    
    // Reset for building a new computer
    void reset() {
        computer_ = std::make_unique<Computer>();
    }
};

// Concrete Builder for Gaming Computer
class GamingComputerBuilder : public ComputerBuilder {
public:
    ComputerBuilder& buildCPU() override {
        computer_->setCPU("Intel Core i9-13900K");
        return *this;
    }
    
    ComputerBuilder& buildGPU() override {
        computer_->setGPU("NVIDIA RTX 4090");
        return *this;
    }
    
    ComputerBuilder& buildRAM() override {
        computer_->setRAM("32GB DDR5-6000");
        return *this;
    }
    
    ComputerBuilder& buildStorage() override {
        computer_->setStorage("2TB NVMe SSD + 4TB HDD");
        return *this;
    }
    
    ComputerBuilder& buildMotherboard() override {
        computer_->setMotherboard("ASUS ROG Maximus Z790 Hero");
        return *this;
    }
    
    ComputerBuilder& buildPowerSupply() override {
        computer_->setPowerSupply("1000W 80+ Gold Modular PSU");
        return *this;
    }
    
    ComputerBuilder& buildCoolingSystem() override {
        computer_->setCoolingSystem("AIO Liquid Cooler 360mm");
        return *this;
    }
    
    ComputerBuilder& buildPeripherals() override {
        computer_->addPeripheral("Gaming Keyboard");
        computer_->addPeripheral("Gaming Mouse");
        computer_->addPeripheral("Gaming Headset");
        computer_->addPeripheral("Webcam");
        computer_->setGamingOptimized(true);
        computer_->setRGBLighting(true);
        return *this;
    }
};

// Concrete Builder for Office Computer
class OfficeComputerBuilder : public ComputerBuilder {
public:
    ComputerBuilder& buildCPU() override {
        computer_->setCPU("Intel Core i5-13600");
        return *this;
    }
    
    ComputerBuilder& buildGPU() override {
        computer_->setGPU("Integrated Graphics");
        return *this;
    }
    
    ComputerBuilder& buildRAM() override {
        computer_->setRAM("16GB DDR4-3200");
        return *this;
    }
    
    ComputerBuilder& buildStorage() override {
        computer_->setStorage("1TB NVMe SSD");
        return *this;
    }
    
    ComputerBuilder& buildMotherboard() override {
        computer_->setMotherboard("MSI Pro B660M-A");
        return *this;
    }
    
    ComputerBuilder& buildPowerSupply() override {
        computer_->setPowerSupply("500W 80+ Bronze PSU");
        return *this;
    }
    
    ComputerBuilder& buildCoolingSystem() override {
        computer_->setCoolingSystem("Stock CPU Cooler");
        return *this;
    }
    
    ComputerBuilder& buildPeripherals() override {
        computer_->addPeripheral("Wireless Keyboard");
        computer_->addPeripheral("Wireless Mouse");
        computer_->addPeripheral("Webcam");
        computer_->setGamingOptimized(false);
        computer_->setRGBLighting(false);
        return *this;
    }
};

// Concrete Builder for Workstation Computer
class WorkstationComputerBuilder : public ComputerBuilder {
public:
    ComputerBuilder& buildCPU() override {
        computer_->setCPU("Intel Xeon W-3375");
        return *this;
    }
    
    ComputerBuilder& buildGPU() override {
        computer_->setGPU("NVIDIA RTX A6000");
        return *this;
    }
    
    ComputerBuilder& buildRAM() override {
        computer_->setRAM("128GB DDR4-3200 ECC");
        return *this;
    }
    
    ComputerBuilder& buildStorage() override {
        computer_->setStorage("4TB NVMe SSD + 16TB HDD RAID");
        return *this;
    }
    
    ComputerBuilder& buildMotherboard() override {
        computer_->setMotherboard("ASUS Pro WS C621-64L SAGE/10G");
        return *this;
    }
    
    ComputerBuilder& buildPowerSupply() override {
        computer_->setPowerSupply("1600W 80+ Platinum PSU");
        return *this;
    }
    
    ComputerBuilder& buildCoolingSystem() override {
        computer_->setCoolingSystem("Custom Loop Liquid Cooling");
        return *this;
    }
    
    ComputerBuilder& buildPeripherals() override {
        computer_->addPeripheral("Professional Keyboard");
        computer_->addPeripheral("CAD Mouse");
        computer_->addPeripheral("Professional Headset");
        computer_->addPeripheral("Graphics Tablet");
        computer_->setGamingOptimized(false);
        computer_->setRGBLighting(false);
        return *this;
    }
};

// Director class that orchestrates the building process
class ComputerDirector {
private:
    ComputerBuilder* builder_;

public:
    void setBuilder(ComputerBuilder* builder) {
        builder_ = builder;
    }
    
    // Build a complete computer
    std::unique_ptr<Computer> buildFullComputer() {
        if (!builder_) {
            throw std::runtime_error("Builder not set");
        }
        
        builder_->reset();
        return builder_->buildCPU()
                     .buildGPU()
                     .buildRAM()
                     .buildStorage()
                     .buildMotherboard()
                     .buildPowerSupply()
                     .buildCoolingSystem()
                     .buildPeripherals()
                     .getResult();
    }
    
    // Build a basic computer (minimal configuration)
    std::unique_ptr<Computer> buildBasicComputer() {
        if (!builder_) {
            throw std::runtime_error("Builder not set");
        }
        
        builder_->reset();
        return builder_->buildCPU()
                     .buildRAM()
                     .buildStorage()
                     .buildMotherboard()
                     .buildPowerSupply()
                     .getResult();
    }
};

// Modern C++ Fluent Builder (without Director)
class FluentComputerBuilder {
private:
    std::unique_ptr<Computer> computer_;

public:
    FluentComputerBuilder() : computer_(std::make_unique<Computer>()) {}
    
    FluentComputerBuilder& withCPU(const std::string& cpu) {
        computer_->setCPU(cpu);
        return *this;
    }
    
    FluentComputerBuilder& withGPU(const std::string& gpu) {
        computer_->setGPU(gpu);
        return *this;
    }
    
    FluentComputerBuilder& withRAM(const std::string& ram) {
        computer_->setRAM(ram);
        return *this;
    }
    
    FluentComputerBuilder& withStorage(const std::string& storage) {
        computer_->setStorage(storage);
        return *this;
    }
    
    FluentComputerBuilder& withMotherboard(const std::string& mb) {
        computer_->setMotherboard(mb);
        return *this;
    }
    
    FluentComputerBuilder& withPowerSupply(const std::string& psu) {
        computer_->setPowerSupply(psu);
        return *this;
    }
    
    FluentComputerBuilder& withCooling(const std::string& cooling) {
        computer_->setCoolingSystem(cooling);
        return *this;
    }
    
    FluentComputerBuilder& withPeripheral(const std::string& peripheral) {
        computer_->addPeripheral(peripheral);
        return *this;
    }
    
    FluentComputerBuilder& enableGamingOptimization() {
        computer_->setGamingOptimized(true);
        return *this;
    }
    
    FluentComputerBuilder& enableRGBLighting() {
        computer_->setRGBLighting(true);
        return *this;
    }
    
    std::unique_ptr<Computer> build() {
        return std::move(computer_);
    }
};

void demonstrateBuilderPattern() {
    std::cout << "=== Builder Pattern Demonstration ===\n\n";
    
    // 1. Traditional Builder Pattern with Director
    std::cout << "1. Traditional Builder Pattern with Director:\n\n";
    
    ComputerDirector director;
    
    // Build Gaming Computer
    GamingComputerBuilder gamingBuilder;
    director.setBuilder(&gamingBuilder);
    
    std::cout << "Building Gaming Computer:\n";
    auto gamingComputer = director.buildFullComputer();
    gamingComputer->displaySpecs();
    std::cout << std::endl;
    
    // Build Office Computer
    OfficeComputerBuilder officeBuilder;
    director.setBuilder(&officeBuilder);
    
    std::cout << "Building Office Computer:\n";
    auto officeComputer = director.buildFullComputer();
    officeComputer->displaySpecs();
    std::cout << std::endl;
    
    // Build Workstation Computer
    WorkstationComputerBuilder workstationBuilder;
    director.setBuilder(&workstationBuilder);
    
    std::cout << "Building Workstation Computer:\n";
    auto workstationComputer = director.buildFullComputer();
    workstationComputer->displaySpecs();
    std::cout << std::endl;
    
    // Build Basic Computer (partial build)
    std::cout << "Building Basic Gaming Computer (partial):\n";
    director.setBuilder(&gamingBuilder);
    auto basicGamingComputer = director.buildBasicComputer();
    basicGamingComputer->displaySpecs();
    std::cout << std::endl;
    
    // 2. Fluent Builder Pattern
    std::cout << "2. Fluent Builder Pattern:\n\n";
    
    std::cout << "Building Custom Computer with Fluent Interface:\n";
    auto customComputer = FluentComputerBuilder()
        .withCPU("AMD Ryzen 9 7950X")
        .withGPU("AMD RX 7900 XTX")
        .withRAM("64GB DDR5-5600")
        .withStorage("8TB NVMe SSD")
        .withMotherboard("MSI MEG X670E ACE")
        .withPowerSupply("1200W 80+ Titanium PSU")
        .withCooling("Custom Loop with 480mm Radiator")
        .withPeripheral("Mechanical Keyboard")
        .withPeripheral("Gaming Mouse")
        .withPeripheral("Ultrawide Monitor")
        .enableGamingOptimization()
        .enableRGBLighting()
        .build();
    
    customComputer->displaySpecs();
    std::cout << std::endl;
    
    // Build minimal computer with fluent builder
    std::cout << "Building Minimal Computer with Fluent Interface:\n";
    auto minimalComputer = FluentComputerBuilder()
        .withCPU("Intel Core i3-12100")
        .withRAM("8GB DDR4-2666")
        .withStorage("256GB SSD")
        .build();
    
    minimalComputer->displaySpecs();
    
    // 3. Demonstrate builder reuse
    std::cout << "\n3. Builder Reuse:\n\n";
    
    std::cout << "Building multiple gaming computers:\n";
    for (int i = 1; i <= 3; ++i) {
        std::cout << "Gaming Computer #" << i << ":\n";
        auto computer = director.buildFullComputer();
        std::cout << "CPU: " << (computer ? "Built successfully" : "Failed") << std::endl;
        std::cout << std::endl;
    }
}

int main() {
    demonstrateBuilderPattern();
    return 0;
}
