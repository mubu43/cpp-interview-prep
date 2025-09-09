#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;
const char* SERVER_IP = "127.0.0.1";

class UDPServer {
private:
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];

public:
    UDPServer() : client_len(sizeof(client_addr)) {
        // Create socket
        server_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (server_fd < 0) {
            throw std::runtime_error("Socket creation failed");
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

        std::cout << "UDP Server listening on port " << PORT << std::endl;
    }

    ~UDPServer() {
        close(server_fd);
    }

    void start() {
        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            
            // Receive message from client
            ssize_t bytes_received = recvfrom(server_fd, buffer, BUFFER_SIZE - 1, 0,
                                            (struct sockaddr*)&client_addr, &client_len);
            
            if (bytes_received < 0) {
                std::cerr << "Error receiving data" << std::endl;
                continue;
            }

            buffer[bytes_received] = '\0';
            std::cout << "Received from client: " << buffer << std::endl;

            // Check for exit condition
            if (strcmp(buffer, "exit") == 0) {
                std::cout << "Server shutting down..." << std::endl;
                break;
            }

            // Echo the message back with a prefix
            std::string response = "Echo: " + std::string(buffer);
            
            // Send response back to client
            ssize_t bytes_sent = sendto(server_fd, response.c_str(), response.length(), 0,
                                      (const struct sockaddr*)&client_addr, client_len);
            
            if (bytes_sent < 0) {
                std::cerr << "Error sending response" << std::endl;
            } else {
                std::cout << "Sent response: " << response << std::endl;
            }
        }
    }
};

class UDPClient {
private:
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

public:
    UDPClient() {
        // Create socket
        client_fd = socket(AF_INET, SOCK_DGRAM, 0);
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

        std::cout << "UDP Client connected to " << SERVER_IP << ":" << PORT << std::endl;
    }

    ~UDPClient() {
        close(client_fd);
    }

    void sendMessage(const std::string& message) {
        // Send message to server
        ssize_t bytes_sent = sendto(client_fd, message.c_str(), message.length(), 0,
                                  (const struct sockaddr*)&server_addr, sizeof(server_addr));
        
        if (bytes_sent < 0) {
            std::cerr << "Error sending message" << std::endl;
            return;
        }

        std::cout << "Sent: " << message << std::endl;

        // Don't wait for response if sending "exit"
        if (message == "exit") {
            return;
        }

        // Receive response from server
        memset(buffer, 0, BUFFER_SIZE);
        socklen_t server_len = sizeof(server_addr);
        
        ssize_t bytes_received = recvfrom(client_fd, buffer, BUFFER_SIZE - 1, 0,
                                        (struct sockaddr*)&server_addr, &server_len);
        
        if (bytes_received < 0) {
            std::cerr << "Error receiving response" << std::endl;
            return;
        }

        buffer[bytes_received] = '\0';
        std::cout << "Received: " << buffer << std::endl;
    }

    void interactiveMode() {
        std::string message;
        std::cout << "\nEnter messages to send to server (type 'exit' to quit):" << std::endl;
        
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, message);
            
            if (message.empty()) continue;
            
            sendMessage(message);
            
            if (message == "exit") {
                break;
            }
        }
    }
};

// Demonstration function for automated testing
void demonstrateUDPCommunication() {
    std::cout << "\n=== UDP Communication Demonstration ===" << std::endl;
    
    try {
        UDPClient client;
        
        // Send some test messages
        std::vector<std::string> testMessages = {
            "Hello, Server!",
            "How are you?",
            "UDP is connectionless",
            "Testing message 123"
        };
        
        for (const auto& msg : testMessages) {
            client.sendMessage(msg);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        // Send exit message
        std::cout << "\nSending exit command..." << std::endl;
        client.sendMessage("exit");
        
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string mode = argv[1];
        
        if (mode == "server") {
            try {
                UDPServer server;
                server.start();
            } catch (const std::exception& e) {
                std::cerr << "Server error: " << e.what() << std::endl;
                return 1;
            }
        } else if (mode == "client") {
            try {
                UDPClient client;
                client.interactiveMode();
            } catch (const std::exception& e) {
                std::cerr << "Client error: " << e.what() << std::endl;
                return 1;
            }
        } else if (mode == "demo") {
            // For demo mode, we need a server running
            std::cout << "Demo mode: Make sure to run './program server' in another terminal first!" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            demonstrateUDPCommunication();
        } else {
            std::cout << "Usage: " << argv[0] << " [server|client|demo]" << std::endl;
            std::cout << "  server - Run as UDP server" << std::endl;
            std::cout << "  client - Run as interactive UDP client" << std::endl;
            std::cout << "  demo   - Run automated demonstration (requires server running)" << std::endl;
            return 1;
        }
    } else {
        std::cout << "Usage: " << argv[0] << " [server|client|demo]" << std::endl;
        std::cout << "  server - Run as UDP server" << std::endl;
        std::cout << "  client - Run as interactive UDP client" << std::endl;
        std::cout << "  demo   - Run automated demonstration (requires server running)" << std::endl;
        return 1;
    }
    
    return 0;
}

/*
COMPILATION AND USAGE:

1. Compile the program:
   g++ -std=c++11 -o udp_program udp_client_server.cpp -pthread

2. Run the server (in one terminal):
   ./udp_program server

3. Run the client (in another terminal):
   ./udp_program client

4. Or run the automated demo:
   ./udp_program demo

KEY CONCEPTS DEMONSTRATED:

1. Socket Creation:
   - socket(AF_INET, SOCK_DGRAM, 0) creates a UDP socket
   - AF_INET: IPv4 address family
   - SOCK_DGRAM: Datagram socket (UDP)

2. Server Operations:
   - bind(): Associates socket with a specific address and port
   - recvfrom(): Receives data from any client
   - sendto(): Sends data to a specific client address

3. Client Operations:
   - sendto(): Sends data to server
   - recvfrom(): Receives response from server
   - No explicit connection needed (connectionless)

4. Address Structures:
   - sockaddr_in: IPv4 socket address structure
   - inet_pton(): Converts IP string to binary format
   - htons(): Host to network byte order conversion

5. UDP Characteristics:
   - Connectionless: No handshake required
   - Unreliable: No guarantee of delivery or order
   - Fast: Lower overhead than TCP
   - Packet-based: Each send/receive is independent

6. Error Handling:
   - Check return values of socket operations
   - Proper resource cleanup with RAII

7. Practical Considerations:
   - Buffer management
   - Address handling
   - Graceful shutdown mechanisms
*/