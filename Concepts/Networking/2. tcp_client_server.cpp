#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <vector>
#include <signal.h>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;
const char* SERVER_IP = "127.0.0.1";
const int MAX_CLIENTS = 5;

class TCPServer {
private:
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];

public:
    TCPServer() : client_len(sizeof(client_addr)) {
        // Create socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            throw std::runtime_error("Socket creation failed");
        }

        // Set socket options to reuse address
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            close(server_fd);
            throw std::runtime_error("Set socket options failed");
        }

        // Configure server address
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        // Bind socket to address
        if (bind(server_fd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(server_fd);
            throw std::runtime_error("Bind failed");
        }

        // Start listening for connections
        if (listen(server_fd, MAX_CLIENTS) < 0) {
            close(server_fd);
            throw std::runtime_error("Listen failed");
        }

        std::cout << "TCP Server listening on port " << PORT << std::endl;
    }

    ~TCPServer() {
        close(server_fd);
    }

    void handleClient(int client_socket) {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "New client connected from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            
            // Receive message from client
            ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    std::cout << "Client disconnected" << std::endl;
                } else {
                    std::cerr << "Error receiving data from client" << std::endl;
                }
                break;
            }

            buffer[bytes_received] = '\0';
            std::cout << "Received from client: " << buffer << std::endl;

            // Check for exit condition
            if (strcmp(buffer, "exit") == 0) {
                std::cout << "Client requested disconnect" << std::endl;
                break;
            }

            // Echo the message back with a prefix
            std::string response = "Echo: " + std::string(buffer);
            
            // Send response back to client
            ssize_t bytes_sent = send(client_socket, response.c_str(), response.length(), 0);
            
            if (bytes_sent < 0) {
                std::cerr << "Error sending response to client" << std::endl;
                break;
            } else {
                std::cout << "Sent response: " << response << std::endl;
            }
        }

        close(client_socket);
        std::cout << "Client connection closed" << std::endl;
    }

    void start() {
        std::cout << "Server started. Waiting for connections..." << std::endl;
        
        while (true) {
            // Accept incoming connection
            int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            
            if (client_socket < 0) {
                std::cerr << "Error accepting connection" << std::endl;
                continue;
            }

            // Handle client in a separate thread for concurrent connections
            std::thread client_thread(&TCPServer::handleClient, this, client_socket);
            client_thread.detach(); // Detach thread to handle multiple clients
        }
    }
};

class TCPClient {
private:
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    bool connected;

public:
    TCPClient() : connected(false) {
        // Create socket
        client_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd < 0) {
            throw std::runtime_error("Socket creation failed");
        }

        // Configure server address
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        
        if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
            close(client_fd);
            throw std::runtime_error("Invalid address/Address not supported");
        }
    }

    ~TCPClient() {
        if (connected) {
            disconnect();
        }
        close(client_fd);
    }

    bool connect_to_server() {
        // Connect to server
        if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Connection to server failed" << std::endl;
            return false;
        }

        connected = true;
        std::cout << "Connected to TCP Server at " << SERVER_IP << ":" << PORT << std::endl;
        return true;
    }

    void disconnect() {
        if (connected) {
            send(client_fd, "exit", 4, 0);
            connected = false;
            std::cout << "Disconnected from server" << std::endl;
        }
    }

    bool sendMessage(const std::string& message) {
        if (!connected) {
            std::cerr << "Not connected to server" << std::endl;
            return false;
        }

        // Send message to server
        ssize_t bytes_sent = send(client_fd, message.c_str(), message.length(), 0);
        
        if (bytes_sent < 0) {
            std::cerr << "Error sending message" << std::endl;
            return false;
        }

        std::cout << "Sent: " << message << std::endl;

        // Handle exit message
        if (message == "exit") {
            connected = false;
            return true;
        }

        // Receive response from server
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            std::cerr << "Error receiving response or server disconnected" << std::endl;
            connected = false;
            return false;
        }

        buffer[bytes_received] = '\0';
        std::cout << "Received: " << buffer << std::endl;
        return true;
    }

    void interactiveMode() {
        if (!connect_to_server()) {
            return;
        }

        std::string message;
        std::cout << "\nEnter messages to send to server (type 'exit' to quit):" << std::endl;
        
        while (connected) {
            std::cout << "> ";
            std::getline(std::cin, message);
            
            if (message.empty()) continue;
            
            if (!sendMessage(message)) {
                break;
            }
            
            if (message == "exit") {
                break;
            }
        }
    }
};

// Demonstration function for automated testing
void demonstrateTCPCommunication() {
    std::cout << "\n=== TCP Communication Demonstration ===" << std::endl;
    
    try {
        TCPClient client;
        
        if (!client.connect_to_server()) {
            return;
        }
        
        // Send some test messages
        std::vector<std::string> testMessages = {
            "Hello, TCP Server!",
            "This is a reliable connection",
            "TCP guarantees delivery",
            "Testing message ordering"
        };
        
        for (const auto& msg : testMessages) {
            if (!client.sendMessage(msg)) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        // Send exit message
        std::cout << "\nSending exit command..." << std::endl;
        client.sendMessage("exit");
        
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}

// Signal handler for graceful shutdown
volatile sig_atomic_t shutdown_requested = 0;

void signal_handler(int signal) {
    shutdown_requested = 1;
    std::cout << "\nShutdown requested..." << std::endl;
}

int main(int argc, char* argv[]) {
    // Set up signal handler for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (argc > 1) {
        std::string mode = argv[1];
        
        if (mode == "server") {
            try {
                TCPServer server;
                server.start();
            } catch (const std::exception& e) {
                std::cerr << "Server error: " << e.what() << std::endl;
                return 1;
            }
        } else if (mode == "client") {
            try {
                TCPClient client;
                client.interactiveMode();
            } catch (const std::exception& e) {
                std::cerr << "Client error: " << e.what() << std::endl;
                return 1;
            }
        } else if (mode == "demo") {
            // For demo mode, we need a server running
            std::cout << "Demo mode: Make sure to run './program server' in another terminal first!" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            demonstrateTCPCommunication();
        } else {
            std::cout << "Usage: " << argv[0] << " [server|client|demo]" << std::endl;
            std::cout << "  server - Run as TCP server" << std::endl;
            std::cout << "  client - Run as interactive TCP client" << std::endl;
            std::cout << "  demo   - Run automated demonstration (requires server running)" << std::endl;
            return 1;
        }
    } else {
        std::cout << "Usage: " << argv[0] << " [server|client|demo]" << std::endl;
        std::cout << "  server - Run as TCP server" << std::endl;
        std::cout << "  client - Run as interactive TCP client" << std::endl;
        std::cout << "  demo   - Run automated demonstration (requires server running)" << std::endl;
        return 1;
    }
    
    return 0;
}

/*
COMPILATION AND USAGE:

1. Compile the program:
   g++ -std=c++11 -o tcp_program tcp_client_server.cpp -pthread

2. Run the server (in one terminal):
   ./tcp_program server

3. Run the client (in another terminal):
   ./tcp_program client

4. Or run the automated demo:
   ./tcp_program demo

KEY CONCEPTS DEMONSTRATED:

1. Socket Creation:
   - socket(AF_INET, SOCK_STREAM, 0) creates a TCP socket
   - AF_INET: IPv4 address family
   - SOCK_STREAM: Stream socket (TCP)

2. Server Operations:
   - bind(): Associates socket with a specific address and port
   - listen(): Puts socket in listening mode
   - accept(): Accepts incoming client connections
   - recv()/send(): Receives/sends data over established connection

3. Client Operations:
   - connect(): Establishes connection to server
   - send()/recv(): Sends/receives data over connection
   - Connection-oriented: Must establish connection before data transfer

4. TCP Characteristics:
   - Connection-oriented: Requires handshake (SYN, SYN-ACK, ACK)
   - Reliable: Guarantees delivery and order
   - Stream-based: Continuous flow of bytes
   - Error detection and correction
   - Flow control and congestion control

5. Advanced Features:
   - Multi-threaded server for concurrent client handling
   - Signal handling for graceful shutdown
   - Socket options (SO_REUSEADDR)
   - Connection state management

6. Error Handling:
   - Connection establishment errors
   - Data transmission errors
   - Client disconnection handling
   - Resource cleanup

7. Practical Considerations:
   - Thread management for concurrent connections
   - Proper connection lifecycle management
   - Graceful shutdown procedures
   - Address reuse configuration
*/
