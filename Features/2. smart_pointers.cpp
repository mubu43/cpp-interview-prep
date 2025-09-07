#include <iostream>
#include <memory>
#include <vector>
#include <string>

/*
 * SMART POINTERS IN C++
 * 
 * Smart pointers are objects that act like traditional pointers but provide
 * automatic memory management. They help prevent memory leaks, dangling pointers,
 * and other memory-related issues.
 * 
 * Main types:
 * 1. unique_ptr - Exclusive ownership
 * 2. shared_ptr - Shared ownership with reference counting
 * 3. weak_ptr - Non-owning observer (breaks circular references)
 */

// Demo class for examples
class Resource {
private:
    std::string name;
    static int count;

public:
    Resource(const std::string& n) : name(n) {
        count++;
        std::cout << "Resource '" << name << "' created. Total count: " << count << std::endl;
    }
    
    ~Resource() {
        count--;
        std::cout << "Resource '" << name << "' destroyed. Remaining count: " << count << std::endl;
    }
    
    void doWork() {
        std::cout << "Resource '" << name << "' is working..." << std::endl;
    }
    
    const std::string& getName() const { return name; }
    static int getCount() { return count; }
};

int Resource::count = 0;

// =============================================================================
// 1. UNIQUE_PTR DEMONSTRATION
// =============================================================================

void demonstrateUniquePtr() {
    std::cout << "\n========== UNIQUE_PTR DEMONSTRATION ==========\n";
    
    // Creating unique_ptr
    std::unique_ptr<Resource> ptr1 = std::make_unique<Resource>("UniqueResource1");
    
    // Using the pointer
    ptr1->doWork();
    std::cout << "Resource name: " << ptr1->getName() << std::endl;
    
    // Transfer ownership (move semantics)
    std::unique_ptr<Resource> ptr2 = std::move(ptr1);
    
    // ptr1 is now null
    if (!ptr1) {
        std::cout << "ptr1 is now null after move\n";
    }
    
    // ptr2 owns the resource
    ptr2->doWork();
    
    // Creating array with unique_ptr
    std::unique_ptr<int[]> arr = std::make_unique<int[]>(5);
    for (int i = 0; i < 5; ++i) {
        arr[i] = i * 10;
    }
    
    std::cout << "Array elements: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
    
    // Custom deleter example
    auto customDeleter = [](Resource* r) {
        std::cout << "Custom deleter called for: " << r->getName() << std::endl;
        delete r;
    };
    
    std::unique_ptr<Resource, decltype(customDeleter)> ptrWithCustomDeleter(
        new Resource("CustomDeleterResource"), customDeleter);
    
    // Automatic cleanup when going out of scope
    std::cout << "End of unique_ptr demonstration\n";
}

// =============================================================================
// 2. SHARED_PTR DEMONSTRATION
// =============================================================================

void demonstrateSharedPtr() {
    std::cout << "\n========== SHARED_PTR DEMONSTRATION ==========\n";
    
    // Creating shared_ptr
    std::shared_ptr<Resource> ptr1 = std::make_shared<Resource>("SharedResource1");
    std::cout << "Reference count after ptr1 creation: " << ptr1.use_count() << std::endl;
    
    {
        // Copy shared_ptr (increases reference count)
        std::shared_ptr<Resource> ptr2 = ptr1;
        std::cout << "Reference count after ptr2 copy: " << ptr1.use_count() << std::endl;
        
        // Another copy
        std::shared_ptr<Resource> ptr3 = ptr1;
        std::cout << "Reference count after ptr3 copy: " << ptr1.use_count() << std::endl;
        
        // All pointers point to the same resource
        ptr1->doWork();
        ptr2->doWork();
        ptr3->doWork();
        
        // ptr2 and ptr3 go out of scope here
    }
    
    std::cout << "Reference count after inner scope: " << ptr1.use_count() << std::endl;
    
    // Reset a shared_ptr
    ptr1.reset();
    std::cout << "ptr1 reset, resource should be destroyed\n";
    
    // Shared ownership example with vector
    std::vector<std::shared_ptr<Resource>> resourceVector;
    
    auto sharedRes = std::make_shared<Resource>("VectorResource");
    resourceVector.push_back(sharedRes);
    resourceVector.push_back(sharedRes);  // Same resource, shared
    
    std::cout << "Reference count in vector: " << sharedRes.use_count() << std::endl;
    
    std::cout << "End of shared_ptr demonstration\n";
}

// =============================================================================
// 3. WEAK_PTR DEMONSTRATION
// =============================================================================

class Node {
public:
    std::string data;
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> parent;  // Use weak_ptr to break circular reference
    
    Node(const std::string& d) : data(d) {
        std::cout << "Node '" << data << "' created\n";
    }
    
    ~Node() {
        std::cout << "Node '" << data << "' destroyed\n";
    }
};

void demonstrateWeakPtr() {
    std::cout << "\n========== WEAK_PTR DEMONSTRATION ==========\n";
    
    std::shared_ptr<Resource> shared = std::make_shared<Resource>("WeakPtrResource");
    std::cout << "Shared reference count: " << shared.use_count() << std::endl;
    
    // Create weak_ptr from shared_ptr
    std::weak_ptr<Resource> weak = shared;
    std::cout << "Shared reference count after weak_ptr creation: " << shared.use_count() << std::endl;
    
    // Check if weak_ptr is valid and lock it
    if (auto locked = weak.lock()) {
        std::cout << "weak_ptr is valid, using resource:\n";
        locked->doWork();
        std::cout << "Reference count while locked: " << shared.use_count() << std::endl;
    }
    
    // Reset shared_ptr
    shared.reset();
    std::cout << "shared_ptr reset\n";
    
    // Try to lock weak_ptr after resource is destroyed
    if (auto locked = weak.lock()) {
        std::cout << "This shouldn't print - resource is gone\n";
    } else {
        std::cout << "weak_ptr is expired - resource has been destroyed\n";
    }
    
    // Demonstrate breaking circular references
    std::cout << "\n--- Circular Reference Example ---\n";
    auto parent = std::make_shared<Node>("Parent");
    auto child = std::make_shared<Node>("Child");
    
    parent->next = child;
    child->parent = parent;  // weak_ptr prevents circular reference
    
    std::cout << "Parent reference count: " << parent.use_count() << std::endl;
    std::cout << "Child reference count: " << child.use_count() << std::endl;
    
    // Even with circular structure, resources will be properly cleaned up
    std::cout << "End of weak_ptr demonstration\n";
}

// =============================================================================
// 4. SMART POINTER BEST PRACTICES
// =============================================================================

void demonstrateBestPractices() {
    std::cout << "\n========== BEST PRACTICES DEMONSTRATION ==========\n";
    
    // 1. Prefer make_unique and make_shared
    auto good_unique = std::make_unique<Resource>("GoodUnique");
    auto good_shared = std::make_shared<Resource>("GoodShared");
    
    // 2. Use unique_ptr by default, shared_ptr only when needed
    std::unique_ptr<Resource> defaultChoice = std::make_unique<Resource>("DefaultChoice");
    
    // 3. Pass smart pointers correctly
    auto passToFunction = [](const std::shared_ptr<Resource>& res) {
        // Pass by const reference when you don't need to modify ownership
        res->doWork();
    };
    
    passToFunction(good_shared);
    
    // 4. Return smart pointers from factory functions
    auto factory = []() -> std::unique_ptr<Resource> {
        return std::make_unique<Resource>("FactoryCreated");
    };
    
    auto factoryProduct = factory();
    factoryProduct->doWork();
    
    // 5. Use weak_ptr for observer patterns
    std::shared_ptr<Resource> observed = std::make_shared<Resource>("Observed");
    std::weak_ptr<Resource> observer = observed;
    
    // Observer can check if the observed object still exists
    if (auto obj = observer.lock()) {
        std::cout << "Observer can access: " << obj->getName() << std::endl;
    }
    
    std::cout << "End of best practices demonstration\n";
}

// =============================================================================
// 5. PERFORMANCE COMPARISON
// =============================================================================

void performanceComparison() {
    std::cout << "\n========== PERFORMANCE NOTES ==========\n";
    
    std::cout << "Performance characteristics:\n";
    std::cout << "1. unique_ptr: Almost zero overhead compared to raw pointers\n";
    std::cout << "2. shared_ptr: Small overhead due to reference counting\n";
    std::cout << "3. weak_ptr: Minimal overhead, no ownership cost\n";
    std::cout << "4. make_shared is more efficient than shared_ptr(new T())\n";
    
    // Demonstrate the difference
    {
        // Less efficient - two allocations
        std::shared_ptr<Resource> ptr1(new Resource("TwoAllocations"));
        
        // More efficient - single allocation
        auto ptr2 = std::make_shared<Resource>("OneAllocation");
    }
    
    std::cout << "Use make_shared for better performance and exception safety\n";
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main() {
    std::cout << "COMPREHENSIVE SMART POINTERS DEMONSTRATION\n";
    std::cout << "==========================================\n";
    
    demonstrateUniquePtr();
    demonstrateSharedPtr();
    demonstrateWeakPtr();
    demonstrateBestPractices();
    performanceComparison();
    
    std::cout << "\n========== SUMMARY ==========\n";
    std::cout << "Final resource count: " << Resource::getCount() << std::endl;
    std::cout << "All resources properly cleaned up!\n";
    
    std::cout << "\nKey Takeaways:\n";
    std::cout << "1. Use unique_ptr for exclusive ownership\n";
    std::cout << "2. Use shared_ptr for shared ownership\n";
    std::cout << "3. Use weak_ptr to break circular references\n";
    std::cout << "4. Prefer make_unique and make_shared\n";
    std::cout << "5. Smart pointers provide automatic memory management\n";
    
    return 0;
}