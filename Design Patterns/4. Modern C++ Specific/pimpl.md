# Pimpl (Pointer to Implementation) Pattern

## Overview
The Pimpl (Pointer to Implementation) idiom is a C++ technique for removing compilation dependencies by moving the private implementation details to a separate class and holding a pointer to it.

## Intent
- **Hide implementation details from header files**
- **Reduce compilation dependencies and compile times**
- **Maintain binary compatibility across library versions**
- **Achieve true encapsulation in C++**

## Problem It Solves
- Header files expose private implementation details
- Changes to private members force recompilation of all client code
- Large include dependencies in headers slow compilation
- Binary incompatibility when internal implementation changes
- Inability to forward declare certain types in headers

## Structure
```
Public Class (in header)
├── Public Interface Methods
└── std::unique_ptr<Impl> pImpl

Implementation Class (in source)
├── All Private Data Members
├── Private Helper Methods
└── Implementation of Public Interface
```

## Key Components

### 1. Public Interface Class
- Declared in header file
- Contains only public interface and a pointer to implementation
- Minimal dependencies in header

### 2. Implementation Class
- Defined in source file (.cpp)
- Contains all private data members and methods
- Can include heavy dependencies without affecting header

### 3. Pointer to Implementation
- Usually `std::unique_ptr<Impl>`
- Manages lifetime of implementation object
- Enables forward declaration in header

## Implementation Details

### Basic Pimpl Structure
```cpp
// In header file (.h)
class MyClass {
public:
    MyClass();
    ~MyClass();
    
    // Rule of 5 for proper pimpl
    MyClass(const MyClass& other);
    MyClass& operator=(const MyClass& other);
    MyClass(MyClass&& other) noexcept;
    MyClass& operator=(MyClass&& other) noexcept;
    
    void doSomething();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

// In source file (.cpp)
class MyClass::Impl {
public:
    void doSomething() { /* implementation */ }
private:
    // All private data here
};

MyClass::MyClass() : pImpl(std::make_unique<Impl>()) {}
MyClass::~MyClass() = default;
```

### Modern C++ Features
- Use `std::unique_ptr` for automatic memory management
- Use `std::make_unique` for construction
- Implement Rule of 5 properly for copy/move semantics
- Use `= default` for compiler-generated functions where appropriate

## Implementation Patterns

### 1. Simple Pimpl
```cpp
class Simple {
    class Impl;
    std::unique_ptr<Impl> pImpl;
public:
    Simple();
    ~Simple();
    void method();
};
```

### 2. Template-based Pimpl
```cpp
template<typename Impl>
class PimplBase {
protected:
    std::unique_ptr<Impl> pImpl;
public:
    PimplBase() : pImpl(std::make_unique<Impl>()) {}
    // Rule of 5 implementation
};
```

### 3. Fast Pimpl (Stack-based)
```cpp
class FastPimpl {
    alignas(Impl) char storage[sizeof(Impl)];
    Impl* pImpl;
public:
    FastPimpl() : pImpl(new(storage) Impl()) {}
    ~FastPimpl() { pImpl->~Impl(); }
};
```

## Real-World Examples

### 1. Database Connections
- Header: Clean interface for database operations
- Implementation: Complex driver dependencies, connection pools
- Benefit: Database library changes don't require client recompilation

### 2. Network Managers
- Header: Simple send/receive interface
- Implementation: Socket libraries, SSL contexts, protocol handlers
- Benefit: Network stack updates don't affect API users

### 3. GUI Widgets
- Header: Public widget interface
- Implementation: Platform-specific rendering code
- Benefit: Cross-platform compatibility without exposing platform code

### 4. Cryptographic Libraries
- Header: Encrypt/decrypt methods
- Implementation: Complex cryptographic algorithms and state
- Benefit: Algorithm updates maintain API stability

## Advantages
1. **Compilation Speed**: Reduces header dependencies and compile times
2. **Binary Compatibility**: Implementation changes don't break ABI
3. **True Encapsulation**: Private members are truly hidden
4. **Dependency Isolation**: Heavy includes only in source files
5. **Interface Stability**: Public API remains stable across versions
6. **Memory Management**: Automatic with smart pointers

## Disadvantages
1. **Performance Overhead**: Extra indirection and heap allocation
2. **Memory Usage**: Additional pointer and separate allocation
3. **Complexity**: More complex than direct implementation
4. **Debug Difficulty**: Implementation details harder to inspect
5. **Copy Semantics**: Requires careful implementation of Rule of 5

## When to Use

### Use When:
- You have large private member variables or complex dependencies
- Binary compatibility is important (libraries, APIs)
- Compilation time is a concern
- You need to hide implementation details completely
- Private members change frequently during development

### Don't Use When:
- Performance is critical and every nanosecond matters
- The class is simple with few private members
- The class is used in performance-critical inner loops
- Copy operations are frequent and performance-sensitive
- The overhead outweighs the benefits

## Common Mistakes
1. **Forgetting Rule of 5**: Not implementing copy/move operations properly
2. **Incomplete Types**: Trying to use incomplete types where complete types are needed
3. **Exception Safety**: Not handling exceptions during construction properly
4. **Performance Assumptions**: Using pimpl where performance is critical
5. **Over-engineering**: Applying pimpl to simple classes unnecessarily

## Interview Questions & Answers

### Q: What is the Pimpl idiom and why is it useful?
**A:** Pimpl hides implementation details by moving private members to a separate class. It reduces compilation dependencies, improves compile times, and maintains binary compatibility.

### Q: How does Pimpl affect performance?
**A:** Pimpl adds one level of indirection and requires heap allocation. This has minimal impact for most applications but can matter in performance-critical code.

### Q: Why do you need to implement the destructor explicitly?
**A:** Because `std::unique_ptr<Impl>` requires a complete type to call the destructor. Forward declaration isn't sufficient for the default destructor.

### Q: How do you handle copy semantics with Pimpl?
**A:** You need to implement the Rule of 5, creating deep copies of the implementation object in copy constructor and assignment operator.

### Q: What's the difference between Pimpl and Bridge pattern?
**A:** Pimpl is primarily for compilation firewall and implementation hiding, while Bridge is for separating abstraction from implementation to support multiple implementations.

## Best Practices
1. **Use smart pointers** - Prefer `std::unique_ptr` for automatic memory management
2. **Implement Rule of 5** - Properly handle copy and move semantics
3. **Forward declare in headers** - Minimize header dependencies
4. **Use make_unique** - For exception safety during construction
5. **Document the pattern** - Make it clear when pimpl is being used
6. **Consider fast pimpl** - For performance-critical applications
7. **Profile before optimizing** - Measure the actual impact
8. **Use consistently** - Apply throughout a library for consistency

## Variations

### 1. Fast Pimpl
- Uses stack allocation instead of heap
- Better performance but less flexible
- Requires knowing implementation size

### 2. Shared Pimpl
- Uses `std::shared_ptr` for shared implementations
- Useful for flyweight-like scenarios
- More complex lifetime management

### 3. Template Pimpl
- Template base class for common pimpl functionality
- Reduces boilerplate code
- Type-safe implementation

## Related Patterns
- **Bridge**: Similar structure, different intent (abstraction vs. implementation hiding)
- **Strategy**: Can be implemented using pimpl for implementation hiding
- **State**: Often uses pimpl for state object implementation
- **Facade**: Both hide complexity, but facade simplifies interface

## Modern C++ Considerations
- Use `std::unique_ptr` for ownership
- Use `std::make_unique` for construction
- Consider `std::optional` for optional pimpl
- Use concepts (C++20) for implementation constraints
- Consider modules (C++20) as alternative to pimpl for some use cases

## Memory and Performance Tips
- Consider object pool for frequent allocation/deallocation
- Use custom allocators for specific memory requirements
- Profile memory usage and access patterns
- Consider cache locality implications
- Use stack-based pimpl for performance-critical code
