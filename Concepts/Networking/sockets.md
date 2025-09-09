# Sockets API Study Guide

## Overview
Sockets provide a programming interface for network communication between processes, either on the same machine or across networks. This guide covers the essential concepts and APIs for implementing TCP and UDP sockets in C/C++.

## Core Socket Concepts

### Socket Types
- **SOCK_STREAM (TCP)**: Connection-oriented, reliable, ordered delivery
- **SOCK_DGRAM (UDP)**: Connectionless, unreliable, message-oriented
- **SOCK_RAW**: Direct access to lower-level protocols

### Address Families
- **AF_INET**: IPv4 addresses
- **AF_INET6**: IPv6 addresses
- **AF_UNIX**: Local/Unix domain sockets

## Essential Socket Functions

### 1. Socket Creation
```cpp
int socket(int domain, int type, int protocol);
```
- **domain**: Address family (AF_INET, AF_INET6)
- **type**: Socket type (SOCK_STREAM, SOCK_DGRAM)
- **protocol**: Usually 0 (auto-select protocol)
- **Returns**: Socket file descriptor or -1 on error

### 2. Address Structures

#### IPv4 Address Structure
```cpp
struct sockaddr_in {
    sa_family_t sin_family;     // AF_INET
    in_port_t sin_port;         // Port number (network byte order)
    struct in_addr sin_addr;    // IP address
    char sin_zero[8];           // Padding
};
```

#### Generic Address Structure
```cpp
struct sockaddr {
    sa_family_t sa_family;      // Address family
    char sa_data[14];           // Address data
};
```

### 3. Address Conversion Functions
```cpp
// String to binary IP address
int inet_pton(int af, const char *src, void *dst);

// Binary to string IP address
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);

// Host to network byte order (16-bit)
uint16_t htons(uint16_t hostshort);

// Network to host byte order (16-bit)
uint16_t ntohs(uint16_t netshort);
```

## TCP Socket Programming

### Server-Side Functions

#### 1. Bind Socket to Address
```cpp
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```
- Associates socket with specific IP address and port
- Must be called before `listen()`

#### 2. Listen for Connections
```cpp
int listen(int sockfd, int backlog);
```
- **backlog**: Maximum number of pending connections
- Puts socket in passive listening mode

#### 3. Accept Connections
```cpp
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```
- Blocks until client connects
- Returns new socket for communication with client
- Original socket continues listening

### Client-Side Functions

#### 1. Connect to Server
```cpp
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```
- Establishes connection to server
- Initiates TCP three-way handshake

### Data Transfer Functions (TCP)
```cpp
// Send data
ssize_t send(int sockfd, const void *buf, size_t len, int flags);

// Receive data
ssize_t recv(int sockfd, void *buf, size_t len, int flags);

// Alternative functions
ssize_t write(int sockfd, const void *buf, size_t count);
ssize_t read(int sockfd, void *buf, size_t count);
```

## UDP Socket Programming

### Data Transfer Functions (UDP)
```cpp
// Send data to specific address
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);

// Receive data from any address
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);
```

## Socket Options

### Setting Socket Options
```cpp
int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen);
```

#### Common Options
- **SO_REUSEADDR**: Allow address reuse
- **SO_KEEPALIVE**: Enable keep-alive packets
- **SO_RCVBUF**: Set receive buffer size
- **SO_SNDBUF**: Set send buffer size

### Getting Socket Options
```cpp
int getsockopt(int sockfd, int level, int optname,
               void *optval, socklen_t *optlen);
```

## Error Handling

### Common Error Codes
- **EADDRINUSE**: Address already in use
- **ECONNREFUSED**: Connection refused
- **ETIMEDOUT**: Connection timed out
- **EWOULDBLOCK/EAGAIN**: Operation would block (non-blocking sockets)

### Error Checking Best Practices
```cpp
if (socket_fd < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
}

if (bind(socket_fd, ...) < 0) {
    perror("bind failed");
    close(socket_fd);
    exit(EXIT_FAILURE);
}
```

## TCP vs UDP Comparison

| Feature | TCP | UDP |
|---------|-----|-----|
| **Connection** | Connection-oriented | Connectionless |
| **Reliability** | Reliable delivery | Unreliable |
| **Ordering** | Ordered delivery | No ordering guarantee |
| **Error Detection** | Built-in | Basic checksum |
| **Flow Control** | Yes | No |
| **Overhead** | Higher | Lower |
| **Use Cases** | Web, Email, File Transfer | Gaming, Streaming, DNS |

## Advanced Topics

### Non-blocking Sockets
```cpp
#include <fcntl.h>

// Make socket non-blocking
int flags = fcntl(sockfd, F_GETFL, 0);
fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
```

### Socket Multiplexing

#### select()
```cpp
int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);
```

#### poll()
```cpp
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
```

#### epoll() (Linux-specific)
```cpp
int epoll_create(int size);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
```

## Best Practices

### 1. Resource Management
- Always close sockets when done
- Use RAII in C++ for automatic cleanup
- Handle errors properly and clean up resources

### 2. Buffer Management
- Use appropriate buffer sizes
- Handle partial reads/writes in TCP
- Null-terminate received strings

### 3. Error Handling
- Check return values of all socket functions
- Use errno and perror() for error diagnosis
- Implement proper error recovery

### 4. Security Considerations
- Validate input data
- Implement timeouts to prevent blocking
- Use secure protocols (TLS/SSL) for sensitive data

### 5. Performance Optimization
- Use appropriate socket buffer sizes
- Consider non-blocking I/O for high-performance applications
- Use connection pooling for frequent connections

## Common Patterns

### TCP Echo Server
1. Create socket
2. Bind to address
3. Listen for connections
4. Accept client connections
5. Read data and echo back
6. Close client socket
7. Repeat from step 4

### UDP Echo Server
1. Create socket
2. Bind to address
3. Receive data from client
4. Send data back to same client
5. Repeat from step 3

### Client Pattern
1. Create socket
2. Connect to server (TCP) or just start sending (UDP)
3. Send/receive data
4. Close socket

## Debugging Tips

### Network Tools
- **netstat**: Show network connections
- **ss**: Modern replacement for netstat
- **tcpdump**: Packet capture and analysis
- **wireshark**: GUI packet analyzer
- **telnet**: Test TCP connections

### Common Issues
- **Port already in use**: Use SO_REUSEADDR
- **Connection refused**: Server not listening
- **Broken pipe**: Client disconnected unexpectedly
- **Address resolution**: Check DNS/IP configuration

## Example Compilation
```bash
# Basic compilation
gcc -o server server.c
gcc -o client client.c

# With threading support
gcc -pthread -o server server.c
gcc -pthread -o client client.c

# C++ compilation
g++ -std=c++11 -pthread -o server server.cpp
g++ -std=c++11 -pthread -o client client.cpp
```

## Further Reading
- Stevens, W. Richard. "Unix Network Programming" (The definitive guide)
- Beej's Guide to Network Programming (Free online resource)
- RFC 793 (TCP Specification)
- RFC 768 (UDP Specification)
