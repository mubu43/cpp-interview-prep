#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <priority_queue>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <string>
#include <functional>
#include <memory>
#include <chrono>
#include <random>

/*
 * COMPREHENSIVE C++ STL DEMONSTRATION
 * 
 * The Standard Template Library (STL) is a powerful collection of:
 * 1. Containers - Data structures (vector, map, set, etc.)
 * 2. Iterators - Objects that traverse containers
 * 3. Algorithms - Functions that operate on containers
 * 4. Function Objects - Callable objects for customization
 * 5. Allocators - Memory management objects
 */

// =============================================================================
// 1. SEQUENCE CONTAINERS DEMONSTRATION
// =============================================================================

void demonstrateSequenceContainers() {
    std::cout << "\n========== SEQUENCE CONTAINERS ==========\n";
    
    // VECTOR - Dynamic array
    std::cout << "\n--- VECTOR (Dynamic Array) ---\n";
    std::vector<int> vec = {1, 2, 3, 4, 5};
    vec.push_back(6);
    vec.insert(vec.begin() + 2, 99);  // Insert at position 2
    
    std::cout << "Vector: ";
    for (const auto& val : vec) {
        std::cout << val << " ";
    }
    std::cout << "\nSize: " << vec.size() << ", Capacity: " << vec.capacity() << std::endl;
    
    // Random access
    std::cout << "Element at index 3: " << vec[3] << std::endl;
    std::cout << "Element at index 3 (safe): " << vec.at(3) << std::endl;
    
    // DEQUE - Double-ended queue
    std::cout << "\n--- DEQUE (Double-ended Queue) ---\n";
    std::deque<std::string> deq = {"middle"};
    deq.push_front("front");
    deq.push_back("back");
    deq.push_front("very_front");
    
    std::cout << "Deque: ";
    for (const auto& str : deq) {
        std::cout << str << " ";
    }
    std::cout << std::endl;
    
    // LIST - Doubly linked list
    std::cout << "\n--- LIST (Doubly Linked List) ---\n";
    std::list<double> lst = {3.14, 2.71, 1.41};
    lst.push_front(0.577);
    lst.push_back(1.618);
    lst.sort();  // List has its own sort method
    
    std::cout << "Sorted List: ";
    for (const auto& val : lst) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    // Remove specific value
    lst.remove(2.71);
    std::cout << "After removing 2.71: ";
    for (const auto& val : lst) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 2. ASSOCIATIVE CONTAINERS DEMONSTRATION
// =============================================================================

void demonstrateAssociativeContainers() {
    std::cout << "\n========== ASSOCIATIVE CONTAINERS ==========\n";
    
    // SET - Ordered unique elements
    std::cout << "\n--- SET (Ordered Unique Elements) ---\n";
    std::set<int> s = {5, 2, 8, 2, 1, 9, 5};  // Duplicates will be removed
    s.insert(3);
    s.insert(7);
    
    std::cout << "Set: ";
    for (const auto& val : s) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    // Find operations
    auto it = s.find(5);
    if (it != s.end()) {
        std::cout << "Found 5 in set\n";
    }
    
    std::cout << "Count of 2: " << s.count(2) << std::endl;
    std::cout << "Lower bound of 4: " << *s.lower_bound(4) << std::endl;
    std::cout << "Upper bound of 4: " << *s.upper_bound(4) << std::endl;
    
    // MULTISET - Ordered elements (allows duplicates)
    std::cout << "\n--- MULTISET (Allows Duplicates) ---\n";
    std::multiset<char> ms = {'a', 'b', 'a', 'c', 'b', 'a'};
    
    std::cout << "Multiset: ";
    for (const auto& ch : ms) {
        std::cout << ch << " ";
    }
    std::cout << "\nCount of 'a': " << ms.count('a') << std::endl;
    
    // MAP - Key-value pairs (ordered)
    std::cout << "\n--- MAP (Key-Value Pairs) ---\n";
    std::map<std::string, int> grades = {
        {"Alice", 95},
        {"Bob", 87},
        {"Charlie", 92}
    };
    
    grades["David"] = 88;  // Insert
    grades["Alice"] = 97;  // Update
    
    std::cout << "Grades:\n";
    for (const auto& pair : grades) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    
    // Safe access
    if (grades.find("Eve") != grades.end()) {
        std::cout << "Eve's grade: " << grades["Eve"] << std::endl;
    } else {
        std::cout << "Eve not found in grades\n";
    }
    
    // MULTIMAP - Multiple values per key
    std::cout << "\n--- MULTIMAP (Multiple Values per Key) ---\n";
    std::multimap<std::string, std::string> courses = {
        {"Alice", "Math"},
        {"Alice", "Physics"},
        {"Bob", "Chemistry"},
        {"Alice", "Computer Science"}
    };
    
    std::cout << "Alice's courses: ";
    auto range = courses.equal_range("Alice");
    for (auto it = range.first; it != range.second; ++it) {
        std::cout << it->second << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// 3. UNORDERED CONTAINERS DEMONSTRATION
// =============================================================================

void demonstrateUnorderedContainers() {
    std::cout << "\n========== UNORDERED CONTAINERS (Hash-based) ==========\n";
    
    // UNORDERED_SET - Hash set
    std::cout << "\n--- UNORDERED_SET (Hash Set) ---\n";
    std::unordered_set<std::string> words = {"hello", "world", "cpp", "stl"};
    words.insert("programming");
    words.insert("hello");  // Duplicate, won't be added
    
    std::cout << "Words: ";
    for (const auto& word : words) {
        std::cout << word << " ";
    }
    std::cout << "\nSize: " << words.size() << std::endl;
    std::cout << "Bucket count: " << words.bucket_count() << std::endl;
    std::cout << "Load factor: " << words.load_factor() << std::endl;
    
    // UNORDERED_MAP - Hash map
    std::cout << "\n--- UNORDERED_MAP (Hash Map) ---\n";
    std::unordered_map<std::string, int> wordCount = {
        {"the", 10},
        {"and", 8},
        {"of", 6}
    };
    
    wordCount["in"] = 5;
    wordCount["to"] = 7;
    
    std::cout << "Word frequencies:\n";
    for (const auto& pair : wordCount) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    
    // Performance comparison note
    std::cout << "\nPerformance: O(1) average for hash containers vs O(log n) for ordered\n";
}

// =============================================================================
// 4. CONTAINER ADAPTORS DEMONSTRATION
// =============================================================================

void demonstrateContainerAdaptors() {
    std::cout << "\n========== CONTAINER ADAPTORS ==========\n";
    
    // STACK - LIFO (Last In, First Out)
    std::cout << "\n--- STACK (LIFO) ---\n";
    std::stack<int> stk;
    for (int i = 1; i <= 5; ++i) {
        stk.push(i);
    }
    
    std::cout << "Stack elements (popping): ";
    while (!stk.empty()) {
        std::cout << stk.top() << " ";
        stk.pop();
    }
    std::cout << std::endl;
    
    // QUEUE - FIFO (First In, First Out)
    std::cout << "\n--- QUEUE (FIFO) ---\n";
    std::queue<std::string> q;
    q.push("First");
    q.push("Second");
    q.push("Third");
    
    std::cout << "Queue elements (dequeuing): ";
    while (!q.empty()) {
        std::cout << q.front() << " ";
        q.pop();
    }
    std::cout << std::endl;
    
    // PRIORITY_QUEUE - Max heap by default
    std::cout << "\n--- PRIORITY_QUEUE (Max Heap) ---\n";
    std::priority_queue<int> pq;
    std::vector<int> nums = {3, 1, 4, 1, 5, 9, 2, 6};
    
    for (int num : nums) {
        pq.push(num);
    }
    
    std::cout << "Priority queue (max heap): ";
    while (!pq.empty()) {
        std::cout << pq.top() << " ";
        pq.pop();
    }
    std::cout << std::endl;
    
    // Min heap using custom comparator
    std::cout << "\n--- MIN HEAP (Custom Comparator) ---\n";
    std::priority_queue<int, std::vector<int>, std::greater<int>> minPq;
    
    for (int num : nums) {
        minPq.push(num);
    }
    
    std::cout << "Min heap: ";
    while (!minPq.empty()) {
        std::cout << minPq.top() << " ";
        minPq.pop();
    }
    std::cout << std::endl;
}

// =============================================================================
// 5. ALGORITHMS DEMONSTRATION
// =============================================================================

void demonstrateAlgorithms() {
    std::cout << "\n========== STL ALGORITHMS ==========\n";
    
    std::vector<int> vec = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    std::vector<int> original = vec;
    
    // SORTING
    std::cout << "\n--- SORTING ALGORITHMS ---\n";
    std::cout << "Original: ";
    for (int val : original) std::cout << val << " ";
    std::cout << std::endl;
    
    std::sort(vec.begin(), vec.end());
    std::cout << "Sorted: ";
    for (int val : vec) std::cout << val << " ";
    std::cout << std::endl;
    
    // Partial sort
    vec = original;
    std::partial_sort(vec.begin(), vec.begin() + 3, vec.end());
    std::cout << "Partial sort (first 3): ";
    for (int val : vec) std::cout << val << " ";
    std::cout << std::endl;
    
    // SEARCHING
    std::cout << "\n--- SEARCHING ALGORITHMS ---\n";
    vec = {1, 2, 3, 4, 5, 6, 7, 8, 9};  // Sorted for binary search
    
    // Binary search
    bool found = std::binary_search(vec.begin(), vec.end(), 5);
    std::cout << "Binary search for 5: " << (found ? "Found" : "Not found") << std::endl;
    
    // Lower bound
    auto lb = std::lower_bound(vec.begin(), vec.end(), 5);
    std::cout << "Lower bound of 5: index " << (lb - vec.begin()) << std::endl;
    
    // Find
    auto it = std::find(vec.begin(), vec.end(), 7);
    if (it != vec.end()) {
        std::cout << "Found 7 at index: " << (it - vec.begin()) << std::endl;
    }
    
    // MODIFYING ALGORITHMS
    std::cout << "\n--- MODIFYING ALGORITHMS ---\n";
    std::vector<int> source = {1, 2, 3, 4, 5};
    std::vector<int> dest(5);
    
    // Copy
    std::copy(source.begin(), source.end(), dest.begin());
    std::cout << "Copied: ";
    for (int val : dest) std::cout << val << " ";
    std::cout << std::endl;
    
    // Transform
    std::transform(source.begin(), source.end(), dest.begin(), 
                   [](int x) { return x * x; });
    std::cout << "Squared: ";
    for (int val : dest) std::cout << val << " ";
    std::cout << std::endl;
    
    // Fill
    std::fill(dest.begin(), dest.end(), 42);
    std::cout << "Filled with 42: ";
    for (int val : dest) std::cout << val << " ";
    std::cout << std::endl;
    
    // NUMERIC ALGORITHMS
    std::cout << "\n--- NUMERIC ALGORITHMS ---\n";
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    int sum = std::accumulate(numbers.begin(), numbers.end(), 0);
    std::cout << "Sum: " << sum << std::endl;
    
    int product = std::accumulate(numbers.begin(), numbers.end(), 1, 
                                  std::multiplies<int>());
    std::cout << "Product: " << product << std::endl;
    
    // Partial sum
    std::vector<int> partialSums(numbers.size());
    std::partial_sum(numbers.begin(), numbers.end(), partialSums.begin());
    std::cout << "Partial sums: ";
    for (int val : partialSums) std::cout << val << " ";
    std::cout << std::endl;
}

// =============================================================================
// 6. ITERATORS DEMONSTRATION
// =============================================================================

void demonstrateIterators() {
    std::cout << "\n========== ITERATORS ==========\n";
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    // Different types of iterators
    std::cout << "\n--- ITERATOR TYPES ---\n";
    
    // Forward iterator
    std::cout << "Forward iteration: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // Reverse iterator
    std::cout << "Reverse iteration: ";
    for (auto it = vec.rbegin(); it != vec.rend(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // Const iterator
    std::cout << "Const iteration: ";
    for (auto it = vec.cbegin(); it != vec.cend(); ++it) {
        std::cout << *it << " ";
        // *it = 10;  // Error: cannot modify through const iterator
    }
    std::cout << std::endl;
    
    // ITERATOR UTILITIES
    std::cout << "\n--- ITERATOR UTILITIES ---\n";
    
    // Distance
    auto distance = std::distance(vec.begin(), vec.end());
    std::cout << "Distance from begin to end: " << distance << std::endl;
    
    // Advance
    auto it = vec.begin();
    std::advance(it, 2);
    std::cout << "Element at position 2: " << *it << std::endl;
    
    // Next and prev
    auto next_it = std::next(vec.begin(), 3);
    auto prev_it = std::prev(vec.end(), 2);
    std::cout << "Next(begin, 3): " << *next_it << std::endl;
    std::cout << "Prev(end, 2): " << *prev_it << std::endl;
    
    // STREAM ITERATORS
    std::cout << "\n--- STREAM ITERATORS ---\n";
    std::vector<int> nums = {10, 20, 30, 40, 50};
    
    std::cout << "Output with ostream_iterator: ";
    std::copy(nums.begin(), nums.end(), 
              std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
}

// =============================================================================
// 7. FUNCTION OBJECTS AND LAMBDAS
// =============================================================================

// Custom function object
struct Multiply {
    int factor;
    Multiply(int f) : factor(f) {}
    int operator()(int x) const { return x * factor; }
};

void demonstrateFunctionObjects() {
    std::cout << "\n========== FUNCTION OBJECTS & LAMBDAS ==========\n";
    
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // PREDEFINED FUNCTION OBJECTS
    std::cout << "\n--- PREDEFINED FUNCTION OBJECTS ---\n";
    std::vector<int> vec1 = numbers;
    std::sort(vec1.begin(), vec1.end(), std::greater<int>());
    std::cout << "Sorted in descending order: ";
    for (int val : vec1) std::cout << val << " ";
    std::cout << std::endl;
    
    // CUSTOM FUNCTION OBJECT
    std::cout << "\n--- CUSTOM FUNCTION OBJECT ---\n";
    std::vector<int> vec2 = numbers;
    std::transform(vec2.begin(), vec2.end(), vec2.begin(), Multiply(3));
    std::cout << "Multiplied by 3: ";
    for (int val : vec2) std::cout << val << " ";
    std::cout << std::endl;
    
    // LAMBDA EXPRESSIONS
    std::cout << "\n--- LAMBDA EXPRESSIONS ---\n";
    
    // Simple lambda
    auto square = [](int x) { return x * x; };
    std::vector<int> vec3 = numbers;
    std::transform(vec3.begin(), vec3.end(), vec3.begin(), square);
    std::cout << "Squared: ";
    for (int val : vec3) std::cout << val << " ";
    std::cout << std::endl;
    
    // Lambda with capture
    int threshold = 5;
    auto count_greater = [threshold](const std::vector<int>& v) {
        return std::count_if(v.begin(), v.end(), 
                           [threshold](int x) { return x > threshold; });
    };
    
    std::cout << "Count of numbers > " << threshold << ": " 
              << count_greater(numbers) << std::endl;
    
    // Complex lambda with multiple captures
    int multiplier = 2;
    std::string prefix = "Value: ";
    
    std::for_each(numbers.begin(), numbers.begin() + 3, 
                  [&prefix, multiplier](int x) {
                      std::cout << prefix << (x * multiplier) << " ";
                  });
    std::cout << std::endl;
}

// =============================================================================
// 8. STL UTILITIES AND HELPERS
// =============================================================================

void demonstrateUtilities() {
    std::cout << "\n========== STL UTILITIES ==========\n";
    
    // PAIR
    std::cout << "\n--- PAIR ---\n";
    std::pair<std::string, int> person("Alice", 25);
    std::cout << "Person: " << person.first << ", Age: " << person.second << std::endl;
    
    // Make pair
    auto coords = std::make_pair(3.14, 2.71);
    std::cout << "Coordinates: (" << coords.first << ", " << coords.second << ")" << std::endl;
    
    // TUPLE (C++11)
    std::cout << "\n--- TUPLE ---\n";
    std::tuple<std::string, int, double> student("Bob", 20, 3.75);
    std::cout << "Student: " << std::get<0>(student) 
              << ", Age: " << std::get<1>(student)
              << ", GPA: " << std::get<2>(student) << std::endl;
    
    // Structured binding (C++17)
    // auto [name, age, gpa] = student;  // Uncomment if C++17 available
    
    // SWAP
    std::cout << "\n--- SWAP ---\n";
    int a = 10, b = 20;
    std::cout << "Before swap: a=" << a << ", b=" << b << std::endl;
    std::swap(a, b);
    std::cout << "After swap: a=" << a << ", b=" << b << std::endl;
    
    // MIN/MAX
    std::cout << "\n--- MIN/MAX ---\n";
    std::vector<int> values = {3, 1, 4, 1, 5, 9, 2, 6};
    auto minElement = std::min_element(values.begin(), values.end());
    auto maxElement = std::max_element(values.begin(), values.end());
    
    std::cout << "Min element: " << *minElement << " at index " 
              << (minElement - values.begin()) << std::endl;
    std::cout << "Max element: " << *maxElement << " at index " 
              << (maxElement - values.begin()) << std::endl;
    
    std::cout << "Min of 5 and 3: " << std::min(5, 3) << std::endl;
    std::cout << "Max of 5 and 3: " << std::max(5, 3) << std::endl;
}

// =============================================================================
// 9. PERFORMANCE DEMONSTRATION
// =============================================================================

void demonstratePerformance() {
    std::cout << "\n========== PERFORMANCE COMPARISON ==========\n";
    
    const int SIZE = 100000;
    
    // Vector vs List insertion performance
    std::cout << "\n--- INSERTION PERFORMANCE ---\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Vector - back insertion
    std::vector<int> vec;
    vec.reserve(SIZE);  // Pre-allocate to avoid reallocations
    for (int i = 0; i < SIZE; ++i) {
        vec.push_back(i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto vector_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Vector push_back: " << vector_time.count() << " microseconds\n";
    
    // List - front insertion
    start = std::chrono::high_resolution_clock::now();
    
    std::list<int> lst;
    for (int i = 0; i < SIZE; ++i) {
        lst.push_front(i);
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto list_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "List push_front: " << list_time.count() << " microseconds\n";
    
    // Map vs Unordered_map lookup
    std::cout << "\n--- LOOKUP PERFORMANCE ---\n";
    
    std::map<int, int> ordered_map;
    std::unordered_map<int, int> hash_map;
    
    // Fill both maps
    for (int i = 0; i < SIZE; ++i) {
        ordered_map[i] = i * 2;
        hash_map[i] = i * 2;
    }
    
    // Test ordered map lookup
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < SIZE; ++i) {
        auto it = ordered_map.find(i);
        (void)it;  // Suppress unused variable warning
    }
    end = std::chrono::high_resolution_clock::now();
    auto map_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Map lookup: " << map_time.count() << " microseconds\n";
    
    // Test unordered map lookup
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < SIZE; ++i) {
        auto it = hash_map.find(i);
        (void)it;  // Suppress unused variable warning
    }
    end = std::chrono::high_resolution_clock::now();
    auto unordered_map_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Unordered_map lookup: " << unordered_map_time.count() << " microseconds\n";
    
    std::cout << "\nNote: Results may vary based on system and data patterns\n";
}

// =============================================================================
// 10. REAL-WORLD EXAMPLES
// =============================================================================

void demonstrateRealWorldExamples() {
    std::cout << "\n========== REAL-WORLD EXAMPLES ==========\n";
    
    // Example 1: Word frequency counter
    std::cout << "\n--- WORD FREQUENCY COUNTER ---\n";
    std::string text = "the quick brown fox jumps over the lazy dog the fox is quick";
    std::unordered_map<std::string, int> wordFreq;
    
    // Simple word extraction (splitting by space)
    std::string word;
    for (char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                wordFreq[word]++;
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        wordFreq[word]++;
    }
    
    std::cout << "Word frequencies:\n";
    for (const auto& pair : wordFreq) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    
    // Example 2: Top K elements
    std::cout << "\n--- TOP K ELEMENTS ---\n";
    std::vector<int> numbers = {4, 1, 7, 2, 8, 3, 9, 5, 6};
    int k = 3;
    
    // Using partial_sort
    std::vector<int> topK = numbers;
    std::partial_sort(topK.begin(), topK.begin() + k, topK.end(), std::greater<int>());
    
    std::cout << "Top " << k << " elements: ";
    for (int i = 0; i < k; ++i) {
        std::cout << topK[i] << " ";
    }
    std::cout << std::endl;
    
    // Example 3: Remove duplicates while preserving order
    std::cout << "\n--- REMOVE DUPLICATES (PRESERVE ORDER) ---\n";
    std::vector<int> withDuplicates = {1, 3, 2, 3, 4, 1, 5, 2, 6};
    std::vector<int> unique;
    std::unordered_set<int> seen;
    
    for (int num : withDuplicates) {
        if (seen.find(num) == seen.end()) {
            seen.insert(num);
            unique.push_back(num);
        }
    }
    
    std::cout << "Original: ";
    for (int num : withDuplicates) std::cout << num << " ";
    std::cout << "\nUnique: ";
    for (int num : unique) std::cout << num << " ";
    std::cout << std::endl;
    
    // Example 4: LRU Cache simulation
    std::cout << "\n--- LRU CACHE SIMULATION ---\n";
    const int CACHE_SIZE = 3;
    std::list<int> cache;
    std::unordered_map<int, std::list<int>::iterator> cacheMap;
    
    auto accessPage = [&](int page) {
        auto it = cacheMap.find(page);
        if (it != cacheMap.end()) {
            // Page hit - move to front
            cache.erase(it->second);
            cache.push_front(page);
            cacheMap[page] = cache.begin();
            std::cout << "Hit: " << page;
        } else {
            // Page miss
            if (cache.size() >= CACHE_SIZE) {
                // Remove least recently used
                int lru = cache.back();
                cache.pop_back();
                cacheMap.erase(lru);
            }
            cache.push_front(page);
            cacheMap[page] = cache.begin();
            std::cout << "Miss: " << page;
        }
        
        std::cout << " | Cache: ";
        for (int p : cache) std::cout << p << " ";
        std::cout << std::endl;
    };
    
    std::vector<int> pageAccesses = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    for (int page : pageAccesses) {
        accessPage(page);
    }
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main() {
    std::cout << "COMPREHENSIVE C++ STL DEMONSTRATION\n";
    std::cout << "===================================\n";
    
    demonstrateSequenceContainers();
    demonstrateAssociativeContainers();
    demonstrateUnorderedContainers();
    demonstrateContainerAdaptors();
    demonstrateAlgorithms();
    demonstrateIterators();
    demonstrateFunctionObjects();
    demonstrateUtilities();
    demonstratePerformance();
    demonstrateRealWorldExamples();
    
    std::cout << "\n========== SUMMARY ==========\n";
    std::cout << "STL provides:\n";
    std::cout << "1. Containers - Efficient data structures\n";
    std::cout << "2. Algorithms - Powerful operations on data\n";
    std::cout << "3. Iterators - Uniform way to traverse containers\n";
    std::cout << "4. Function Objects - Customizable behavior\n";
    std::cout << "5. Utilities - Helper functions and classes\n";
    std::cout << "\nMaster the STL for efficient and elegant C++ programming!\n";
    
    return 0;
}