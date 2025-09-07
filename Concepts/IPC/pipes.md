# Pipes IPC

## Overview
Pipes are one of the oldest and simplest forms of Inter-Process Communication (IPC) in Unix-like systems. They provide a unidirectional communication channel between processes, following a producer-consumer model.

## Intent
- **Provide simple, reliable communication between processes**
- **Enable data streaming between producer and consumer**
- **Support shell-like command chaining and data flow**
- **Offer portable IPC mechanism across Unix-like systems**

## Problem It Solves
- Need for simple process-to-process communication
- Data streaming from one process to another
- Command chaining and pipeline processing
- Parent-child process communication
- Simple producer-consumer scenarios

## Types of Pipes

### 1. Anonymous Pipes
- **Scope**: Related processes only (parent-child)
- **Lifetime**: Exist only while processes are running
- **Creation**: `pipe()` system call
- **Usage**: Simple parent-child communication

### 2. Named Pipes (FIFOs)
- **Scope**: Any processes on the same system
- **Lifetime**: Persist in filesystem until removed
- **Creation**: `mkfifo()` system call or command
- **Usage**: Communication between unrelated processes

## Key Characteristics

### 1. Unidirectional Communication
- Data flows in one direction only
- Separate pipes needed for bidirectional communication
- Writer process sends data, reader process receives

### 2. FIFO Behavior
- First In, First Out ordering
- Data written first is read first
- Sequential data access

### 3. Automatic Synchronization
- Blocking behavior by default
- Reader blocks when no data available
- Writer blocks when pipe buffer is full

### 4. Buffering
- Kernel maintains internal buffer
- Buffer size typically 4KB to 64KB
- Automatic flow control

## Implementation Approaches

### 1. Anonymous Pipes
```cpp
#include <unistd.h>

int pipe_fds[2];
if (pipe(pipe_fds) == -1) {
    perror("pipe");
    return -1;
}

// pipe_fds[0] = read end
// pipe_fds[1] = write end
```

### 2. Named Pipes (FIFOs)
```cpp
#include <sys/stat.h>
#include <fcntl.h>

// Create named pipe
if (mkfifo("/tmp/mypipe", 0666) == -1) {
    perror("mkfifo");
    return -1;
}

// Open for writing
int write_fd = open("/tmp/mypipe", O_WRONLY);

// Open for reading
int read_fd = open("/tmp/mypipe", O_RDONLY);
```

### 3. Command Pipes (popen)
```cpp
#include <cstdio>

FILE* pipe = popen("ls -la", "r");
char buffer[256];
while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    printf("%s", buffer);
}
pclose(pipe);
```

## Communication Patterns

### 1. Parent-Child Communication
```cpp
int pipe_fds[2];
pipe(pipe_fds);

pid_t pid = fork();
if (pid == 0) {
    // Child process
    close(pipe_fds[1]);  // Close write end
    
    char buffer[256];
    read(pipe_fds[0], buffer, sizeof(buffer));
    printf("Child received: %s\n", buffer);
    
} else {
    // Parent process
    close(pipe_fds[0]);  // Close read end
    
    const char* message = "Hello child!";
    write(pipe_fds[1], message, strlen(message));
}
```

### 2. Producer-Consumer Pattern
```cpp
// Producer
void producer(int write_fd) {
    for (int i = 0; i < 100; ++i) {
        std::string data = "Data item " + std::to_string(i);
        write(write_fd, data.c_str(), data.length());
    }
    close(write_fd);  // Signal end of data
}

// Consumer
void consumer(int read_fd) {
    char buffer[256];
    ssize_t bytes_read;
    
    while ((bytes_read = read(read_fd, buffer, sizeof(buffer))) > 0) {
        buffer[bytes_read] = '\0';
        processData(buffer);
    }
}
```

### 3. Pipeline Processing
```cpp
// Simulate: command1 | command2 | command3
void createPipeline() {
    int pipe1[2], pipe2[2];
    pipe(pipe1);
    pipe(pipe2);
    
    if (fork() == 0) {
        // First process: command1
        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        exec_command1();
    }
    
    if (fork() == 0) {
        // Second process: command2
        dup2(pipe1[0], STDIN_FILENO);
        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        exec_command2();
    }
    
    if (fork() == 0) {
        // Third process: command3
        dup2(pipe2[0], STDIN_FILENO);
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        exec_command3();
    }
    
    // Parent closes all pipe ends
    close(pipe1[0]); close(pipe1[1]);
    close(pipe2[0]); close(pipe2[1]);
}
```

## Real-World Applications

### 1. Shell Command Pipelines
- **Use Case**: Chaining commands for data processing
- **Example**: `cat file.txt | grep "pattern" | sort | uniq`
- **Benefits**: Modular processing, reusable components

### 2. Log Processing
- **Use Case**: Real-time log analysis and filtering
- **Example**: Log aggregation, filtering, and forwarding
- **Benefits**: Stream processing, low memory usage

### 3. Data Processing Pipelines
- **Use Case**: ETL (Extract, Transform, Load) operations
- **Example**: Data parsing, transformation, and storage
- **Benefits**: Streaming data processing, scalable architecture

### 4. Inter-Process Coordination
- **Use Case**: Process synchronization and communication
- **Example**: Master-worker process coordination
- **Benefits**: Simple synchronization, automatic buffering

### 5. System Utilities
- **Use Case**: System administration and monitoring tools
- **Example**: Process monitoring, system status reporting
- **Benefits**: Simple implementation, portable across systems

## Advantages
1. **Simplicity**: Easy to understand and implement
2. **Portability**: Available on all Unix-like systems
3. **Automatic Synchronization**: Built-in flow control
4. **Low Overhead**: Minimal system resource usage
5. **Reliable**: Data integrity guaranteed by kernel
6. **Stream Processing**: Suitable for continuous data flow

## Disadvantages
1. **Unidirectional**: Single direction communication only
2. **Limited Scope**: Anonymous pipes only work with related processes
3. **No Message Boundaries**: Stream-oriented, not message-oriented
4. **Buffer Limitations**: Fixed buffer size limits
5. **No Priority**: All data treated equally
6. **Blocking Behavior**: Can cause deadlocks if not managed properly

## Advanced Pipe Techniques

### 1. Non-blocking Pipes
```cpp
#include <fcntl.h>

// Set non-blocking mode
int flags = fcntl(pipe_fd, F_GETFL);
fcntl(pipe_fd, F_SETFL, flags | O_NONBLOCK);

// Non-blocking read
ssize_t bytes_read = read(pipe_fd, buffer, sizeof(buffer));
if (bytes_read == -1 && errno == EAGAIN) {
    // No data available
}
```

### 2. Pipe Capacity Management
```cpp
// Get pipe capacity (Linux-specific)
long capacity = fcntl(pipe_fd, F_GETPIPE_SZ);

// Set pipe capacity (Linux-specific)
fcntl(pipe_fd, F_SETPIPE_SZ, new_capacity);
```

### 3. Signal Handling
```cpp
// Handle SIGPIPE when writing to broken pipe
signal(SIGPIPE, SIG_IGN);  // Ignore SIGPIPE

ssize_t result = write(pipe_fd, data, size);
if (result == -1 && errno == EPIPE) {
    // Pipe broken, reader closed
}
```

### 4. Bidirectional Communication
```cpp
class BidirectionalPipe {
    int parent_to_child[2];
    int child_to_parent[2];
    
public:
    void initialize() {
        pipe(parent_to_child);
        pipe(child_to_parent);
    }
    
    void setupParent() {
        close(parent_to_child[0]);  // Parent writes here
        close(child_to_parent[1]);  // Parent reads here
    }
    
    void setupChild() {
        close(parent_to_child[1]);  // Child reads here
        close(child_to_parent[0]);  // Child writes here
    }
};
```

## Performance Considerations

### Factors Affecting Performance
- **Buffer Size**: Larger buffers reduce system call overhead
- **Data Size**: Smaller writes more efficient for small data
- **Blocking vs Non-blocking**: Non-blocking requires more complex logic
- **Process Scheduling**: Context switches affect throughput

### Optimization Strategies
```cpp
// Buffered writing
class BufferedPipeWriter {
    std::vector<char> buffer_;
    size_t buffer_pos_;
    int pipe_fd_;
    
public:
    void write(const char* data, size_t size) {
        if (buffer_pos_ + size > buffer_.size()) {
            flush();
        }
        memcpy(buffer_.data() + buffer_pos_, data, size);
        buffer_pos_ += size;
    }
    
    void flush() {
        if (buffer_pos_ > 0) {
            ::write(pipe_fd_, buffer_.data(), buffer_pos_);
            buffer_pos_ = 0;
        }
    }
};
```

## Error Handling

### Common Error Scenarios
1. **Broken Pipe (SIGPIPE)**: Reader closes while writer is active
2. **Full Pipe**: Writer blocks when pipe buffer is full
3. **Permission Denied**: Access rights issues with named pipes
4. **File Descriptor Limits**: Too many open file descriptors

### Error Handling Patterns
```cpp
ssize_t safe_write(int fd, const void* buf, size_t count) {
    ssize_t total_written = 0;
    const char* data = static_cast<const char*>(buf);
    
    while (total_written < count) {
        ssize_t written = write(fd, data + total_written, 
                               count - total_written);
        if (written == -1) {
            if (errno == EINTR) {
                continue;  // Interrupted by signal, retry
            } else if (errno == EPIPE) {
                return -1;  // Broken pipe
            } else {
                perror("write");
                return -1;
            }
        }
        total_written += written;
    }
    return total_written;
}
```

## Best Practices

### 1. Design Guidelines
- **Close unused ends**: Always close unused pipe ends
- **Handle SIGPIPE**: Ignore or handle broken pipe signals
- **Use appropriate buffer sizes**: Balance memory and performance
- **Design for failures**: Handle pipe breakage gracefully

### 2. Resource Management
- **RAII pattern**: Use destructors for cleanup
- **Exception safety**: Ensure pipes are closed in all paths
- **File descriptor limits**: Monitor and manage fd usage
- **Memory management**: Avoid memory leaks in buffers

### 3. Synchronization
- **Avoid deadlocks**: Careful ordering of read/write operations
- **Use timeouts**: For non-critical operations
- **Signal handling**: Proper signal management
- **Process lifecycle**: Coordinate process startup/shutdown

### 4. Performance Optimization
- **Batch operations**: Group small writes together
- **Use splice()**: Zero-copy data transfer (Linux)
- **Monitor pipe usage**: Profile pipe performance
- **Consider alternatives**: For high-performance scenarios

## Security Considerations

### Named Pipe Security
```cpp
// Create pipe with specific permissions
mode_t old_umask = umask(0);
mkfifo("/tmp/secure_pipe", 0600);  // Owner read/write only
umask(old_umask);

// Validate pipe ownership and permissions
struct stat pipe_stat;
if (stat("/tmp/secure_pipe", &pipe_stat) == 0) {
    if (pipe_stat.st_uid != getuid()) {
        // Pipe not owned by current user
        return -1;
    }
}
```

### Data Validation
```cpp
bool validatePipeData(const char* data, size_t size) {
    // Check for reasonable size limits
    if (size > MAX_PIPE_MESSAGE_SIZE) {
        return false;
    }
    
    // Validate data format
    if (!isValidFormat(data, size)) {
        return false;
    }
    
    return true;
}
```

## Interview Questions & Answers

### Q: What's the difference between anonymous and named pipes?
**A:** Anonymous pipes connect related processes (parent-child) and exist only during process lifetime. Named pipes (FIFOs) exist in the filesystem and can connect any processes on the same system.

### Q: How do you handle bidirectional communication with pipes?
**A:** Create two pipes - one for each direction. Each process closes the unused ends and uses one pipe for reading and another for writing.

### Q: What happens when you write to a pipe with no readers?
**A:** The writer receives a SIGPIPE signal and the write() call returns -1 with errno set to EPIPE. The signal can be ignored to handle this gracefully.

### Q: How do pipes compare to other IPC mechanisms?
**A:** Pipes are simpler but less powerful than shared memory or message queues. They're best for simple producer-consumer scenarios and command chaining.

### Q: What are the limitations of pipe buffer size?
**A:** Pipe buffers are typically 4KB-64KB. Writers block when the buffer is full, and the size is usually not configurable (except on Linux with F_SETPIPE_SZ).

## Modern C++ Considerations
- Use RAII for automatic pipe cleanup
- Leverage smart pointers for pipe management
- Consider async I/O libraries for non-blocking operations
- Use standard library containers for buffering
- Implement proper exception safety

## Alternative Technologies
- **Unix Domain Sockets**: More flexible than pipes
- **Message Queues**: For structured message passing
- **Shared Memory**: For high-performance communication
- **Network Sockets**: For distributed communication
- **Memory-Mapped Files**: For large data sharing
