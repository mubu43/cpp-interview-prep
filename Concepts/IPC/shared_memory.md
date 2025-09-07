# Shared Memory IPC

## Overview
Shared Memory is the fastest form of Inter-Process Communication (IPC) where multiple processes can access the same memory region directly. It provides zero-copy data sharing between processes.

## Intent
- **Enable multiple processes to share data efficiently**
- **Provide the fastest possible IPC mechanism**
- **Allow direct memory access without data copying**
- **Support high-throughput, low-latency communication**

## Problem It Solves
- Need for high-performance data exchange between processes
- Avoiding the overhead of copying data through kernel space
- Real-time systems requiring minimal latency
- Large data structures that are expensive to copy

## Key Concepts

### 1. Memory Mapping
- Virtual memory pages mapped to the same physical memory
- Processes see the same data at potentially different virtual addresses
- Operating system manages the mapping transparently

### 2. Synchronization Requirements
- **Race Conditions**: Multiple processes accessing same memory
- **Critical Sections**: Code that must execute atomically
- **Synchronization Primitives**: Semaphores, mutexes, condition variables

### 3. Memory Management
- **Creation**: One process creates the shared memory segment
- **Attachment**: Other processes attach to existing segment
- **Cleanup**: Explicit removal required (doesn't auto-cleanup)

## Implementation Approaches

### 1. POSIX Shared Memory
```cpp
// Create shared memory
int shm_fd = shm_open("/myshm", O_CREAT | O_RDWR, 0666);
ftruncate(shm_fd, sizeof(MyData));

// Map to process address space
MyData* data = (MyData*)mmap(0, sizeof(MyData), 
                            PROT_READ | PROT_WRITE, 
                            MAP_SHARED, shm_fd, 0);
```

### 2. System V Shared Memory
```cpp
// Create shared memory
key_t key = ftok("/tmp/myfile", 65);
int shmid = shmget(key, sizeof(MyData), 0666 | IPC_CREAT);

// Attach to address space
MyData* data = (MyData*)shmat(shmid, NULL, 0);
```

### 3. Memory-Mapped Files
```cpp
// Create/open file
int fd = open("datafile", O_CREAT | O_RDWR, 0666);
ftruncate(fd, sizeof(MyData));

// Map file to memory
MyData* data = (MyData*)mmap(0, sizeof(MyData),
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
```

## Synchronization Mechanisms

### 1. Semaphores
```cpp
sem_t* sem = sem_open("/mysem", O_CREAT, 0666, 1);

// Critical section
sem_wait(sem);  // Lock
// Access shared data
sem_post(sem);  // Unlock
```

### 2. Mutexes (Process-shared)
```cpp
pthread_mutex_t mutex;
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
pthread_mutex_init(&mutex, &attr);
```

### 3. Atomic Operations
```cpp
#include <atomic>
struct SharedData {
    std::atomic<int> counter{0};
    std::atomic<bool> ready{false};
};
```

## Real-World Applications

### 1. High-Frequency Trading
- **Use Case**: Ultra-low latency market data sharing
- **Benefits**: Microsecond-level data updates
- **Example**: Market data feeds, order book updates

### 2. Multimedia Processing
- **Use Case**: Video/audio buffer sharing
- **Benefits**: Avoid copying large frame buffers
- **Example**: Video players, streaming applications

### 3. Database Systems
- **Use Case**: Buffer pools, shared caches
- **Benefits**: Multiple processes sharing cached data
- **Example**: PostgreSQL shared buffers, Redis clusters

### 4. Scientific Computing
- **Use Case**: Large datasets, parallel processing
- **Benefits**: Workers share computation results
- **Example**: Matrix operations, simulation data

### 5. Game Engines
- **Use Case**: Asset loading, state synchronization
- **Benefits**: Fast data sharing between engine components
- **Example**: Asset managers, physics engines

## Advantages
1. **Performance**: Fastest IPC method available
2. **Zero-Copy**: No data copying between processes
3. **Large Data**: Efficient for big data structures
4. **Flexibility**: Can share complex data structures
5. **Scalability**: Multiple processes can share same segment

## Disadvantages
1. **Complexity**: Requires careful synchronization
2. **Fragility**: Memory corruption affects all processes
3. **Platform Dependent**: Different APIs on different systems
4. **No Built-in Synchronization**: Manual synchronization required
5. **Cleanup Issues**: Manual cleanup can lead to resource leaks

## Common Pitfalls

### 1. Race Conditions
```cpp
// WRONG: No synchronization
shared_data->counter++;

// CORRECT: With synchronization
sem_wait(semaphore);
shared_data->counter++;
sem_post(semaphore);
```

### 2. Memory Leaks
```cpp
// WRONG: Forgetting cleanup
// Shared memory persists after process exit

// CORRECT: Explicit cleanup
shm_unlink("/myshm");
```

### 3. Pointer Issues
```cpp
// WRONG: Storing pointers in shared memory
struct BadSharedData {
    char* message;  // Invalid in other processes
};

// CORRECT: Storing data directly
struct GoodSharedData {
    char message[256];  // Valid in all processes
};
```

## Best Practices

### 1. Design Guidelines
- **Keep it simple**: Minimize shared data complexity
- **Use fixed-size structures**: Avoid dynamic allocation
- **Version your data**: Handle structure evolution
- **Document synchronization**: Clear locking protocols

### 2. Synchronization Strategy
- **Minimize critical sections**: Reduce lock contention
- **Use atomic operations**: For simple counters/flags
- **Prefer RAII**: Automatic lock management
- **Avoid deadlocks**: Consistent lock ordering

### 3. Error Handling
- **Check all system calls**: Handle failures gracefully
- **Validate shared data**: Detect corruption early
- **Implement timeouts**: Avoid infinite waits
- **Plan for cleanup**: Handle abnormal termination

### 4. Performance Optimization
- **Align data structures**: Optimize cache performance
- **Batch operations**: Reduce synchronization overhead
- **Use lock-free algorithms**: When possible
- **Profile memory access**: Understand access patterns

## Security Considerations

### 1. Access Control
```cpp
// Set appropriate permissions
shm_open("/myshm", O_CREAT | O_RDWR, 0600);  // Owner only
```

### 2. Data Validation
```cpp
// Validate shared data integrity
if (shared_data->magic_number != EXPECTED_MAGIC) {
    // Handle corruption
}
```

### 3. Resource Limits
- Monitor shared memory usage
- Implement size limits
- Handle resource exhaustion

## Performance Characteristics

### Typical Performance Numbers
- **Latency**: 10-100 nanoseconds for local access
- **Throughput**: Limited by memory bandwidth (~10-50 GB/s)
- **Overhead**: Minimal for data access, higher for synchronization

### Factors Affecting Performance
- **Data size**: Larger structures may have cache misses
- **Access patterns**: Sequential vs. random access
- **Synchronization frequency**: Lock contention
- **Process count**: More processes = more contention

## Interview Questions & Answers

### Q: How does shared memory differ from other IPC mechanisms?
**A:** Shared memory is the fastest IPC method because it allows direct memory access without copying data through kernel space, unlike pipes or message queues which involve system calls and data copying.

### Q: What are the main challenges with shared memory?
**A:** Synchronization complexity, potential for memory corruption, manual cleanup requirements, and platform-specific implementations.

### Q: When would you choose shared memory over message queues?
**A:** When you need maximum performance, are sharing large amounts of data, require low latency, or need to share complex data structures between processes.

### Q: How do you handle synchronization in shared memory?
**A:** Use semaphores, mutexes, or atomic operations to protect critical sections. Design clear locking protocols and minimize lock contention.

### Q: What happens if a process crashes while holding a lock?
**A:** The lock may remain held indefinitely, causing deadlock. Use robust mutexes, timeouts, or process monitoring to handle this scenario.

## Modern C++ Considerations
- Use smart pointers for RAII where possible
- Leverage atomic operations for simple synchronization
- Consider Boost.Interprocess for portable implementation
- Use memory pools for complex allocation scenarios
- Implement proper exception safety

## Alternative Libraries
- **Boost.Interprocess**: Portable, feature-rich
- **Intel TBB**: High-performance concurrent containers
- **Facebook Folly**: Production-proven shared memory utilities
- **Apache Arrow**: Columnar data sharing
- **ZeroMQ**: High-level messaging with shared memory transport
