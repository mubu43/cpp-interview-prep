# Strategy Pattern Study Guide

## Overview
The Strategy pattern defines a family of algorithms, encapsulates each one, and makes them interchangeable. It lets the algorithm vary independently from clients that use it.

## Intent
- Define a family of algorithms and make them interchangeable
- Encapsulate algorithms and make them independent of clients
- Eliminate conditional statements for algorithm selection
- Support Open/Closed Principle for adding new algorithms

## Structure
```
Context                    Strategy (Abstract)
+-- strategy               +-- algorithmInterface()
+-- setStrategy()          
+-- executeStrategy()      ConcreteStrategyA
                          +-- algorithmInterface()
                          
                          ConcreteStrategyB
                          +-- algorithmInterface()
```

## When to Use
✅ **Good for:**
- Multiple ways to perform a task
- Runtime algorithm selection
- Eliminating large conditional statements
- Adding new algorithms without modifying existing code
- Different implementations for different contexts
- A/B testing different approaches
- Platform-specific implementations

❌ **Avoid when:**
- Only one algorithm exists and won't change
- Algorithms are very simple
- Client must be aware of algorithm differences
- Strategy creation overhead is significant

## Classic Implementation

### Basic Strategy Pattern
```cpp
class Strategy {
public:
    virtual ~Strategy() = default;
    virtual void execute() = 0;
};

class ConcreteStrategyA : public Strategy {
public:
    void execute() override {
        std::cout << "Algorithm A" << std::endl;
    }
};

class ConcreteStrategyB : public Strategy {
public:
    void execute() override {
        std::cout << "Algorithm B" << std::endl;
    }
};

class Context {
private:
    std::unique_ptr<Strategy> strategy_;

public:
    void setStrategy(std::unique_ptr<Strategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    void executeStrategy() {
        if (strategy_) {
            strategy_->execute();
        }
    }
};
```

## Modern C++ Approaches

### 1. Function-Based Strategy
```cpp
using SortFunction = std::function<void(std::vector<int>&)>;

class Sorter {
private:
    SortFunction sortFunc_;

public:
    void setStrategy(SortFunction func) {
        sortFunc_ = func;
    }
    
    void sort(std::vector<int>& data) {
        if (sortFunc_) {
            sortFunc_(data);
        }
    }
};

// Usage
Sorter sorter;
sorter.setStrategy([](std::vector<int>& data) {
    std::sort(data.begin(), data.end());
});
```

### 2. Template Strategy
```cpp
template<typename Strategy>
class Context {
private:
    Strategy strategy_;

public:
    template<typename... Args>
    Context(Args&&... args) : strategy_(std::forward<Args>(args)...) {}
    
    template<typename... Args>
    auto execute(Args&&... args) {
        return strategy_.execute(std::forward<Args>(args)...);
    }
};

// Strategy as a callable object
struct QuickSort {
    void execute(std::vector<int>& data) {
        // Quick sort implementation
    }
};

// Usage
Context<QuickSort> sorter;
```

### 3. std::variant Strategy
```cpp
struct StrategyA {
    void execute() { std::cout << "Strategy A\n"; }
};

struct StrategyB {
    void execute() { std::cout << "Strategy B\n"; }
};

using Strategy = std::variant<StrategyA, StrategyB>;

class Context {
private:
    Strategy strategy_;

public:
    void setStrategy(Strategy strategy) {
        strategy_ = strategy;
    }
    
    void execute() {
        std::visit([](auto& strategy) {
            strategy.execute();
        }, strategy_);
    }
};
```

### 4. Policy-Based Design
```cpp
template<typename SortPolicy, typename OutputPolicy>
class DataProcessor {
private:
    SortPolicy sortPolicy_;
    OutputPolicy outputPolicy_;

public:
    void process(std::vector<int>& data) {
        sortPolicy_.sort(data);
        outputPolicy_.output(data);
    }
};

struct QuickSortPolicy {
    void sort(std::vector<int>& data) {
        // Quick sort implementation
    }
};

struct ConsoleOutputPolicy {
    void output(const std::vector<int>& data) {
        for (int value : data) {
            std::cout << value << " ";
        }
    }
};

// Usage
DataProcessor<QuickSortPolicy, ConsoleOutputPolicy> processor;
```

## Advanced Techniques

### 1. Strategy Factory
```cpp
class StrategyFactory {
public:
    enum class StrategyType { BUBBLE_SORT, QUICK_SORT, MERGE_SORT };
    
    static std::unique_ptr<SortStrategy> create(StrategyType type) {
        switch (type) {
            case StrategyType::BUBBLE_SORT:
                return std::make_unique<BubbleSort>();
            case StrategyType::QUICK_SORT:
                return std::make_unique<QuickSort>();
            case StrategyType::MERGE_SORT:
                return std::make_unique<MergeSort>();
            default:
                return nullptr;
        }
    }
};
```

### 2. Strategy Chain
```cpp
class StrategyChain {
private:
    std::vector<std::unique_ptr<Strategy>> strategies_;

public:
    void addStrategy(std::unique_ptr<Strategy> strategy) {
        strategies_.push_back(std::move(strategy));
    }
    
    void executeAll() {
        for (const auto& strategy : strategies_) {
            strategy->execute();
        }
    }
};
```

### 3. Conditional Strategy Selection
```cpp
class AdaptiveContext {
private:
    std::map<std::string, std::unique_ptr<Strategy>> strategies_;
    std::function<std::string(const Data&)> selector_;

public:
    void addStrategy(const std::string& name, std::unique_ptr<Strategy> strategy) {
        strategies_[name] = std::move(strategy);
    }
    
    void setSelector(std::function<std::string(const Data&)> selector) {
        selector_ = selector;
    }
    
    void process(const Data& data) {
        if (selector_) {
            std::string strategyName = selector_(data);
            auto it = strategies_.find(strategyName);
            if (it != strategies_.end()) {
                it->second->execute(data);
            }
        }
    }
};
```

### 4. Lazy Strategy Loading
```cpp
class LazyContext {
private:
    std::map<std::string, std::function<std::unique_ptr<Strategy>()>> strategyFactories_;
    std::map<std::string, std::unique_ptr<Strategy>> loadedStrategies_;

public:
    void registerStrategy(const std::string& name, 
                         std::function<std::unique_ptr<Strategy>()> factory) {
        strategyFactories_[name] = factory;
    }
    
    void executeStrategy(const std::string& name) {
        auto it = loadedStrategies_.find(name);
        if (it == loadedStrategies_.end()) {
            // Load strategy on first use
            auto factoryIt = strategyFactories_.find(name);
            if (factoryIt != strategyFactories_.end()) {
                loadedStrategies_[name] = factoryIt->second();
                it = loadedStrategies_.find(name);
            }
        }
        
        if (it != loadedStrategies_.end()) {
            it->second->execute();
        }
    }
};
```

## Strategy Pattern Variations

### 1. State-Strategy Hybrid
```cpp
class StatefulStrategy {
protected:
    int state_ = 0;

public:
    virtual ~StatefulStrategy() = default;
    virtual void execute() = 0;
    virtual void reset() { state_ = 0; }
    
    int getState() const { return state_; }
};

class IncrementalStrategy : public StatefulStrategy {
public:
    void execute() override {
        std::cout << "Executing step " << ++state_ << std::endl;
    }
};
```

### 2. Parameterized Strategy
```cpp
template<typename T>
class ParameterizedStrategy {
public:
    virtual ~ParameterizedStrategy() = default;
    virtual void execute(const T& data) = 0;
};

template<typename T>
class ConfigurableStrategy : public ParameterizedStrategy<T> {
private:
    std::map<std::string, std::any> config_;

public:
    void setParameter(const std::string& key, const std::any& value) {
        config_[key] = value;
    }
    
    void execute(const T& data) override {
        // Use configuration parameters in execution
    }
};
```

### 3. Multi-Strategy Context
```cpp
class MultiStrategyContext {
private:
    std::unique_ptr<SortStrategy> sortStrategy_;
    std::unique_ptr<SearchStrategy> searchStrategy_;
    std::unique_ptr<OutputStrategy> outputStrategy_;

public:
    void setSortStrategy(std::unique_ptr<SortStrategy> strategy) {
        sortStrategy_ = std::move(strategy);
    }
    
    void setSearchStrategy(std::unique_ptr<SearchStrategy> strategy) {
        searchStrategy_ = std::move(strategy);
    }
    
    void setOutputStrategy(std::unique_ptr<OutputStrategy> strategy) {
        outputStrategy_ = std::move(strategy);
    }
    
    void processData(std::vector<int>& data, int target) {
        if (sortStrategy_) sortStrategy_->sort(data);
        if (searchStrategy_) searchStrategy_->search(data, target);
        if (outputStrategy_) outputStrategy_->output(data);
    }
};
```

## Performance Considerations

### Virtual Function Overhead
```cpp
// Virtual function call overhead
class VirtualStrategy {
public:
    virtual void execute() = 0;  // ~1-2 CPU cycles overhead
};

// Template-based zero-overhead
template<typename Strategy>
class TemplateStrategy {
    Strategy strategy_;
public:
    void execute() {
        strategy_.execute();  // Inlined, zero overhead
    }
};
```

### Strategy Object Lifecycle
```cpp
// Reuse strategy objects to avoid allocation overhead
class OptimizedContext {
private:
    std::unordered_map<std::string, std::unique_ptr<Strategy>> strategyPool_;
    
public:
    void executeStrategy(const std::string& name) {
        auto it = strategyPool_.find(name);
        if (it != strategyPool_.end()) {
            it->second->execute();  // Reuse existing strategy
        }
    }
};
```

### Memory Usage
```cpp
// Compare memory usage patterns
class HeavyStrategy {
    std::vector<double> lookupTable_;  // Large memory footprint
public:
    HeavyStrategy() : lookupTable_(1000000) {}  // Expensive construction
};

class LightStrategy {
public:
    void execute() {
        // Algorithm without large memory requirements
    }
};
```

## Common Pitfalls

### 1. **Strategy Explosion**
```cpp
// Bad - too many fine-grained strategies
class AddStrategy : public MathStrategy {
    double execute(double a, double b) override { return a + b; }
};

class SubtractStrategy : public MathStrategy {
    double execute(double a, double b) override { return a - b; }
};

// Good - use function objects or lambdas for simple operations
using MathOperation = std::function<double(double, double)>;

MathOperation add = [](double a, double b) { return a + b; };
MathOperation subtract = [](double a, double b) { return a - b; };
```

### 2. **Context Dependency**
```cpp
// Bad - strategy depends on context internals
class BadStrategy : public Strategy {
public:
    void execute(Context* context) override {
        auto* derived = dynamic_cast<SpecificContext*>(context);
        if (derived) {
            derived->getInternalData();  // Tight coupling
        }
    }
};

// Good - pass only necessary data
class GoodStrategy : public Strategy {
public:
    void execute(const Data& data) override {
        // Operate only on provided data
    }
};
```

### 3. **Incomplete Strategy Interface**
```cpp
// Bad - strategy interface too narrow
class NarrowStrategy {
public:
    virtual void sort(std::vector<int>& data) = 0;
    // Missing configuration, progress reporting, etc.
};

// Good - comprehensive interface
class ComprehensiveStrategy {
public:
    virtual void sort(std::vector<int>& data) = 0;
    virtual void configure(const Config& config) {}
    virtual void setProgressCallback(std::function<void(int)> callback) {}
    virtual std::string getName() const = 0;
    virtual size_t getComplexity() const = 0;
};
```

### 4. **Strategy State Management**
```cpp
// Bad - strategies with unmanaged state
class StatefulStrategy {
    int operationCount_;  // Shared state between calls
public:
    void execute() {
        ++operationCount_;  // Breaks strategy independence
    }
};

// Good - stateless strategies or explicit state management
class StatelessStrategy {
public:
    void execute(ExecutionContext& context) {
        ++context.operationCount;  // State managed by context
    }
};
```

## Testing Strategies

### 1. Mock Strategies
```cpp
class MockStrategy : public Strategy {
public:
    MOCK_METHOD(void, execute, (), (override));
};

TEST(ContextTest, ExecutesStrategy) {
    auto mockStrategy = std::make_unique<MockStrategy>();
    auto* rawPtr = mockStrategy.get();
    
    Context context;
    context.setStrategy(std::move(mockStrategy));
    
    EXPECT_CALL(*rawPtr, execute()).Times(1);
    context.executeStrategy();
}
```

### 2. Strategy Comparison Tests
```cpp
TEST(StrategyTest, AllStrategiesProduceSameResult) {
    std::vector<int> data = {3, 1, 4, 1, 5, 9, 2, 6};
    std::vector<int> expected = {1, 1, 2, 3, 4, 5, 6, 9};
    
    std::vector<std::unique_ptr<SortStrategy>> strategies;
    strategies.push_back(std::make_unique<BubbleSort>());
    strategies.push_back(std::make_unique<QuickSort>());
    strategies.push_back(std::make_unique<MergeSort>());
    
    for (auto& strategy : strategies) {
        auto testData = data;
        strategy->sort(testData);
        EXPECT_EQ(testData, expected) << "Strategy: " << strategy->getName();
    }
}
```

### 3. Performance Testing
```cpp
TEST(StrategyTest, PerformanceComparison) {
    std::vector<int> largeData(10000);
    std::iota(largeData.begin(), largeData.end(), 1);
    std::random_shuffle(largeData.begin(), largeData.end());
    
    auto quickSort = std::make_unique<QuickSort>();
    auto mergeSort = std::make_unique<MergeSort>();
    
    auto testData1 = largeData;
    auto start = std::chrono::high_resolution_clock::now();
    quickSort->sort(testData1);
    auto quickSortTime = std::chrono::high_resolution_clock::now() - start;
    
    auto testData2 = largeData;
    start = std::chrono::high_resolution_clock::now();
    mergeSort->sort(testData2);
    auto mergeSortTime = std::chrono::high_resolution_clock::now() - start;
    
    std::cout << "QuickSort: " << quickSortTime.count() << "ns\n";
    std::cout << "MergeSort: " << mergeSortTime.count() << "ns\n";
}
```

## Real-World Examples

### File Compression
```cpp
class CompressionStrategy {
public:
    virtual ~CompressionStrategy() = default;
    virtual std::vector<uint8_t> compress(const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> decompress(const std::vector<uint8_t>& data) = 0;
};

class ZipCompression : public CompressionStrategy {
    // ZIP compression implementation
};

class GzipCompression : public CompressionStrategy {
    // GZIP compression implementation
};

class FileArchiver {
    std::unique_ptr<CompressionStrategy> compression_;
public:
    void setCompressionStrategy(std::unique_ptr<CompressionStrategy> strategy) {
        compression_ = std::move(strategy);
    }
    
    void archiveFile(const std::string& filename) {
        // Read file and compress using current strategy
    }
};
```

### Rendering System
```cpp
class RenderStrategy {
public:
    virtual void render(const Scene& scene) = 0;
};

class OpenGLRenderer : public RenderStrategy {
    void render(const Scene& scene) override {
        // OpenGL rendering
    }
};

class VulkanRenderer : public RenderStrategy {
    void render(const Scene& scene) override {
        // Vulkan rendering
    }
};

class SoftwareRenderer : public RenderStrategy {
    void render(const Scene& scene) override {
        // Software rendering
    }
};
```

### Payment Processing
```cpp
class PaymentStrategy {
public:
    virtual bool processPayment(double amount, const PaymentDetails& details) = 0;
    virtual std::string getPaymentMethod() const = 0;
};

class CreditCardPayment : public PaymentStrategy {
    bool processPayment(double amount, const PaymentDetails& details) override {
        // Credit card processing
        return true;
    }
    
    std::string getPaymentMethod() const override { return "Credit Card"; }
};

class PayPalPayment : public PaymentStrategy {
    bool processPayment(double amount, const PaymentDetails& details) override {
        // PayPal processing
        return true;
    }
    
    std::string getPaymentMethod() const override { return "PayPal"; }
};
```

## Interview Questions

**Q: When would you use Strategy over State pattern?**
A: Use Strategy when you want to change algorithms/behavior without changing state. Use State when behavior depends on object's internal state.

**Q: How does Strategy differ from Template Method?**
A: Strategy uses composition to change entire algorithms, Template Method uses inheritance to change specific steps of an algorithm.

**Q: Performance implications of Strategy pattern?**
A: Virtual function calls add minimal overhead (~1-2 cycles). Template-based strategies can be zero overhead. Function objects offer flexibility with good performance.

**Q: How do you handle strategy configuration?**
A: Pass configuration to strategy constructor, provide configuration methods, or pass configuration with each execution call.

**Q: Can Strategy pattern be combined with other patterns?**
A: Yes - commonly combined with Factory (for strategy creation), State (for stateful strategies), and Observer (for strategy change notifications).

## Conclusion
The Strategy pattern is excellent for eliminating conditional logic and supporting the Open/Closed Principle. Modern C++ offers multiple implementation approaches:
- Classic inheritance for polymorphic behavior
- Function objects for flexibility
- Templates for performance
- std::variant for type safety

Choose the approach based on your specific requirements for performance, flexibility, and type safety.
