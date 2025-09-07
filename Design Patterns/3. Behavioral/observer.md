# Observer Pattern Study Guide

## Overview
The Observer pattern defines a one-to-many dependency between objects so that when one object changes state, all its dependents are notified and updated automatically. It's also known as the Publish-Subscribe pattern.

## Intent
- Define a subscription mechanism to notify multiple objects about events
- Maintain loose coupling between subject and observers
- Support broadcast communication
- Allow dynamic subscription and unsubscription

## Structure
```
Subject                    Observer
+-- attach(observer)       +-- update(subject)
+-- detach(observer)       
+-- notify()               ConcreteObserver
|                          +-- update(subject)
ConcreteSubject            +-- observerState
+-- subjectState           
+-- getState()             
+-- setState()             
```

## When to Use
✅ **Good for:**
- Event-driven programming
- Model-View architectures (MVC, MVP, MVVM)
- Real-time data updates (stock prices, sensor data)
- Notification systems
- UI components that need to react to data changes
- Publish-subscribe messaging
- Implementing reactive programming

❌ **Avoid when:**
- Simple one-to-one relationships
- Performance is critical (observer notification overhead)
- Observer order matters (observers are notified in arbitrary order)
- Complex inter-observer dependencies

## Classic Implementation

### Basic Observer Pattern
```cpp
class Subject {
public:
    virtual ~Subject() = default;
    virtual void attach(Observer* observer) = 0;
    virtual void detach(Observer* observer) = 0;
    virtual void notify() = 0;
};

class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(Subject* subject) = 0;
};

class ConcreteSubject : public Subject {
private:
    std::vector<Observer*> observers_;
    int state_;

public:
    void attach(Observer* observer) override {
        observers_.push_back(observer);
    }
    
    void detach(Observer* observer) override {
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end());
    }
    
    void notify() override {
        for (auto* observer : observers_) {
            observer->update(this);
        }
    }
    
    void setState(int state) {
        state_ = state;
        notify();
    }
    
    int getState() const { return state_; }
};
```

## Modern C++ Implementations

### 1. Smart Pointer Based (Memory Safe)
```cpp
class SafeSubject {
private:
    std::vector<std::weak_ptr<Observer>> observers_;

public:
    void attach(std::shared_ptr<Observer> observer) {
        observers_.push_back(observer);
    }
    
    void notify() {
        auto it = observers_.begin();
        while (it != observers_.end()) {
            if (auto observer = it->lock()) {
                observer->update(this);
                ++it;
            } else {
                it = observers_.erase(it);  // Remove expired observer
            }
        }
    }
};
```

### 2. Function-Based Observer
```cpp
template<typename EventData>
class FunctionObservable {
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

// Usage
FunctionObservable<int> counter;
counter.subscribe([](int value) {
    std::cout << "Value changed to: " << value << std::endl;
});
```

### 3. Template Observer Pattern
```cpp
template<typename T>
class Observable {
public:
    using ObserverType = std::function<void(const T&)>;
    
private:
    std::vector<ObserverType> observers_;
    T data_;

public:
    void subscribe(ObserverType observer) {
        observers_.push_back(observer);
    }
    
    void setValue(const T& value) {
        data_ = value;
        notify();
    }
    
    const T& getValue() const { return data_; }

private:
    void notify() {
        for (const auto& observer : observers_) {
            observer(data_);
        }
    }
};
```

### 4. Signal-Slot Pattern (Qt-style)
```cpp
template<typename... Args>
class Signal {
private:
    std::vector<std::function<void(Args...)>> slots_;

public:
    void connect(std::function<void(Args...)> slot) {
        slots_.push_back(slot);
    }
    
    void emit(Args... args) {
        for (const auto& slot : slots_) {
            slot(args...);
        }
    }
    
    void operator()(Args... args) {
        emit(args...);
    }
};

// Usage
Signal<int, std::string> signal;
signal.connect([](int id, const std::string& msg) {
    std::cout << "ID: " << id << ", Message: " << msg << std::endl;
});
signal.emit(42, "Hello World");
```

## Advanced Techniques

### 1. Type-Safe Observer with CRTP
```cpp
template<typename Subject, typename EventData>
class TypedObserver {
public:
    virtual ~TypedObserver() = default;
    virtual void onNotify(Subject* subject, const EventData& event) = 0;
};

template<typename EventData>
class TypedSubject {
private:
    std::vector<TypedObserver<TypedSubject, EventData>*> observers_;

public:
    void attach(TypedObserver<TypedSubject, EventData>* observer) {
        observers_.push_back(observer);
    }
    
    void notify(const EventData& event) {
        for (auto* observer : observers_) {
            observer->onNotify(this, event);
        }
    }
};
```

### 2. Filtered Observer
```cpp
template<typename T>
class FilteredObservable {
private:
    std::vector<std::pair<std::function<bool(const T&)>, std::function<void(const T&)>>> observers_;

public:
    void subscribe(std::function<void(const T&)> observer, 
                  std::function<bool(const T&)> filter = [](const T&) { return true; }) {
        observers_.emplace_back(filter, observer);
    }
    
    void notify(const T& data) {
        for (const auto& [filter, observer] : observers_) {
            if (filter(data)) {
                observer(data);
            }
        }
    }
};

// Usage
FilteredObservable<int> numbers;
numbers.subscribe(
    [](int n) { std::cout << "Even: " << n << std::endl; },
    [](int n) { return n % 2 == 0; }  // Filter for even numbers
);
```

### 3. Async Observer
```cpp
class AsyncObservable {
private:
    std::vector<std::function<void()>> observers_;
    std::thread::id mainThreadId_;

public:
    AsyncObservable() : mainThreadId_(std::this_thread::get_id()) {}
    
    void subscribe(std::function<void()> observer) {
        observers_.push_back(observer);
    }
    
    void notifyAsync() {
        for (const auto& observer : observers_) {
            std::async(std::launch::async, observer);
        }
    }
    
    void notifyOnMainThread() {
        if (std::this_thread::get_id() == mainThreadId_) {
            for (const auto& observer : observers_) {
                observer();
            }
        } else {
            // Post to main thread (implementation depends on framework)
            postToMainThread([this]() { notifyOnMainThread(); });
        }
    }
};
```

### 4. Observable Collections
```cpp
template<typename T>
class ObservableVector {
public:
    enum class ChangeType { Added, Removed, Modified };
    
    struct ChangeEvent {
        ChangeType type;
        size_t index;
        T oldValue;
        T newValue;
    };
    
    using ChangeObserver = std::function<void(const ChangeEvent&)>;

private:
    std::vector<T> data_;
    std::vector<ChangeObserver> observers_;

public:
    void subscribe(ChangeObserver observer) {
        observers_.push_back(observer);
    }
    
    void push_back(const T& value) {
        size_t index = data_.size();
        data_.push_back(value);
        notify({ChangeType::Added, index, T{}, value});
    }
    
    void erase(size_t index) {
        if (index < data_.size()) {
            T oldValue = data_[index];
            data_.erase(data_.begin() + index);
            notify({ChangeType::Removed, index, oldValue, T{}});
        }
    }
    
    void set(size_t index, const T& value) {
        if (index < data_.size()) {
            T oldValue = data_[index];
            data_[index] = value;
            notify({ChangeType::Modified, index, oldValue, value});
        }
    }

private:
    void notify(const ChangeEvent& event) {
        for (const auto& observer : observers_) {
            observer(event);
        }
    }
};
```

## Observer Variations

### 1. Push vs Pull Model
```cpp
// Push Model - Subject sends all data
class PushSubject : public Subject {
    int state_;
public:
    void notify() override {
        for (auto* observer : observers_) {
            observer->update(state_);  // Push data
        }
    }
};

// Pull Model - Observer queries subject for data
class PullSubject : public Subject {
    int state_;
public:
    void notify() override {
        for (auto* observer : observers_) {
            observer->update(this);  // Observer pulls data
        }
    }
    
    int getState() const { return state_; }
};
```

### 2. Event-Specific Observers
```cpp
class MultiEventSubject {
public:
    Signal<int> onValueChanged;
    Signal<std::string> onNameChanged;
    Signal<> onDestroyed;
    
    void setValue(int value) {
        value_ = value;
        onValueChanged.emit(value);
    }
    
    void setName(const std::string& name) {
        name_ = name;
        onNameChanged.emit(name);
    }
    
    ~MultiEventSubject() {
        onDestroyed.emit();
    }

private:
    int value_;
    std::string name_;
};
```

## Performance Considerations

### Memory Management
```cpp
// Use weak_ptr to avoid circular references
class SafeObserver : public std::enable_shared_from_this<SafeObserver> {
public:
    void subscribeToSubject(std::shared_ptr<Subject> subject) {
        subject->attach(shared_from_this());
        subjects_.push_back(subject);  // Keep reference to subject
    }

private:
    std::vector<std::weak_ptr<Subject>> subjects_;
};
```

### Performance Optimization
```cpp
class OptimizedSubject {
private:
    std::vector<Observer*> observers_;
    bool isNotifying_ = false;
    std::vector<Observer*> pendingRemovals_;

public:
    void detach(Observer* observer) {
        if (isNotifying_) {
            pendingRemovals_.push_back(observer);
        } else {
            observers_.erase(
                std::remove(observers_.begin(), observers_.end(), observer),
                observers_.end());
        }
    }
    
    void notify() {
        isNotifying_ = true;
        
        for (auto* observer : observers_) {
            observer->update(this);
        }
        
        isNotifying_ = false;
        
        // Process pending removals
        for (auto* observer : pendingRemovals_) {
            observers_.erase(
                std::remove(observers_.begin(), observers_.end(), observer),
                observers_.end());
        }
        pendingRemovals_.clear();
    }
};
```

## Common Pitfalls

### 1. **Circular References**
```cpp
// Bad - potential circular reference
class BadObserver : public Observer {
    std::shared_ptr<Subject> subject_;  // Strong reference
public:
    void setSubject(std::shared_ptr<Subject> subject) {
        subject_ = subject;
        subject_->attach(shared_from_this());  // Subject holds strong ref to this
    }
};

// Good - use weak references
class GoodObserver : public Observer {
    std::weak_ptr<Subject> subject_;  // Weak reference
public:
    void setSubject(std::shared_ptr<Subject> subject) {
        subject_ = subject;
        subject_->attach(shared_from_this());
    }
};
```

### 2. **Observer Order Dependency**
```cpp
// Bad - depending on notification order
class OrderDependentObserver : public Observer {
public:
    void update(Subject* subject) override {
        // Assumes other observers have already processed the update
        auto* other = getOtherObserver();
        if (other->hasProcessed()) {  // BAD - order dependency
            doSomething();
        }
    }
};

// Good - make observers independent
class IndependentObserver : public Observer {
public:
    void update(Subject* subject) override {
        // Process update independently
        auto state = subject->getState();
        processState(state);
    }
};
```

### 3. **Exception Safety**
```cpp
// Bad - one observer exception breaks others
void notify() {
    for (auto* observer : observers_) {
        observer->update(this);  // If this throws, others aren't notified
    }
}

// Good - exception safe notification
void notify() {
    for (auto* observer : observers_) {
        try {
            observer->update(this);
        } catch (const std::exception& e) {
            // Log error but continue with other observers
            std::cerr << "Observer error: " << e.what() << std::endl;
        }
    }
}
```

### 4. **Memory Leaks with Raw Pointers**
```cpp
// Bad - potential memory leaks
class BadSubject {
    std::vector<Observer*> observers_;  // Raw pointers
public:
    ~BadSubject() {
        // Who deletes the observers?
    }
};

// Good - clear ownership
class GoodSubject {
    std::vector<std::weak_ptr<Observer>> observers_;  // Weak pointers
public:
    // Automatic cleanup of expired observers
};
```

## Testing Strategies

### 1. Mock Observers
```cpp
class MockObserver : public Observer {
public:
    MOCK_METHOD(void, update, (Subject* subject), (override));
};

TEST(ObserverTest, NotifiesObservers) {
    auto subject = std::make_unique<ConcreteSubject>();
    auto mockObserver = std::make_shared<MockObserver>();
    
    subject->attach(mockObserver);
    
    EXPECT_CALL(*mockObserver, update(subject.get())).Times(1);
    subject->setState(42);
}
```

### 2. Test Observer Lifecycle
```cpp
TEST(ObserverTest, HandlesObserverDestruction) {
    auto subject = std::make_unique<SafeSubject>();
    
    {
        auto observer = std::make_shared<ConcreteObserver>();
        subject->attach(observer);
        EXPECT_EQ(subject->getObserverCount(), 1);
    }  // Observer destroyed
    
    subject->notify();  // Should not crash
    EXPECT_EQ(subject->getObserverCount(), 0);
}
```

### 3. Test Exception Safety
```cpp
TEST(ObserverTest, HandlesObserverExceptions) {
    auto subject = std::make_unique<ConcreteSubject>();
    
    auto throwingObserver = std::make_shared<ThrowingObserver>();
    auto normalObserver = std::make_shared<MockObserver>();
    
    subject->attach(throwingObserver);
    subject->attach(normalObserver);
    
    EXPECT_CALL(*normalObserver, update(testing::_)).Times(1);
    
    EXPECT_NO_THROW(subject->notify());  // Should handle exception gracefully
}
```

## Real-World Examples

### Model-View-Controller
```cpp
class Model : public Subject {
    std::string data_;
public:
    void setData(const std::string& data) {
        data_ = data;
        notify();
    }
    
    const std::string& getData() const { return data_; }
};

class View : public Observer {
public:
    void update(Subject* subject) override {
        if (auto* model = dynamic_cast<Model*>(subject)) {
            render(model->getData());
        }
    }
    
private:
    void render(const std::string& data) {
        std::cout << "Rendering: " << data << std::endl;
    }
};
```

### Event System
```cpp
class EventManager {
private:
    std::unordered_map<std::string, std::vector<std::function<void(const Event&)>>> handlers_;

public:
    void subscribe(const std::string& eventType, std::function<void(const Event&)> handler) {
        handlers_[eventType].push_back(handler);
    }
    
    void publish(const std::string& eventType, const Event& event) {
        auto it = handlers_.find(eventType);
        if (it != handlers_.end()) {
            for (const auto& handler : it->second) {
                handler(event);
            }
        }
    }
};
```

## Interview Questions

**Q: What's the difference between Observer and Publish-Subscribe?**
A: Observer pattern typically has direct references between subject and observers. Pub-Sub uses an intermediary (message broker) for decoupling.

**Q: How do you handle observer exceptions?**
A: Wrap observer notifications in try-catch blocks to prevent one observer's exception from affecting others.

**Q: What's the difference between push and pull observer models?**
A: Push model sends data with notification, pull model requires observers to query the subject for data.

**Q: How do you prevent memory leaks in Observer pattern?**
A: Use weak_ptr for observer references, implement proper unsubscription, and use RAII for automatic cleanup.

**Q: When should you use Observer vs other patterns?**
A: Use Observer for one-to-many event notification where loose coupling is desired. Use Command for decoupled request handling, Strategy for algorithm selection.

## Conclusion
The Observer pattern is fundamental for event-driven architectures and reactive programming. Modern C++ implementations should use smart pointers for memory safety and consider function-based approaches for flexibility. It's essential for GUI frameworks, real-time systems, and any application requiring loose coupling between data producers and consumers.
