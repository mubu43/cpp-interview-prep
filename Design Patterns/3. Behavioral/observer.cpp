#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <string>
#include <functional>

// Forward declaration
class Observer;

// Subject interface
class Subject {
public:
    virtual ~Subject() = default;
    virtual void attach(std::shared_ptr<Observer> observer) = 0;
    virtual void detach(std::shared_ptr<Observer> observer) = 0;
    virtual void notify() = 0;
};

// Observer interface
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(Subject* subject) = 0;
    virtual std::string getName() const = 0;
};

// Concrete Subject - Stock Price Monitor
class Stock : public Subject {
private:
    std::vector<std::weak_ptr<Observer>> observers_;
    std::string symbol_;
    double price_;
    double previousPrice_;

public:
    explicit Stock(const std::string& symbol, double initialPrice = 0.0)
        : symbol_(symbol), price_(initialPrice), previousPrice_(initialPrice) {}

    void attach(std::shared_ptr<Observer> observer) override {
        observers_.push_back(observer);
        std::cout << "Attached observer: " << observer->getName() 
                  << " to stock: " << symbol_ << std::endl;
    }

    void detach(std::shared_ptr<Observer> observer) override {
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [&observer](const std::weak_ptr<Observer>& weak_obs) {
                    auto shared_obs = weak_obs.lock();
                    return !shared_obs || shared_obs == observer;
                }),
            observers_.end());
        std::cout << "Detached observer: " << observer->getName() 
                  << " from stock: " << symbol_ << std::endl;
    }

    void notify() override {
        std::cout << "\nNotifying observers of " << symbol_ 
                  << " price change: $" << previousPrice_ 
                  << " -> $" << price_ << std::endl;
        
        // Clean up expired weak_ptrs and notify valid observers
        auto it = observers_.begin();
        while (it != observers_.end()) {
            if (auto observer = it->lock()) {
                observer->update(this);
                ++it;
            } else {
                it = observers_.erase(it);  // Remove expired weak_ptr
            }
        }
    }

    void setPrice(double newPrice) {
        if (newPrice != price_) {
            previousPrice_ = price_;
            price_ = newPrice;
            notify();
        }
    }

    // Getters
    const std::string& getSymbol() const { return symbol_; }
    double getPrice() const { return price_; }
    double getPreviousPrice() const { return previousPrice_; }
    double getPriceChange() const { return price_ - previousPrice_; }
    double getPriceChangePercent() const { 
        return previousPrice_ != 0 ? (getPriceChange() / previousPrice_) * 100 : 0; 
    }
    
    size_t getObserverCount() const {
        // Count valid observers
        size_t count = 0;
        for (const auto& weak_obs : observers_) {
            if (weak_obs.lock()) ++count;
        }
        return count;
    }
};

// Concrete Observer - Individual Investor
class Investor : public Observer {
private:
    std::string name_;
    double portfolioValue_;
    std::vector<std::pair<std::string, int>> holdings_;  // symbol, shares

public:
    explicit Investor(const std::string& name, double portfolioValue = 10000.0)
        : name_(name), portfolioValue_(portfolioValue) {}

    void update(Subject* subject) override {
        if (auto stock = dynamic_cast<Stock*>(subject)) {
            double change = stock->getPriceChange();
            double changePercent = stock->getPriceChangePercent();
            
            std::cout << "📊 Investor " << name_ << " notified: "
                      << stock->getSymbol() << " "
                      << (change >= 0 ? "↗️" : "↘️") << " $" << stock->getPrice()
                      << " (" << (change >= 0 ? "+" : "") << changePercent << "%)"
                      << std::endl;

            // Check if investor holds this stock
            auto it = std::find_if(holdings_.begin(), holdings_.end(),
                [&](const auto& holding) { return holding.first == stock->getSymbol(); });
            
            if (it != holdings_.end()) {
                int shares = it->second;
                double positionChange = change * shares;
                portfolioValue_ += positionChange;
                
                std::cout << "   💰 Portfolio impact: " 
                          << (positionChange >= 0 ? "+" : "") << positionChange
                          << " (holding " << shares << " shares)" << std::endl;
            }
        }
    }

    std::string getName() const override { return name_; }
    
    void buyStock(const std::string& symbol, int shares) {
        auto it = std::find_if(holdings_.begin(), holdings_.end(),
            [&](const auto& holding) { return holding.first == symbol; });
        
        if (it != holdings_.end()) {
            it->second += shares;
        } else {
            holdings_.emplace_back(symbol, shares);
        }
        
        std::cout << "🛒 " << name_ << " bought " << shares << " shares of " << symbol << std::endl;
    }
    
    double getPortfolioValue() const { return portfolioValue_; }
    
    void printHoldings() const {
        std::cout << "Holdings for " << name_ << ":" << std::endl;
        for (const auto& [symbol, shares] : holdings_) {
            std::cout << "  " << symbol << ": " << shares << " shares" << std::endl;
        }
    }
};

// Concrete Observer - Trading Algorithm
class TradingAlgorithm : public Observer {
private:
    std::string name_;
    double buyThreshold_;
    double sellThreshold_;
    bool isActive_;

public:
    TradingAlgorithm(const std::string& name, double buyThreshold, double sellThreshold)
        : name_(name), buyThreshold_(buyThreshold), sellThreshold_(sellThreshold), isActive_(true) {}

    void update(Subject* subject) override {
        if (!isActive_) return;
        
        if (auto stock = dynamic_cast<Stock*>(subject)) {
            double changePercent = stock->getPriceChangePercent();
            
            std::cout << "🤖 Algorithm " << name_ << " analyzing: "
                      << stock->getSymbol() << " change: " << changePercent << "%" << std::endl;

            if (changePercent <= sellThreshold_) {
                std::cout << "   🔴 SELL signal triggered (threshold: " << sellThreshold_ << "%)" << std::endl;
            } else if (changePercent >= buyThreshold_) {
                std::cout << "   🟢 BUY signal triggered (threshold: " << buyThreshold_ << "%)" << std::endl;
            } else {
                std::cout << "   ⚪ HOLD - no action" << std::endl;
            }
        }
    }

    std::string getName() const override { return name_; }
    
    void setActive(bool active) { isActive_ = active; }
    bool isActive() const { return isActive_; }
};

// Concrete Observer - News Reporter
class NewsReporter : public Observer {
private:
    std::string outlet_;
    double significantChangeThreshold_;

public:
    NewsReporter(const std::string& outlet, double threshold = 5.0)
        : outlet_(outlet), significantChangeThreshold_(threshold) {}

    void update(Subject* subject) override {
        if (auto stock = dynamic_cast<Stock*>(subject)) {
            double changePercent = std::abs(stock->getPriceChangePercent());
            
            if (changePercent >= significantChangeThreshold_) {
                std::cout << "📰 " << outlet_ << " BREAKING NEWS: "
                          << stock->getSymbol() << " sees significant movement of "
                          << stock->getPriceChangePercent() << "%" << std::endl;
                std::cout << "   Current price: $" << stock->getPrice() 
                          << " (was $" << stock->getPreviousPrice() << ")" << std::endl;
            }
        }
    }

    std::string getName() const override { return outlet_; }
};

// Function-based Observer using std::function
class FunctionObserver : public Observer {
private:
    std::string name_;
    std::function<void(Stock*)> callback_;

public:
    FunctionObserver(const std::string& name, std::function<void(Stock*)> callback)
        : name_(name), callback_(callback) {}

    void update(Subject* subject) override {
        if (auto stock = dynamic_cast<Stock*>(subject)) {
            callback_(stock);
        }
    }

    std::string getName() const override { return name_; }
};

// Observable template for generic observer pattern
template<typename EventData>
class Observable {
private:
    std::vector<std::function<void(const EventData&)>> observers_;

public:
    void subscribe(std::function<void(const EventData&)> observer) {
        observers_.push_back(observer);
    }

    void notify(const EventData& data) {
        for (const auto& observer : observers_) {
            observer(data);
        }
    }
};

// Event data structure
struct PriceChangeEvent {
    std::string symbol;
    double oldPrice;
    double newPrice;
    double changePercent;
    std::chrono::system_clock::time_point timestamp;
};

void demonstrateBasicObserver() {
    std::cout << "=== Basic Observer Pattern ===\n\n";
    
    // Create stocks
    auto appleStock = std::make_unique<Stock>("AAPL", 150.0);
    auto googleStock = std::make_unique<Stock>("GOOGL", 2800.0);
    
    // Create observers
    auto investor1 = std::make_shared<Investor>("Alice", 50000.0);
    auto investor2 = std::make_shared<Investor>("Bob", 30000.0);
    auto algorithm = std::make_shared<TradingAlgorithm>("MomentumBot", 5.0, -3.0);
    auto reporter = std::make_shared<NewsReporter>("TechNews Daily", 4.0);
    
    // Set up some holdings
    investor1->buyStock("AAPL", 100);
    investor2->buyStock("AAPL", 50);
    investor2->buyStock("GOOGL", 10);
    
    // Attach observers to subjects
    appleStock->attach(investor1);
    appleStock->attach(investor2);
    appleStock->attach(algorithm);
    appleStock->attach(reporter);
    
    googleStock->attach(investor2);
    googleStock->attach(algorithm);
    googleStock->attach(reporter);
    
    std::cout << "\n--- Price Changes ---\n";
    
    // Simulate price changes
    appleStock->setPrice(157.5);  // +5% change
    std::cout << std::endl;
    
    googleStock->setPrice(2716.0);  // -3% change
    std::cout << std::endl;
    
    appleStock->setPrice(142.5);  // -10% change from original
    std::cout << std::endl;
    
    // Show observer counts
    std::cout << "Observer counts:" << std::endl;
    std::cout << "AAPL: " << appleStock->getObserverCount() << " observers" << std::endl;
    std::cout << "GOOGL: " << googleStock->getObserverCount() << " observers" << std::endl;
}

void demonstrateFunctionObserver() {
    std::cout << "\n=== Function-based Observer ===\n\n";
    
    auto teslaStock = std::make_unique<Stock>("TSLA", 800.0);
    
    // Lambda-based observers
    auto priceLogger = std::make_shared<FunctionObserver>("PriceLogger",
        [](Stock* stock) {
            std::cout << "📝 Logging price change: " << stock->getSymbol() 
                      << " is now $" << stock->getPrice() << std::endl;
        });
    
    auto volatilityTracker = std::make_shared<FunctionObserver>("VolatilityTracker",
        [](Stock* stock) {
            double changePercent = std::abs(stock->getPriceChangePercent());
            if (changePercent > 10.0) {
                std::cout << "⚠️  HIGH VOLATILITY ALERT: " << stock->getSymbol() 
                          << " moved " << changePercent << "%" << std::endl;
            }
        });
    
    teslaStock->attach(priceLogger);
    teslaStock->attach(volatilityTracker);
    
    teslaStock->setPrice(720.0);  // -10% change
    teslaStock->setPrice(880.0);  // +22.2% change from original
}

void demonstrateTemplateObservable() {
    std::cout << "\n=== Template-based Observable ===\n\n";
    
    Observable<PriceChangeEvent> eventSystem;
    
    // Subscribe various handlers
    eventSystem.subscribe([](const PriceChangeEvent& event) {
        std::cout << "📈 Event Logger: " << event.symbol 
                  << " changed from $" << event.oldPrice 
                  << " to $" << event.newPrice << std::endl;
    });
    
    eventSystem.subscribe([](const PriceChangeEvent& event) {
        if (std::abs(event.changePercent) > 5.0) {
            std::cout << "🚨 Alert System: Significant change detected in " 
                      << event.symbol << " (" << event.changePercent << "%)" << std::endl;
        }
    });
    
    eventSystem.subscribe([](const PriceChangeEvent& event) {
        std::cout << "💾 Database: Storing price update for " << event.symbol << std::endl;
    });
    
    // Simulate events
    PriceChangeEvent event1{
        "MSFT", 300.0, 315.0, 5.0, std::chrono::system_clock::now()
    };
    
    PriceChangeEvent event2{
        "AMZN", 3200.0, 3040.0, -5.0, std::chrono::system_clock::now()
    };
    
    std::cout << "Broadcasting MSFT event:" << std::endl;
    eventSystem.notify(event1);
    
    std::cout << "\nBroadcasting AMZN event:" << std::endl;
    eventSystem.notify(event2);
}

void demonstrateObserverLifecycle() {
    std::cout << "\n=== Observer Lifecycle Management ===\n\n";
    
    auto stock = std::make_unique<Stock>("META", 200.0);
    
    {
        // Create observers in local scope
        auto tempInvestor = std::make_shared<Investor>("Charlie", 25000.0);
        auto tempAlgorithm = std::make_shared<TradingAlgorithm>("ScalpBot", 2.0, -1.5);
        
        stock->attach(tempInvestor);
        stock->attach(tempAlgorithm);
        
        std::cout << "Observers attached. Count: " << stock->getObserverCount() << std::endl;
        
        stock->setPrice(210.0);  // Notify observers
        
        // Explicitly detach one observer
        stock->detach(tempInvestor);
        std::cout << "After detaching investor. Count: " << stock->getObserverCount() << std::endl;
        
        stock->setPrice(195.0);  // Only algorithm should be notified
        
    }  // Observers go out of scope here
    
    std::cout << "\nAfter observers went out of scope:" << std::endl;
    std::cout << "Observer count: " << stock->getObserverCount() << std::endl;
    
    stock->setPrice(180.0);  // Should have no observers to notify
}

void demonstrateObserverPattern() {
    std::cout << "=== Observer Pattern Demonstration ===\n\n";
    
    demonstrateBasicObserver();
    demonstrateFunctionObserver();
    demonstrateTemplateObservable();
    demonstrateObserverLifecycle();
    
    std::cout << "\n=== Observer Pattern Benefits ===\n";
    std::cout << "✓ Loose coupling between subject and observers\n";
    std::cout << "✓ Dynamic subscription/unsubscription\n";
    std::cout << "✓ Automatic notification of state changes\n";
    std::cout << "✓ Support for multiple observers\n";
    std::cout << "✓ Extensible - new observer types can be added easily\n";
    std::cout << "✓ Memory safety with weak_ptr usage\n";
}

int main() {
    demonstrateObserverPattern();
    return 0;
}
