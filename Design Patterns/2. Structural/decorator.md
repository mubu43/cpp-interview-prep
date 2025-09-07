# Decorator Pattern

## Overview
The Decorator pattern allows behavior to be added to objects dynamically without altering their structure. It provides a flexible alternative to subclassing for extending functionality.

## Intent
- **Attach additional responsibilities to an object dynamically**
- **Provide a flexible alternative to subclassing for extending functionality**
- **Compose behaviors rather than inherit them**

## Problem It Solves
- Need to add functionality to objects without modifying their class
- Subclassing becomes impractical when you have many combinations of features
- Want to add/remove responsibilities at runtime
- Inheritance creates static relationships that can't be changed at runtime

## Structure
```
Component (Coffee)
├── ConcreteComponent (SimpleCoffee)
└── Decorator (CoffeeDecorator)
    ├── ConcreteDecoratorA (MilkDecorator)
    ├── ConcreteDecoratorB (SugarDecorator)
    └── ConcreteDecoratorC (WhipDecorator)
```

## Key Components

### 1. Component
- Defines the interface for objects that can have responsibilities added dynamically
- Abstract base class or interface

### 2. ConcreteComponent
- The original object to which additional responsibilities can be attached
- Implements the Component interface

### 3. Decorator
- Maintains a reference to a Component object
- Implements the Component interface
- Base class for all concrete decorators

### 4. ConcreteDecorator
- Adds specific responsibilities to the component
- Can add state and behavior

## Implementation Details

### Basic Structure
```cpp
class Component {
public:
    virtual ~Component() = default;
    virtual std::string operation() = 0;
};

class ConcreteComponent : public Component {
public:
    std::string operation() override {
        return "ConcreteComponent";
    }
};

class Decorator : public Component {
protected:
    std::unique_ptr<Component> component_;
public:
    Decorator(std::unique_ptr<Component> component) 
        : component_(std::move(component)) {}
    
    std::string operation() override {
        return component_->operation();
    }
};
```

### Modern C++ Features
- Use `std::unique_ptr` for automatic memory management
- Use `std::move` semantics for efficient transfers
- Use `override` keyword for virtual functions
- Use RAII for resource management

## Real-World Examples

### 1. Coffee Shop
- Base: Simple Coffee
- Decorators: Milk, Sugar, Whipped Cream, Vanilla
- Each decorator adds cost and description

### 2. Text Formatting
- Base: Plain Text
- Decorators: Bold, Italic, Underline, Color
- Each decorator adds formatting tags

### 3. Stream Processing
- Base: FileStream
- Decorators: BufferedStream, CompressedStream, EncryptedStream
- Each decorator adds processing capability

### 4. UI Components
- Base: Window
- Decorators: BorderDecorator, ScrollDecorator, TitleDecorator
- Each decorator adds visual elements

## Advantages
1. **Runtime Composition**: Add/remove behavior at runtime
2. **Single Responsibility**: Each decorator has one specific purpose
3. **Open/Closed Principle**: Open for extension, closed for modification
4. **Flexible**: More flexible than static inheritance
5. **Combinable**: Decorators can be combined in any order

## Disadvantages
1. **Complexity**: Can create many small objects
2. **Identity Issues**: Decorated object != original object
3. **Order Dependency**: Order of decoration can matter
4. **Debugging**: Stack of decorators can be hard to debug
5. **Performance**: Extra indirection and object creation overhead

## When to Use

### Use When:
- You need to add responsibilities to objects dynamically
- Extension by subclassing is impractical
- You want to add features in combinations
- You need to remove responsibilities later
- Inheritance would result in an explosion of subclasses

### Don't Use When:
- The component hierarchy is simple and stable
- Performance is critical and overhead matters
- The behavior changes are few and well-defined
- You need type safety for specific combinations

## Common Mistakes
1. **Breaking the Interface**: Decorators must maintain the same interface
2. **Order Dependencies**: Making decorators depend on specific ordering
3. **Heavy Decorators**: Adding too much functionality to a single decorator
4. **Memory Leaks**: Not properly managing decorator chains
5. **Interface Bloat**: Making the component interface too large

## Interview Questions & Answers

### Q: How does Decorator differ from Inheritance?
**A:** Decorator uses composition and allows runtime behavior changes, while inheritance is compile-time and static. Decorator is more flexible but requires more objects.

### Q: What's the difference between Decorator and Adapter?
**A:** Decorator adds new functionality while keeping the same interface. Adapter changes the interface to make incompatible classes work together.

### Q: How do you handle the order of decorators?
**A:** Order matters in Decorator pattern. Design decorators to be independent when possible, or document the required order clearly.

### Q: What are the performance implications?
**A:** Each decorator adds a level of indirection. For performance-critical code, consider alternatives or optimize the decorator chain.

### Q: How do you manage memory in decorator chains?
**A:** Use smart pointers (`std::unique_ptr`) for automatic memory management and move semantics for efficient transfers.

## Best Practices
1. **Keep decorators lightweight** - Single responsibility
2. **Use smart pointers** - Automatic memory management
3. **Make decorators independent** - Avoid order dependencies
4. **Document interfaces clearly** - What each decorator adds
5. **Consider caching** - For expensive operations
6. **Use factory methods** - For common decorator combinations
7. **Test combinations** - Ensure decorators work together
8. **Profile performance** - Monitor overhead in critical paths

## Related Patterns
- **Composite**: Both use recursive composition
- **Strategy**: Both provide alternatives to inheritance
- **Chain of Responsibility**: Similar structure, different intent
- **Proxy**: Similar structure, controls access vs. adding behavior

## Modern C++ Considerations
- Use `std::unique_ptr` for ownership
- Use perfect forwarding for constructor arguments
- Consider `std::variant` for type-safe decoration
- Use CRTP for compile-time decoration when appropriate
- Consider concepts (C++20) for decorator constraints
