#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <random>
#include <chrono>

// Strategy Interface
class SortStrategy {
public:
    virtual ~SortStrategy() = default;
    virtual void sort(std::vector<int>& data) = 0;
    virtual std::string getName() const = 0;
};

// Concrete Strategies
class BubbleSort : public SortStrategy {
public:
    void sort(std::vector<int>& data) override {
        std::cout << "Performing Bubble Sort..." << std::endl;
        size_t n = data.size();
        for (size_t i = 0; i < n - 1; ++i) {
            for (size_t j = 0; j < n - i - 1; ++j) {
                if (data[j] > data[j + 1]) {
                    std::swap(data[j], data[j + 1]);
                }
            }
        }
    }
    
    std::string getName() const override { return "Bubble Sort"; }
};

class QuickSort : public SortStrategy {
private:
    void quickSort(std::vector<int>& data, int low, int high) {
        if (low < high) {
            int pi = partition(data, low, high);
            quickSort(data, low, pi - 1);
            quickSort(data, pi + 1, high);
        }
    }
    
    int partition(std::vector<int>& data, int low, int high) {
        int pivot = data[high];
        int i = low - 1;
        
        for (int j = low; j <= high - 1; j++) {
            if (data[j] < pivot) {
                i++;
                std::swap(data[i], data[j]);
            }
        }
        std::swap(data[i + 1], data[high]);
        return i + 1;
    }

public:
    void sort(std::vector<int>& data) override {
        std::cout << "Performing Quick Sort..." << std::endl;
        if (!data.empty()) {
            quickSort(data, 0, static_cast<int>(data.size()) - 1);
        }
    }
    
    std::string getName() const override { return "Quick Sort"; }
};

class MergeSort : public SortStrategy {
private:
    void mergeSort(std::vector<int>& data, int left, int right) {
        if (left < right) {
            int mid = left + (right - left) / 2;
            mergeSort(data, left, mid);
            mergeSort(data, mid + 1, right);
            merge(data, left, mid, right);
        }
    }
    
    void merge(std::vector<int>& data, int left, int mid, int right) {
        int n1 = mid - left + 1;
        int n2 = right - mid;
        
        std::vector<int> leftArr(n1), rightArr(n2);
        
        for (int i = 0; i < n1; i++) leftArr[i] = data[left + i];
        for (int j = 0; j < n2; j++) rightArr[j] = data[mid + 1 + j];
        
        int i = 0, j = 0, k = left;
        
        while (i < n1 && j < n2) {
            if (leftArr[i] <= rightArr[j]) {
                data[k] = leftArr[i];
                i++;
            } else {
                data[k] = rightArr[j];
                j++;
            }
            k++;
        }
        
        while (i < n1) {
            data[k] = leftArr[i];
            i++;
            k++;
        }
        
        while (j < n2) {
            data[k] = rightArr[j];
            j++;
            k++;
        }
    }

public:
    void sort(std::vector<int>& data) override {
        std::cout << "Performing Merge Sort..." << std::endl;
        if (!data.empty()) {
            mergeSort(data, 0, static_cast<int>(data.size()) - 1);
        }
    }
    
    std::string getName() const override { return "Merge Sort"; }
};

class StdSort : public SortStrategy {
public:
    void sort(std::vector<int>& data) override {
        std::cout << "Performing std::sort (typically Introsort)..." << std::endl;
        std::sort(data.begin(), data.end());
    }
    
    std::string getName() const override { return "std::sort"; }
};

// Context class
class Sorter {
private:
    std::unique_ptr<SortStrategy> strategy_;

public:
    explicit Sorter(std::unique_ptr<SortStrategy> strategy) 
        : strategy_(std::move(strategy)) {}
    
    void setStrategy(std::unique_ptr<SortStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    void performSort(std::vector<int>& data) {
        if (strategy_) {
            auto start = std::chrono::high_resolution_clock::now();
            strategy_->sort(data);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Sort completed in " << duration.count() << " microseconds" << std::endl;
        }
    }
    
    std::string getCurrentStrategy() const {
        return strategy_ ? strategy_->getName() : "No strategy set";
    }
};

// Function-based Strategy Pattern
using PaymentProcessor = std::function<bool(double amount, const std::string& details)>;

class PaymentContext {
private:
    PaymentProcessor processor_;

public:
    void setPaymentMethod(PaymentProcessor processor) {
        processor_ = processor;
    }
    
    bool processPayment(double amount, const std::string& details) {
        if (processor_) {
            return processor_(amount, details);
        }
        std::cout << "No payment processor set!" << std::endl;
        return false;
    }
};

// Payment strategies as functions
PaymentProcessor createCreditCardProcessor() {
    return [](double amount, const std::string& details) -> bool {
        std::cout << "Processing credit card payment of $" << amount 
                  << " with details: " << details << std::endl;
        std::cout << "Validating card number..." << std::endl;
        std::cout << "Contacting bank..." << std::endl;
        std::cout << "Payment approved!" << std::endl;
        return true;
    };
}

PaymentProcessor createPayPalProcessor() {
    return [](double amount, const std::string& details) -> bool {
        std::cout << "Processing PayPal payment of $" << amount 
                  << " with details: " << details << std::endl;
        std::cout << "Redirecting to PayPal..." << std::endl;
        std::cout << "PayPal authentication successful!" << std::endl;
        std::cout << "Payment completed!" << std::endl;
        return true;
    };
}

PaymentProcessor createBankTransferProcessor() {
    return [](double amount, const std::string& details) -> bool {
        std::cout << "Processing bank transfer of $" << amount 
                  << " with details: " << details << std::endl;
        std::cout << "Verifying account details..." << std::endl;
        std::cout << "Initiating transfer..." << std::endl;
        std::cout << "Transfer queued for processing!" << std::endl;
        return true;
    };
}

// Template-based Strategy Pattern
template<typename T>
class CompressionStrategy {
public:
    virtual ~CompressionStrategy() = default;
    virtual std::vector<T> compress(const std::vector<T>& data) = 0;
    virtual std::vector<T> decompress(const std::vector<T>& data) = 0;
    virtual std::string getName() const = 0;
};

template<typename T>
class NoCompression : public CompressionStrategy<T> {
public:
    std::vector<T> compress(const std::vector<T>& data) override {
        std::cout << "No compression applied" << std::endl;
        return data;
    }
    
    std::vector<T> decompress(const std::vector<T>& data) override {
        std::cout << "No decompression needed" << std::endl;
        return data;
    }
    
    std::string getName() const override { return "No Compression"; }
};

template<typename T>
class SimpleCompression : public CompressionStrategy<T> {
public:
    std::vector<T> compress(const std::vector<T>& data) override {
        std::cout << "Applying simple compression (removing duplicates)" << std::endl;
        std::vector<T> compressed;
        for (const auto& item : data) {
            if (std::find(compressed.begin(), compressed.end(), item) == compressed.end()) {
                compressed.push_back(item);
            }
        }
        return compressed;
    }
    
    std::vector<T> decompress(const std::vector<T>& data) override {
        std::cout << "Simple decompression (data as-is)" << std::endl;
        return data;
    }
    
    std::string getName() const override { return "Simple Compression"; }
};

template<typename T>
class DataProcessor {
private:
    std::unique_ptr<CompressionStrategy<T>> strategy_;

public:
    void setCompressionStrategy(std::unique_ptr<CompressionStrategy<T>> strategy) {
        strategy_ = std::move(strategy);
    }
    
    std::vector<T> processAndCompress(const std::vector<T>& data) {
        if (strategy_) {
            std::cout << "Using " << strategy_->getName() << std::endl;
            return strategy_->compress(data);
        }
        return data;
    }
    
    std::vector<T> decompress(const std::vector<T>& data) {
        if (strategy_) {
            return strategy_->decompress(data);
        }
        return data;
    }
};

// Modern C++ Strategy with std::variant
#include <variant>

struct LinearPricing {
    double basePrice;
    
    double calculate(double quantity) const {
        return basePrice * quantity;
    }
};

struct TieredPricing {
    std::vector<std::pair<double, double>> tiers; // {threshold, price}
    
    double calculate(double quantity) const {
        double total = 0.0;
        double remaining = quantity;
        
        for (const auto& [threshold, price] : tiers) {
            if (remaining <= 0) break;
            
            double tierQuantity = std::min(remaining, threshold);
            total += tierQuantity * price;
            remaining -= tierQuantity;
        }
        
        return total;
    }
};

struct DiscountPricing {
    double basePrice;
    double discountPercent;
    double minimumQuantity;
    
    double calculate(double quantity) const {
        double total = basePrice * quantity;
        if (quantity >= minimumQuantity) {
            total *= (1.0 - discountPercent / 100.0);
        }
        return total;
    }
};

using PricingStrategy = std::variant<LinearPricing, TieredPricing, DiscountPricing>;

class PricingCalculator {
private:
    PricingStrategy strategy_;

public:
    void setStrategy(PricingStrategy strategy) {
        strategy_ = strategy;
    }
    
    double calculatePrice(double quantity) const {
        return std::visit([quantity](const auto& strategy) {
            return strategy.calculate(quantity);
        }, strategy_);
    }
    
    std::string getStrategyName() const {
        return std::visit([](const auto& strategy) -> std::string {
            if constexpr (std::is_same_v<std::decay_t<decltype(strategy)>, LinearPricing>) {
                return "Linear Pricing";
            } else if constexpr (std::is_same_v<std::decay_t<decltype(strategy)>, TieredPricing>) {
                return "Tiered Pricing";
            } else {
                return "Discount Pricing";
            }
        }, strategy_);
    }
};

void demonstrateBasicStrategy() {
    std::cout << "=== Basic Strategy Pattern (Sorting) ===\n\n";
    
    // Create test data
    std::vector<int> testData = {64, 34, 25, 12, 22, 11, 90, 88, 5, 77, 30, 42, 15};
    
    std::cout << "Original data: ";
    for (int num : testData) {
        std::cout << num << " ";
    }
    std::cout << "\n\n";
    
    // Test different sorting strategies
    std::vector<std::unique_ptr<SortStrategy>> strategies;
    strategies.push_back(std::make_unique<BubbleSort>());
    strategies.push_back(std::make_unique<QuickSort>());
    strategies.push_back(std::make_unique<MergeSort>());
    strategies.push_back(std::make_unique<StdSort>());
    
    for (auto& strategy : strategies) {
        auto dataCopy = testData;
        
        Sorter sorter(std::move(strategy));
        std::cout << "Using " << sorter.getCurrentStrategy() << ":" << std::endl;
        sorter.performSort(dataCopy);
        
        std::cout << "Result: ";
        for (int num : dataCopy) {
            std::cout << num << " ";
        }
        std::cout << "\n\n";
    }
}

void demonstrateFunctionStrategy() {
    std::cout << "=== Function-based Strategy Pattern (Payment) ===\n\n";
    
    PaymentContext paymentSystem;
    
    std::vector<std::pair<PaymentProcessor, std::string>> paymentMethods = {
        {createCreditCardProcessor(), "Credit Card"},
        {createPayPalProcessor(), "PayPal"},
        {createBankTransferProcessor(), "Bank Transfer"}
    };
    
    double amount = 99.99;
    std::string orderDetails = "Order #12345 - Premium subscription";
    
    for (const auto& [processor, name] : paymentMethods) {
        std::cout << "--- Processing payment via " << name << " ---" << std::endl;
        paymentSystem.setPaymentMethod(processor);
        paymentSystem.processPayment(amount, orderDetails);
        std::cout << std::endl;
    }
}

void demonstrateTemplateStrategy() {
    std::cout << "=== Template Strategy Pattern (Compression) ===\n\n";
    
    std::vector<int> data = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5};
    
    std::cout << "Original data: ";
    for (int num : data) {
        std::cout << num << " ";
    }
    std::cout << " (size: " << data.size() << ")\n\n";
    
    DataProcessor<int> processor;
    
    // No compression
    std::cout << "--- No Compression Strategy ---" << std::endl;
    processor.setCompressionStrategy(std::make_unique<NoCompression<int>>());
    auto result1 = processor.processAndCompress(data);
    std::cout << "Compressed data: ";
    for (int num : result1) {
        std::cout << num << " ";
    }
    std::cout << " (size: " << result1.size() << ")\n\n";
    
    // Simple compression
    std::cout << "--- Simple Compression Strategy ---" << std::endl;
    processor.setCompressionStrategy(std::make_unique<SimpleCompression<int>>());
    auto result2 = processor.processAndCompress(data);
    std::cout << "Compressed data: ";
    for (int num : result2) {
        std::cout << num << " ";
    }
    std::cout << " (size: " << result2.size() << ")\n\n";
}

void demonstrateVariantStrategy() {
    std::cout << "=== std::variant Strategy Pattern (Pricing) ===\n\n";
    
    PricingCalculator calculator;
    
    // Linear pricing
    calculator.setStrategy(LinearPricing{10.0});
    std::cout << "--- " << calculator.getStrategyName() << " ---" << std::endl;
    std::cout << "Price for 5 units: $" << calculator.calculatePrice(5) << std::endl;
    std::cout << "Price for 10 units: $" << calculator.calculatePrice(10) << std::endl;
    std::cout << std::endl;
    
    // Tiered pricing
    calculator.setStrategy(TieredPricing{{{10, 8.0}, {20, 6.0}, {1000, 5.0}}});
    std::cout << "--- " << calculator.getStrategyName() << " ---" << std::endl;
    std::cout << "Price for 5 units: $" << calculator.calculatePrice(5) << std::endl;
    std::cout << "Price for 15 units: $" << calculator.calculatePrice(15) << std::endl;
    std::cout << "Price for 35 units: $" << calculator.calculatePrice(35) << std::endl;
    std::cout << std::endl;
    
    // Discount pricing
    calculator.setStrategy(DiscountPricing{10.0, 15.0, 20.0});
    std::cout << "--- " << calculator.getStrategyName() << " ---" << std::endl;
    std::cout << "Price for 10 units: $" << calculator.calculatePrice(10) << std::endl;
    std::cout << "Price for 25 units: $" << calculator.calculatePrice(25) << " (with 15% discount)" << std::endl;
    std::cout << std::endl;
}

void demonstrateStrategySelection() {
    std::cout << "=== Dynamic Strategy Selection ===\n\n";
    
    // Simulate choosing sorting strategy based on data size
    auto chooseSortingStrategy = [](size_t dataSize) -> std::unique_ptr<SortStrategy> {
        if (dataSize < 10) {
            std::cout << "Small dataset detected. Using Bubble Sort for simplicity." << std::endl;
            return std::make_unique<BubbleSort>();
        } else if (dataSize < 1000) {
            std::cout << "Medium dataset detected. Using Quick Sort for efficiency." << std::endl;
            return std::make_unique<QuickSort>();
        } else {
            std::cout << "Large dataset detected. Using std::sort for optimal performance." << std::endl;
            return std::make_unique<StdSort>();
        }
    };
    
    std::vector<size_t> dataSizes = {5, 50, 5000};
    
    for (size_t size : dataSizes) {
        std::cout << "Processing dataset of size " << size << ":" << std::endl;
        
        // Generate random data
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 100);
        
        std::vector<int> data(size);
        std::generate(data.begin(), data.end(), [&]() { return dis(gen); });
        
        auto strategy = chooseSortingStrategy(size);
        Sorter sorter(std::move(strategy));
        sorter.performSort(data);
        
        std::cout << "Strategy used: " << sorter.getCurrentStrategy() << std::endl;
        std::cout << "First 10 elements after sorting: ";
        for (size_t i = 0; i < std::min(size_t(10), data.size()); ++i) {
            std::cout << data[i] << " ";
        }
        std::cout << "\n\n";
    }
}

void demonstrateStrategyPattern() {
    std::cout << "=== Strategy Pattern Demonstration ===\n\n";
    
    demonstrateBasicStrategy();
    demonstrateFunctionStrategy();
    demonstrateTemplateStrategy();
    demonstrateVariantStrategy();
    demonstrateStrategySelection();
    
    std::cout << "=== Strategy Pattern Benefits ===\n";
    std::cout << "✓ Algorithms can be switched at runtime\n";
    std::cout << "✓ Easy to add new strategies without modifying existing code\n";
    std::cout << "✓ Eliminates conditional statements for algorithm selection\n";
    std::cout << "✓ Each strategy is independently testable\n";
    std::cout << "✓ Follows Open/Closed Principle\n";
    std::cout << "✓ Supports both inheritance and functional approaches\n";
}

int main() {
    demonstrateStrategyPattern();
    return 0;
}
