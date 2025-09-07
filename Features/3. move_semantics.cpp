#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <algorithm>
#include <chrono>

/*
 * COMPREHENSIVE C++ MOVE SEMANTICS DEMONSTRATION
 * 
 * Move semantics (C++11) is a fundamental feature that enables:
 * 1. Efficient transfer of resources instead of copying
 * 2. Perfect forwarding in template functions
 * 3. Elimination of unnecessary temporary object copies
 * 4. RAII-compliant resource management
 * 5. Performance optimization for expensive-to-copy objects
 */

// =============================================================================
// 1. BASIC MOVE SEMANTICS DEMONSTRATION
// =============================================================================

class SimpleResource {
private:
    int* data;
    size_t size;
    std::string name;

public:
    // Constructor
    SimpleResource(const std::string& n, size_t s) 
        : size(s), name(n) {
        data = new int[size];
        for (size_t i = 0; i < size; ++i) {
            data[i] = static_cast<int>(i);
        }
        std::cout << "SimpleResource '" << name << "' constructed with " << size << " elements\n";
    }
    
    // Copy constructor
    SimpleResource(const SimpleResource& other) 
        : size(other.size), name(other.name + "_copy") {
        data = new int[size];
        std::copy(other.data, other.data + size, data);
        std::cout << "SimpleResource '" << name << "' COPIED from '" << other.name << "'\n";
    }
    
    // Move constructor
    SimpleResource(SimpleResource&& other) noexcept 
        : data(other.data), size(other.size), name(std::move(other.name) + "_moved") {
        // Transfer ownership
        other.data = nullptr;
        other.size = 0;
        std::cout << "SimpleResource '" << name << "' MOVED (no copying!)\n";
    }
    
    // Copy assignment operator
    SimpleResource& operator=(const SimpleResource& other) {
        if (this != &other) {
            delete[] data;
            
            size = other.size;
            name = other.name + "_copy_assigned";
            data = new int[size];
            std::copy(other.data, other.data + size, data);
            std::cout << "SimpleResource '" << name << "' COPY ASSIGNED\n";
        }
        return *this;
    }
    
    // Move assignment operator
    SimpleResource& operator=(SimpleResource&& other) noexcept {
        if (this != &other) {
            delete[] data;
            
            // Transfer ownership
            data = other.data;
            size = other.size;
            name = std::move(other.name) + "_move_assigned";
            
            // Reset other
            other.data = nullptr;
            other.size = 0;
            std::cout << "SimpleResource '" << name << "' MOVE ASSIGNED\n";
        }
        return *this;
    }
    
    // Destructor
    ~SimpleResource() {
        delete[] data;
        if (!name.empty()) {
            std::cout << "SimpleResource '" << name << "' destroyed\n";
        }
    }
    
    // Utility methods
    void print() const {
        std::cout << "Resource '" << name << "' has " << size << " elements: ";
        if (data && size > 0) {
            for (size_t i = 0; i < std::min(size, size_t(5)); ++i) {
                std::cout << data[i] << " ";
            }
            if (size > 5) std::cout << "...";
        } else {
            std::cout << "(empty/moved)";
        }
        std::cout << std::endl;
    }
    
    size_t getSize() const { return size; }
    const std::string& getName() const { return name; }
};

void demonstrateBasicMoveSemantics() {
    std::cout << "\n========== BASIC MOVE SEMANTICS ==========\n";
    
    // Create original resource
    SimpleResource original("original", 5);
    original.print();
    
    std::cout << "\n--- Copy vs Move Comparison ---\n";
    
    // Copy constructor
    SimpleResource copied = original;
    copied.print();
    original.print();  // Original unchanged
    
    // Move constructor
    SimpleResource moved = std::move(original);
    moved.print();
    original.print();  // Original is now empty
    
    std::cout << "\n--- Assignment Operations ---\n";
    
    // Create resources for assignment
    SimpleResource res1("res1", 3);
    SimpleResource res2("res2", 4);
    
    // Copy assignment
    SimpleResource copy_target("copy_target", 1);
    copy_target = res1;  // Copy assignment
    copy_target.print();
    res1.print();  // res1 unchanged
    
    // Move assignment
    SimpleResource move_target("move_target", 1);
    move_target = std::move(res2);  // Move assignment
    move_target.print();
    res2.print();  // res2 is now empty
}

// =============================================================================
// 2. RVALUE REFERENCES AND PERFECT FORWARDING
// =============================================================================

void demonstrateRvalueReferences() {
    std::cout << "\n========== RVALUE REFERENCES ==========\n";
    
    std::cout << "\n--- Understanding Lvalues and Rvalues ---\n";
    
    int x = 10;
    int& lvalue_ref = x;        // Lvalue reference
    // int& invalid = 10;       // Error: can't bind lvalue reference to rvalue
    
    int&& rvalue_ref = 10;      // Rvalue reference to literal
    int&& rvalue_ref2 = std::move(x);  // Rvalue reference to moved lvalue
    
    std::cout << "x = " << x << std::endl;
    std::cout << "lvalue_ref = " << lvalue_ref << std::endl;
    std::cout << "rvalue_ref = " << rvalue_ref << std::endl;
    std::cout << "rvalue_ref2 = " << rvalue_ref2 << std::endl;
    
    // Note: rvalue references are lvalues when used in expressions
    // int&& another = rvalue_ref;  // Error: rvalue_ref is an lvalue here
    int&& another = std::move(rvalue_ref);  // OK: explicitly move
    
    std::cout << "\n--- Function Overloading with References ---\n";
    
    auto process = [](const std::string& s) {
        std::cout << "Processing lvalue: " << s << std::endl;
    };
    
    auto process_rvalue = [](std::string&& s) {
        std::cout << "Processing rvalue: " << s << " (can modify)" << std::endl;
        s += "_modified";
        std::cout << "Modified to: " << s << std::endl;
    };
    
    std::string str = "hello";
    process(str);                    // Calls lvalue version
    process_rvalue(std::move(str));  // Calls rvalue version
    std::cout << "str after move: '" << str << "'\n";  // str is in valid but unspecified state
}

// Perfect forwarding template
template<typename T>
void perfect_forward(T&& arg) {
    std::cout << "perfect_forward called with: ";
    
    // Check if T is lvalue reference
    if constexpr (std::is_lvalue_reference_v<T>) {
        std::cout << "lvalue reference\n";
    } else {
        std::cout << "rvalue reference\n";
    }
    
    // Forward to another function
    auto target = [](const std::string& s) {
        std::cout << "  target received lvalue: " << s << std::endl;
    };
    
    auto target_rvalue = [](std::string&& s) {
        std::cout << "  target received rvalue: " << s << std::endl;
    };
    
    // Perfect forwarding preserves value category
    if constexpr (std::is_lvalue_reference_v<T>) {
        target(arg);
    } else {
        target_rvalue(std::forward<T>(arg));
    }
}

void demonstratePerfectForwarding() {
    std::cout << "\n========== PERFECT FORWARDING ==========\n";
    
    std::string str = "test";
    
    perfect_forward(str);                    // Lvalue
    perfect_forward(std::move(str));         // Rvalue
    perfect_forward(std::string("temp"));    // Rvalue
}

// =============================================================================
// 3. MOVE-AWARE CONTAINER
// =============================================================================

template<typename T>
class MoveAwareVector {
private:
    T* data;
    size_t size;
    size_t capacity;
    
    void reallocate(size_t new_capacity) {
        T* new_data = static_cast<T*>(std::malloc(new_capacity * sizeof(T)));
        
        // Move construct elements to new location
        for (size_t i = 0; i < size; ++i) {
            new (new_data + i) T(std::move(data[i]));
            data[i].~T();  // Destroy old object
        }
        
        std::free(data);
        data = new_data;
        capacity = new_capacity;
        std::cout << "Reallocated to capacity " << capacity << " using move semantics\n";
    }

public:
    MoveAwareVector() : data(nullptr), size(0), capacity(0) {}
    
    // Copy constructor
    MoveAwareVector(const MoveAwareVector& other) 
        : size(other.size), capacity(other.capacity) {
        data = static_cast<T*>(std::malloc(capacity * sizeof(T)));
        for (size_t i = 0; i < size; ++i) {
            new (data + i) T(other.data[i]);  // Copy construct
        }
        std::cout << "MoveAwareVector copied\n";
    }
    
    // Move constructor
    MoveAwareVector(MoveAwareVector&& other) noexcept
        : data(other.data), size(other.size), capacity(other.capacity) {
        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
        std::cout << "MoveAwareVector moved\n";
    }
    
    // Copy assignment
    MoveAwareVector& operator=(const MoveAwareVector& other) {
        if (this != &other) {
            clear();
            std::free(data);
            
            size = other.size;
            capacity = other.capacity;
            data = static_cast<T*>(std::malloc(capacity * sizeof(T)));
            
            for (size_t i = 0; i < size; ++i) {
                new (data + i) T(other.data[i]);
            }
            std::cout << "MoveAwareVector copy assigned\n";
        }
        return *this;
    }
    
    // Move assignment
    MoveAwareVector& operator=(MoveAwareVector&& other) noexcept {
        if (this != &other) {
            clear();
            std::free(data);
            
            data = other.data;
            size = other.size;
            capacity = other.capacity;
            
            other.data = nullptr;
            other.size = 0;
            other.capacity = 0;
            std::cout << "MoveAwareVector move assigned\n";
        }
        return *this;
    }
    
    ~MoveAwareVector() {
        clear();
        std::free(data);
    }
    
    void push_back(const T& value) {
        if (size >= capacity) {
            reallocate(capacity == 0 ? 1 : capacity * 2);
        }
        new (data + size) T(value);
        ++size;
    }
    
    void push_back(T&& value) {
        if (size >= capacity) {
            reallocate(capacity == 0 ? 1 : capacity * 2);
        }
        new (data + size) T(std::move(value));
        ++size;
        std::cout << "Element moved into vector\n";
    }
    
    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (size >= capacity) {
            reallocate(capacity == 0 ? 1 : capacity * 2);
        }
        new (data + size) T(std::forward<Args>(args)...);
        ++size;
        std::cout << "Element emplaced (constructed in-place)\n";
    }
    
    void clear() {
        for (size_t i = 0; i < size; ++i) {
            data[i].~T();
        }
        size = 0;
    }
    
    size_t getSize() const { return size; }
    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }
};

void demonstrateMoveAwareContainer() {
    std::cout << "\n========== MOVE-AWARE CONTAINER ==========\n";
    
    MoveAwareVector<SimpleResource> vec;
    
    std::cout << "\n--- Adding elements ---\n";
    
    // Copy semantics
    SimpleResource res1("element1", 3);
    vec.push_back(res1);  // Copy
    
    // Move semantics
    SimpleResource res2("element2", 4);
    vec.push_back(std::move(res2));  // Move
    
    // Emplace (construct in place)
    vec.emplace_back("element3", 5);  // No copy or move!
    
    std::cout << "\n--- Container operations ---\n";
    
    // Move entire container
    MoveAwareVector<SimpleResource> vec2 = std::move(vec);
    std::cout << "Moved entire container\n";
    
    std::cout << "vec size after move: " << vec.getSize() << std::endl;
    std::cout << "vec2 size: " << vec2.getSize() << std::endl;
}

// =============================================================================
// 4. RETURN VALUE OPTIMIZATION (RVO) AND MOVE
// =============================================================================

SimpleResource createResource(const std::string& name, size_t size) {
    std::cout << "Creating resource in function\n";
    return SimpleResource(name, size);  // RVO: no move needed
}

SimpleResource createAndModifyResource(const std::string& name, size_t size) {
    std::cout << "Creating and modifying resource in function\n";
    SimpleResource res(name, size);
    // Some modifications...
    return res;  // NRVO: named return value optimization
}

SimpleResource conditionalReturn(bool condition) {
    if (condition) {
        return SimpleResource("conditional_true", 3);
    } else {
        return SimpleResource("conditional_false", 5);
    }
    // No RVO here due to multiple return paths, but move is used
}

void demonstrateRVO() {
    std::cout << "\n========== RETURN VALUE OPTIMIZATION ==========\n";
    
    std::cout << "\n--- RVO (Return Value Optimization) ---\n";
    auto res1 = createResource("rvo_test", 4);
    res1.print();
    
    std::cout << "\n--- NRVO (Named Return Value Optimization) ---\n";
    auto res2 = createAndModifyResource("nrvo_test", 3);
    res2.print();
    
    std::cout << "\n--- No RVO (Move used instead) ---\n";
    auto res3 = conditionalReturn(true);
    res3.print();
}

// =============================================================================
// 5. MOVE SEMANTICS WITH STL CONTAINERS
// =============================================================================

void demonstrateSTLMoveSemantics() {
    std::cout << "\n========== STL MOVE SEMANTICS ==========\n";
    
    std::cout << "\n--- Vector with move semantics ---\n";
    std::vector<SimpleResource> vec;
    
    // Reserve space to avoid reallocations
    vec.reserve(3);
    
    // Emplace back (construct in place)
    vec.emplace_back("vec_element1", 2);
    
    // Move into vector
    SimpleResource temp("temp_resource", 3);
    vec.push_back(std::move(temp));
    
    std::cout << "\n--- Moving entire containers ---\n";
    std::vector<SimpleResource> vec2 = std::move(vec);
    std::cout << "Moved entire vector\n";
    std::cout << "Original vector size: " << vec.size() << std::endl;
    std::cout << "New vector size: " << vec2.size() << std::endl;
    
    std::cout << "\n--- String move semantics ---\n";
    std::string str1 = "Hello, World!";
    std::string str2 = std::move(str1);
    
    std::cout << "str1 after move: '" << str1 << "'\n";
    std::cout << "str2: '" << str2 << "'\n";
    
    std::cout << "\n--- Unique_ptr move semantics ---\n";
    auto ptr1 = std::make_unique<SimpleResource>("unique_resource", 4);
    auto ptr2 = std::move(ptr1);  // Transfer ownership
    
    std::cout << "ptr1 is " << (ptr1 ? "valid" : "null") << std::endl;
    std::cout << "ptr2 is " << (ptr2 ? "valid" : "null") << std::endl;
    
    if (ptr2) {
        ptr2->print();
    }
}

// =============================================================================
// 6. PERFORMANCE COMPARISON
// =============================================================================

void demonstratePerformance() {
    std::cout << "\n========== PERFORMANCE COMPARISON ==========\n";
    
    const size_t NUM_ELEMENTS = 1000;
    const size_t ELEMENT_SIZE = 1000;
    
    // Test copy vs move performance
    std::vector<SimpleResource> resources;
    resources.reserve(NUM_ELEMENTS);
    
    // Create test resources
    for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
        resources.emplace_back("perf_test_" + std::to_string(i), ELEMENT_SIZE);
    }
    
    std::cout << "\n--- Performance Test: Copy vs Move ---\n";
    
    // Test copying
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<SimpleResource> copied_vec;
    copied_vec.reserve(NUM_ELEMENTS);
    
    for (const auto& res : resources) {
        copied_vec.push_back(res);  // Copy
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto copy_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Copy time: " << copy_time.count() << " ms\n";
    
    // Test moving
    start = std::chrono::high_resolution_clock::now();
    std::vector<SimpleResource> moved_vec;
    moved_vec.reserve(NUM_ELEMENTS);
    
    for (auto&& res : resources) {
        moved_vec.push_back(std::move(res));  // Move
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto move_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Move time: " << move_time.count() << " ms\n";
    std::cout << "Performance improvement: " 
              << (copy_time.count() > 0 ? (double)copy_time.count() / move_time.count() : 0) 
              << "x faster\n";
}

// =============================================================================
// 7. COMMON PITFALLS AND BEST PRACTICES
// =============================================================================

void demonstratePitfalls() {
    std::cout << "\n========== COMMON PITFALLS ==========\n";
    
    std::cout << "\n--- Pitfall 1: Using moved objects ---\n";
    SimpleResource res("original", 3);
    SimpleResource moved_res = std::move(res);
    
    // res is now in a valid but unspecified state
    std::cout << "Moved object name: '" << res.getName() << "'\n";
    std::cout << "Moved object size: " << res.getSize() << std::endl;
    // Using res further is dangerous except for assignment or destruction
    
    std::cout << "\n--- Pitfall 2: Unnecessary std::move ---\n";
    auto create_bad = []() {
        std::string local = "local string";
        return std::move(local);  // BAD: prevents RVO
    };
    
    auto create_good = []() {
        std::string local = "local string";
        return local;  // GOOD: enables RVO
    };
    
    auto str1 = create_bad();
    auto str2 = create_good();
    std::cout << "Both work, but create_good is better\n";
    
    std::cout << "\n--- Pitfall 3: Moving const objects ---\n";
    const SimpleResource const_res("const_resource", 2);
    // SimpleResource moved_const = std::move(const_res);  // Copies instead of moving!
    std::cout << "std::move on const objects results in copy, not move\n";
    
    std::cout << "\n--- Best Practice: Rule of Five ---\n";
    std::cout << "If you implement one of: destructor, copy constructor, copy assignment,\n";
    std::cout << "move constructor, move assignment - consider implementing all five\n";
}

// =============================================================================
// 8. MOVE SEMANTICS IN ALGORITHMS
// =============================================================================

void demonstrateMoveInAlgorithms() {
    std::cout << "\n========== MOVE SEMANTICS IN ALGORITHMS ==========\n";
    
    std::vector<SimpleResource> vec;
    vec.reserve(5);
    
    // Create some resources
    for (int i = 0; i < 5; ++i) {
        vec.emplace_back("algo_test_" + std::to_string(i), i + 1);
    }
    
    std::cout << "\n--- std::move in algorithms ---\n";
    
    // Move elements to another container
    std::vector<SimpleResource> destination;
    destination.reserve(5);
    
    std::transform(
        std::make_move_iterator(vec.begin()),
        std::make_move_iterator(vec.end()),
        std::back_inserter(destination),
        [](SimpleResource&& res) {
            return std::move(res);  // Explicit move
        }
    );
    
    std::cout << "After moving with algorithm:\n";
    std::cout << "Source vector size: " << vec.size() << std::endl;
    std::cout << "Destination vector size: " << destination.size() << std::endl;
    
    // Check if source objects were moved from
    for (const auto& res : vec) {
        std::cout << "Source element: ";
        res.print();
    }
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main() {
    std::cout << "COMPREHENSIVE C++ MOVE SEMANTICS DEMONSTRATION\n";
    std::cout << "==============================================\n";
    
    demonstrateBasicMoveSemantics();
    demonstrateRvalueReferences();
    demonstratePerfectForwarding();
    demonstrateMoveAwareContainer();
    demonstrateRVO();
    demonstrateSTLMoveSemantics();
    demonstratePerformance();
    demonstratePitfalls();
    demonstrateMoveInAlgorithms();
    
    std::cout << "\n========== SUMMARY ==========\n";
    std::cout << "Move semantics provides:\n";
    std::cout << "1. Efficient resource transfer without copying\n";
    std::cout << "2. Perfect forwarding for generic code\n";
    std::cout << "3. Elimination of unnecessary temporary copies\n";
    std::cout << "4. Better performance for expensive-to-copy objects\n";
    std::cout << "5. More expressive code with clear ownership transfer\n";
    std::cout << "\nKey principles:\n";
    std::cout << "- Use std::move for explicit ownership transfer\n";
    std::cout << "- Implement Rule of Five for resource-managing classes\n";
    std::cout << "- Prefer emplace over push when possible\n";
    std::cout << "- Don't use std::move on return values (prefer RVO)\n";
    std::cout << "- Don't use moved-from objects (except for assignment)\n";
    
    return 0;
}
