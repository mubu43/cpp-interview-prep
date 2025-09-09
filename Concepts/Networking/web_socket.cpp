#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <vector>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

const int PORT = 8080;
const int BUFFER_SIZE = 4096;
const std::string WEBSOCKET_MAGIC = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

// WebSocket frame opcodes
enum class WebSocketOpcode : uint8_t {
    CONTINUATION = 0x0,
    TEXT = 0x1,
    BINARY = 0x2,
    CLOSE = 0x8,
    PING = 0x9,
    PONG = 0xA
};

// WebSocket frame structure
struct WebSocketFrame {
    bool fin;
    bool rsv1, rsv2, rsv3;
    WebSocketOpcode opcode;
    bool mask;
    uint64_t payload_length;
    uint32_t masking_key;
    std::vector<uint8_t> payload;
};

class WebSocketUtils {
public:
    // Base64 encoding function
    static std::string base64_encode(const std::string& input) {
        BIO *bio, *b64;
        BUF_MEM *bufferPtr;
        
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);
        
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, input.c_str(), input.length());
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);
        
        std::string result(bufferPtr->data, bufferPtr->length);
        BIO_free_all(bio);
        
        return result;
    }
    
    // Generate WebSocket accept key
    static std::string generate_accept_key(const std::string& key) {
        std::string combined = key + WEBSOCKET_MAGIC;
        
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.length(), hash);
        
        std::string hash_str(reinterpret_cast<char*>(hash), SHA_DIGEST_LENGTH);
        return base64_encode(hash_str);
    }
    
    // Parse HTTP headers
    static std::map<std::string, std::string> parse_headers(const std::string& request) {
        std::map<std::string, std::string> headers;
        std::istringstream stream(request);
        std::string line;
        
        // Skip the first line (HTTP request line)
        std::getline(stream, line);
        
        while (std::getline(stream, line) && line != "\r") {
            auto colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string key = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 1);
                
                // Trim whitespace
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                
                headers[key] = value;
            }
        }
        
        return headers;
    }
    
    // Create WebSocket handshake response
    static std::string create_handshake_response(const std::string& accept_key) {
        std::string response = 
            "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: " + accept_key + "\r\n"
            "\r\n";
        return response;
    }
    
    // Parse WebSocket frame
    static WebSocketFrame parse_frame(const std::vector<uint8_t>& data) {
        WebSocketFrame frame;
        size_t offset = 0;
        
        if (data.size() < 2) {
            throw std::runtime_error("Frame too short");
        }
        
        // First byte: FIN, RSV, Opcode
        uint8_t first_byte = data[offset++];
        frame.fin = (first_byte & 0x80) != 0;
        frame.rsv1 = (first_byte & 0x40) != 0;
        frame.rsv2 = (first_byte & 0x20) != 0;
        frame.rsv3 = (first_byte & 0x10) != 0;
        frame.opcode = static_cast<WebSocketOpcode>(first_byte & 0x0F);
        
        // Second byte: MASK, Payload length
        uint8_t second_byte = data[offset++];
        frame.mask = (second_byte & 0x80) != 0;
        uint8_t payload_len = second_byte & 0x7F;
        
        // Extended payload length
        if (payload_len == 126) {
            if (data.size() < offset + 2) {
                throw std::runtime_error("Frame too short for extended length");
            }
            frame.payload_length = (data[offset] << 8) | data[offset + 1];
            offset += 2;
        } else if (payload_len == 127) {
            if (data.size() < offset + 8) {
                throw std::runtime_error("Frame too short for extended length");
            }
            frame.payload_length = 0;
            for (int i = 0; i < 8; i++) {
                frame.payload_length = (frame.payload_length << 8) | data[offset + i];
            }
            offset += 8;
        } else {
            frame.payload_length = payload_len;
        }
        
        // Masking key
        if (frame.mask) {
            if (data.size() < offset + 4) {
                throw std::runtime_error("Frame too short for masking key");
            }
            frame.masking_key = (data[offset] << 24) | (data[offset + 1] << 16) | 
                               (data[offset + 2] << 8) | data[offset + 3];
            offset += 4;
        }
        
        // Payload
        if (data.size() < offset + frame.payload_length) {
            throw std::runtime_error("Frame too short for payload");
        }
        
        frame.payload.resize(frame.payload_length);
        for (uint64_t i = 0; i < frame.payload_length; i++) {
            frame.payload[i] = data[offset + i];
            if (frame.mask) {
                frame.payload[i] ^= ((frame.masking_key >> (8 * (3 - (i % 4)))) & 0xFF);
            }
        }
        
        return frame;
    }
    
    // Create WebSocket frame
    static std::vector<uint8_t> create_frame(WebSocketOpcode opcode, const std::string& payload, bool fin = true) {
        std::vector<uint8_t> frame;
        
        // First byte: FIN, RSV, Opcode
        uint8_t first_byte = static_cast<uint8_t>(opcode);
        if (fin) {
            first_byte |= 0x80;
        }
        frame.push_back(first_byte);
        
        // Payload length
        uint64_t payload_len = payload.length();
        if (payload_len < 126) {
            frame.push_back(static_cast<uint8_t>(payload_len));
        } else if (payload_len <= 0xFFFF) {
            frame.push_back(126);
            frame.push_back((payload_len >> 8) & 0xFF);
            frame.push_back(payload_len & 0xFF);
        } else {
            frame.push_back(127);
            for (int i = 7; i >= 0; i--) {
                frame.push_back((payload_len >> (8 * i)) & 0xFF);
            }
        }
        
        // Payload
        for (char c : payload) {
            frame.push_back(static_cast<uint8_t>(c));
        }
        
        return frame;
    }
};

class WebSocketServer {
private:
    int server_fd;
    struct sockaddr_in server_addr;
    
public:
    WebSocketServer() {
        // Create socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            throw std::runtime_error("Socket creation failed");
        }
        
        // Set socket options
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
        
        // Bind socket
        if (bind(server_fd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(server_fd);
            throw std::runtime_error("Bind failed");
        }
        
        // Listen
        if (listen(server_fd, 5) < 0) {
            close(server_fd);
            throw std::runtime_error("Listen failed");
        }
        
        std::cout << "WebSocket Server listening on port " << PORT << std::endl;
    }
    
    ~WebSocketServer() {
        close(server_fd);
    }
    
    void handleWebSocketClient(int client_socket) {
        char buffer[BUFFER_SIZE];
        
        // Read HTTP upgrade request
        ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            close(client_socket);
            return;
        }
        
        buffer[bytes_received] = '\0';
        std::string request(buffer);
        
        std::cout << "Received HTTP request:\n" << request << std::endl;
        
        // Parse headers
        auto headers = WebSocketUtils::parse_headers(request);
        
        // Validate WebSocket upgrade request
        if (headers["Upgrade"] != "websocket" || headers["Connection"].find("Upgrade") == std::string::npos) {
            std::cout << "Invalid WebSocket upgrade request" << std::endl;
            close(client_socket);
            return;
        }
        
        // Generate accept key
        std::string websocket_key = headers["Sec-WebSocket-Key"];
        std::string accept_key = WebSocketUtils::generate_accept_key(websocket_key);
        
        // Send handshake response
        std::string response = WebSocketUtils::create_handshake_response(accept_key);
        send(client_socket, response.c_str(), response.length(), 0);
        
        std::cout << "WebSocket handshake completed!" << std::endl;
        
        // Handle WebSocket communication
        while (true) {
            std::vector<uint8_t> frame_data(BUFFER_SIZE);
            ssize_t bytes = recv(client_socket, frame_data.data(), BUFFER_SIZE, 0);
            
            if (bytes <= 0) {
                std::cout << "Client disconnected" << std::endl;
                break;
            }
            
            frame_data.resize(bytes);
            
            try {
                WebSocketFrame frame = WebSocketUtils::parse_frame(frame_data);
                
                if (frame.opcode == WebSocketOpcode::TEXT) {
                    std::string message(frame.payload.begin(), frame.payload.end());
                    std::cout << "Received: " << message << std::endl;
                    
                    // Echo the message back
                    std::string echo_msg = "Echo: " + message;
                    auto response_frame = WebSocketUtils::create_frame(WebSocketOpcode::TEXT, echo_msg);
                    send(client_socket, response_frame.data(), response_frame.size(), 0);
                    
                } else if (frame.opcode == WebSocketOpcode::CLOSE) {
                    std::cout << "Received close frame" << std::endl;
                    auto close_frame = WebSocketUtils::create_frame(WebSocketOpcode::CLOSE, "");
                    send(client_socket, close_frame.data(), close_frame.size(), 0);
                    break;
                    
                } else if (frame.opcode == WebSocketOpcode::PING) {
                    std::cout << "Received ping" << std::endl;
                    auto pong_frame = WebSocketUtils::create_frame(WebSocketOpcode::PONG, "");
                    send(client_socket, pong_frame.data(), pong_frame.size(), 0);
                }
                
            } catch (const std::exception& e) {
                std::cerr << "Error parsing frame: " << e.what() << std::endl;
                break;
            }
        }
        
        close(client_socket);
    }
    
    void start() {
        std::cout << "WebSocket server started. Connect using a WebSocket client." << std::endl;
        std::cout << "You can test with a simple HTML page or WebSocket client." << std::endl;
        
        while (true) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_socket < 0) {
                std::cerr << "Error accepting connection" << std::endl;
                continue;
            }
            
            std::cout << "New client connected" << std::endl;
            
            // Handle client in separate thread
            std::thread client_thread(&WebSocketServer::handleWebSocketClient, this, client_socket);
            client_thread.detach();
        }
    }
};

// Simple WebSocket client for testing
class WebSocketClient {
private:
    int client_fd;
    struct sockaddr_in server_addr;
    
public:
    WebSocketClient() {
        client_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd < 0) {
            throw std::runtime_error("Socket creation failed");
        }
        
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    }
    
    ~WebSocketClient() {
        close(client_fd);
    }
    
    bool connect_to_server() {
        if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Connection failed" << std::endl;
            return false;
        }
        
        // Send WebSocket upgrade request
        std::string request = 
            "GET / HTTP/1.1\r\n"
            "Host: localhost:8080\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
            "Sec-WebSocket-Version: 13\r\n"
            "\r\n";
        
        send(client_fd, request.c_str(), request.length(), 0);
        
        // Read response
        char buffer[BUFFER_SIZE];
        ssize_t bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes <= 0) {
            return false;
        }
        
        buffer[bytes] = '\0';
        std::string response(buffer);
        
        if (response.find("101 Switching Protocols") != std::string::npos) {
            std::cout << "WebSocket connection established!" << std::endl;
            return true;
        }
        
        return false;
    }
    
    void sendMessage(const std::string& message) {
        auto frame = WebSocketUtils::create_frame(WebSocketOpcode::TEXT, message);
        
        // Add masking for client frames (required by WebSocket protocol)
        frame[1] |= 0x80; // Set mask bit
        
        // Add masking key (simplified - use random in production)
        uint32_t mask = 0x12345678;
        frame.insert(frame.begin() + 2, {
            static_cast<uint8_t>((mask >> 24) & 0xFF),
            static_cast<uint8_t>((mask >> 16) & 0xFF),
            static_cast<uint8_t>((mask >> 8) & 0xFF),
            static_cast<uint8_t>(mask & 0xFF)
        });
        
        // Mask payload
        size_t payload_start = frame.size() - message.length();
        for (size_t i = 0; i < message.length(); i++) {
            frame[payload_start + i] ^= ((mask >> (8 * (3 - (i % 4)))) & 0xFF);
        }
        
        send(client_fd, frame.data(), frame.size(), 0);
        std::cout << "Sent: " << message << std::endl;
    }
    
    void receiveMessage() {
        std::vector<uint8_t> buffer(BUFFER_SIZE);
        ssize_t bytes = recv(client_fd, buffer.data(), BUFFER_SIZE, 0);
        
        if (bytes > 0) {
            buffer.resize(bytes);
            try {
                WebSocketFrame frame = WebSocketUtils::parse_frame(buffer);
                if (frame.opcode == WebSocketOpcode::TEXT) {
                    std::string message(frame.payload.begin(), frame.payload.end());
                    std::cout << "Received: " << message << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing response: " << e.what() << std::endl;
            }
        }
    }
    
    void testCommunication() {
        if (!connect_to_server()) {
            return;
        }
        
        std::vector<std::string> testMessages = {
            "Hello WebSocket!",
            "This is a real-time message",
            "WebSockets are awesome!",
            "Full-duplex communication"
        };
        
        for (const auto& msg : testMessages) {
            sendMessage(msg);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            receiveMessage();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
};

// Function to create a simple HTML test page
void createTestHTML() {
    std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>WebSocket Test</title>
</head>
<body>
    <h1>WebSocket Test Page</h1>
    <div id="messages"></div>
    <input type="text" id="messageInput" placeholder="Enter message">
    <button onclick="sendMessage()">Send</button>
    <button onclick="connect()">Connect</button>
    <button onclick="disconnect()">Disconnect</button>

    <script>
        let socket = null;
        
        function connect() {
            socket = new WebSocket('ws://localhost:8080');
            
            socket.onopen = function(event) {
                addMessage('Connected to WebSocket server');
            };
            
            socket.onmessage = function(event) {
                addMessage('Received: ' + event.data);
            };
            
            socket.onclose = function(event) {
                addMessage('Disconnected from server');
            };
            
            socket.onerror = function(error) {
                addMessage('Error: ' + error);
            };
        }
        
        function disconnect() {
            if (socket) {
                socket.close();
            }
        }
        
        function sendMessage() {
            const input = document.getElementById('messageInput');
            if (socket && socket.readyState === WebSocket.OPEN) {
                socket.send(input.value);
                addMessage('Sent: ' + input.value);
                input.value = '';
            }
        }
        
        function addMessage(message) {
            const div = document.createElement('div');
            div.textContent = new Date().toLocaleTimeString() + ' - ' + message;
            document.getElementById('messages').appendChild(div);
        }
        
        document.getElementById('messageInput').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                sendMessage();
            }
        });
    </script>
</body>
</html>
)";
    
    std::ofstream file("websocket_test.html");
    file << html;
    file.close();
    
    std::cout << "Created websocket_test.html for testing" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string mode = argv[1];
        
        if (mode == "server") {
            try {
                WebSocketServer server;
                server.start();
            } catch (const std::exception& e) {
                std::cerr << "Server error: " << e.what() << std::endl;
                return 1;
            }
        } else if (mode == "client") {
            try {
                WebSocketClient client;
                client.testCommunication();
            } catch (const std::exception& e) {
                std::cerr << "Client error: " << e.what() << std::endl;
                return 1;
            }
        } else if (mode == "html") {
            createTestHTML();
            std::cout << "Open websocket_test.html in your browser to test WebSocket connection" << std::endl;
        } else {
            std::cout << "Usage: " << argv[0] << " [server|client|html]" << std::endl;
            std::cout << "  server - Run WebSocket server" << std::endl;
            std::cout << "  client - Run test client" << std::endl;
            std::cout << "  html   - Create HTML test page" << std::endl;
            return 1;
        }
    } else {
        std::cout << "Usage: " << argv[0] << " [server|client|html]" << std::endl;
        std::cout << "  server - Run WebSocket server" << std::endl;
        std::cout << "  client - Run test client" << std::endl;
        std::cout << "  html   - Create HTML test page" << std::endl;
        return 1;
    }
    
    return 0;
}

/*
COMPILATION AND USAGE:

1. Install OpenSSL development libraries:
   sudo apt-get install libssl-dev  # Ubuntu/Debian
   brew install openssl             # macOS

2. Compile the program:
   g++ -std=c++11 -o websocket_program web_socket.cpp -pthread -lssl -lcrypto

3. Run the server:
   ./websocket_program server

4. Test with the client:
   ./websocket_program client

5. Create HTML test page:
   ./websocket_program html
   (Then open websocket_test.html in a browser)

KEY CONCEPTS DEMONSTRATED:

1. WebSocket Protocol:
   - HTTP upgrade handshake
   - Frame-based communication
   - Masking for client-to-server messages
   - Different frame types (text, binary, close, ping, pong)

2. WebSocket Handshake:
   - Client sends HTTP upgrade request with Sec-WebSocket-Key
   - Server responds with 101 status and Sec-WebSocket-Accept
   - Uses SHA-1 hash and Base64 encoding

3. Frame Structure:
   - FIN bit (final fragment)
   - Opcode (frame type)
   - Mask bit and masking key
   - Payload length (with extended length encoding)
   - Payload data

4. Real-time Communication:
   - Full-duplex communication
   - Low-latency messaging
   - Connection persistence
   - Automatic reconnection handling

5. Production Considerations:
   - Proper error handling
   - Frame fragmentation support
   - Compression extensions
   - Authentication and authorization
   - Rate limiting and security

WebSockets are ideal for:
- Real-time chat applications
- Live data feeds
- Gaming applications
- Collaborative editing
- Live notifications
*/
