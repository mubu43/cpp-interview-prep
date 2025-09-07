#include <iostream>
#include <string>
#include <cstring>
#include <mqueue.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <thread>
#include <chrono>
#include <vector>
#include <json/json.h>  // For JSON message serialization (optional)

// Message structure for typed communication
struct Message {
    enum Type {
        TEXT = 1,
        COMMAND = 2,
        DATA = 3,
        RESPONSE = 4
    };
    
    Type type;
    int sequence_id;
    size_t data_size;
    char data[256];
    
    Message() : type(TEXT), sequence_id(0), data_size(0) {
        memset(data, 0, sizeof(data));
    }
    
    Message(Type t, int seq, const std::string& content) 
        : type(t), sequence_id(seq) {
        strncpy(data, content.c_str(), sizeof(data) - 1);
        data_size = content.length();
    }
    
    std::string toString() const {
        return std::string(data, data_size);
    }
};

class MessageQueueManager {
private:
    const char* queue_name_;
    mqd_t mq_descriptor_;
    struct mq_attr attributes_;
    bool is_creator_;

public:
    MessageQueueManager(const char* queue_name, bool create = false) 
        : queue_name_(queue_name), mq_descriptor_(-1), is_creator_(create) {
        
        // Set queue attributes
        attributes_.mq_flags = 0;           // Blocking mode
        attributes_.mq_maxmsg = 10;         // Maximum messages in queue
        attributes_.mq_msgsize = sizeof(Message);  // Maximum message size
        attributes_.mq_curmsgs = 0;         // Current messages (read-only)
    }
    
    ~MessageQueueManager() {
        cleanup();
    }
    
    bool initialize() {
        if (is_creator_) {
            return createQueue();
        } else {
            return openQueue();
        }
    }

private:
    bool createQueue() {
        // Remove existing queue if it exists
        mq_unlink(queue_name_);
        
        // Create new message queue
        mq_descriptor_ = mq_open(queue_name_, 
                                O_CREAT | O_RDWR | O_NONBLOCK, 
                                0666, 
                                &attributes_);
        
        if (mq_descriptor_ == -1) {
            perror("mq_open (create)");
            return false;
        }
        
        std::cout << "Message queue created: " << queue_name_ << std::endl;
        return true;
    }
    
    bool openQueue() {
        // Open existing message queue
        mq_descriptor_ = mq_open(queue_name_, O_RDWR | O_NONBLOCK);
        
        if (mq_descriptor_ == -1) {
            perror("mq_open (open)");
            return false;
        }
        
        std::cout << "Opened existing message queue: " << queue_name_ << std::endl;
        return true;
    }

public:
    bool sendMessage(const Message& msg, unsigned int priority = 0) {
        if (mq_descriptor_ == -1) {
            std::cerr << "Queue not initialized\n";
            return false;
        }
        
        if (mq_send(mq_descriptor_, 
                   reinterpret_cast<const char*>(&msg), 
                   sizeof(Message), 
                   priority) == -1) {
            if (errno == EAGAIN) {
                std::cerr << "Queue is full\n";
            } else {
                perror("mq_send");
            }
            return false;
        }
        
        std::cout << "Sent message: type=" << msg.type 
                  << ", seq=" << msg.sequence_id 
                  << ", data=" << msg.toString() << std::endl;
        return true;
    }
    
    bool receiveMessage(Message& msg, unsigned int* priority = nullptr) {
        if (mq_descriptor_ == -1) {
            std::cerr << "Queue not initialized\n";
            return false;
        }
        
        unsigned int prio = 0;
        ssize_t bytes_read = mq_receive(mq_descriptor_,
                                       reinterpret_cast<char*>(&msg),
                                       sizeof(Message),
                                       &prio);
        
        if (bytes_read == -1) {
            if (errno == EAGAIN) {
                // No messages available (non-blocking mode)
                return false;
            } else {
                perror("mq_receive");
                return false;
            }
        }
        
        if (priority) {
            *priority = prio;
        }
        
        std::cout << "Received message: type=" << msg.type 
                  << ", seq=" << msg.sequence_id 
                  << ", data=" << msg.toString() 
                  << ", priority=" << prio << std::endl;
        return true;
    }
    
    // Blocking receive with timeout
    bool receiveMessageTimeout(Message& msg, int timeout_seconds) {
        if (mq_descriptor_ == -1) {
            std::cerr << "Queue not initialized\n";
            return false;
        }
        
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += timeout_seconds;
        
        ssize_t bytes_read = mq_timedreceive(mq_descriptor_,
                                           reinterpret_cast<char*>(&msg),
                                           sizeof(Message),
                                           nullptr,
                                           &timeout);
        
        if (bytes_read == -1) {
            if (errno == ETIMEDOUT) {
                std::cout << "Receive timeout after " << timeout_seconds << " seconds\n";
            } else {
                perror("mq_timedreceive");
            }
            return false;
        }
        
        std::cout << "Received message (with timeout): " << msg.toString() << std::endl;
        return true;
    }
    
    void getQueueInfo() {
        if (mq_descriptor_ == -1) {
            std::cerr << "Queue not initialized\n";
            return;
        }
        
        struct mq_attr current_attr;
        if (mq_getattr(mq_descriptor_, &current_attr) == -1) {
            perror("mq_getattr");
            return;
        }
        
        std::cout << "Queue Information:\n";
        std::cout << "  Max messages: " << current_attr.mq_maxmsg << "\n";
        std::cout << "  Max message size: " << current_attr.mq_msgsize << "\n";
        std::cout << "  Current messages: " << current_attr.mq_curmsgs << "\n";
        std::cout << "  Flags: " << current_attr.mq_flags << "\n";
    }
    
    void cleanup() {
        if (mq_descriptor_ != -1) {
            mq_close(mq_descriptor_);
            mq_descriptor_ = -1;
        }
        
        if (is_creator_) {
            mq_unlink(queue_name_);
            std::cout << "Message queue cleaned up: " << queue_name_ << std::endl;
        }
    }
};

// Advanced: Request-Response pattern
class RequestResponseClient {
private:
    MessageQueueManager request_queue_;
    MessageQueueManager response_queue_;
    int sequence_counter_;

public:
    RequestResponseClient() 
        : request_queue_("/request_queue", false),
          response_queue_("/response_queue", false),
          sequence_counter_(0) {}
    
    bool initialize() {
        return request_queue_.initialize() && response_queue_.initialize();
    }
    
    bool sendRequest(const std::string& request_data, std::string& response_data) {
        // Send request
        Message request(Message::COMMAND, ++sequence_counter_, request_data);
        if (!request_queue_.sendMessage(request, 1)) {  // High priority
            return false;
        }
        
        // Wait for response
        Message response;
        for (int attempts = 0; attempts < 10; ++attempts) {
            if (response_queue_.receiveMessage(response)) {
                if (response.sequence_id == sequence_counter_) {
                    response_data = response.toString();
                    return true;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cerr << "Timeout waiting for response\n";
        return false;
    }
};

class RequestResponseServer {
private:
    MessageQueueManager request_queue_;
    MessageQueueManager response_queue_;

public:
    RequestResponseServer() 
        : request_queue_("/request_queue", true),
          response_queue_("/response_queue", true) {}
    
    bool initialize() {
        return request_queue_.initialize() && response_queue_.initialize();
    }
    
    void processRequests() {
        std::cout << "Server started, waiting for requests...\n";
        
        while (true) {
            Message request;
            if (request_queue_.receiveMessage(request)) {
                // Process request
                std::string response_data = processRequest(request.toString());
                
                // Send response
                Message response(Message::RESPONSE, request.sequence_id, response_data);
                response_queue_.sendMessage(response);
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

private:
    std::string processRequest(const std::string& request) {
        std::cout << "Processing request: " << request << std::endl;
        
        // Simple echo server with timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        return "Echo: " + request + " (processed at " + std::to_string(time_t) + ")";
    }
};

void demonstrateBasicMessageQueue() {
    std::cout << "=== Basic Message Queue Demonstration ===\n\n";
    
    const char* queue_name = "/demo_queue";
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process (receiver)
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        MessageQueueManager receiver(queue_name, false);
        if (!receiver.initialize()) {
            std::cerr << "Child: Failed to open message queue\n";
            exit(1);
        }
        
        std::cout << "\nChild process receiving messages:\n";
        
        // Receive messages
        for (int i = 0; i < 5; ++i) {
            Message msg;
            while (!receiver.receiveMessage(msg)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        receiver.getQueueInfo();
        exit(0);
        
    } else if (pid > 0) {
        // Parent process (sender)
        MessageQueueManager sender(queue_name, true);
        if (!sender.initialize()) {
            std::cerr << "Parent: Failed to create message queue\n";
            return;
        }
        
        std::cout << "Parent process sending messages:\n";
        
        // Send messages with different priorities
        std::vector<std::pair<std::string, unsigned int>> messages = {
            {"Low priority message", 0},
            {"Normal message", 1},
            {"High priority message", 2},
            {"Another normal message", 1},
            {"Final message", 0}
        };
        
        for (size_t i = 0; i < messages.size(); ++i) {
            Message msg(Message::TEXT, static_cast<int>(i + 1), messages[i].first);
            sender.sendMessage(msg, messages[i].second);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        // Wait for child to complete
        int status;
        wait(&status);
        
        std::cout << "\nChild process completed\n";
        sender.getQueueInfo();
        
    } else {
        perror("fork failed");
    }
}

void demonstrateRequestResponse() {
    std::cout << "\n=== Request-Response Pattern Demonstration ===\n\n";
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process (server)
        RequestResponseServer server;
        if (!server.initialize()) {
            std::cerr << "Server: Failed to initialize\n";
            exit(1);
        }
        
        // Process requests for a limited time
        std::thread server_thread([&server]() {
            server.processRequests();
        });
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        exit(0);
        
    } else if (pid > 0) {
        // Parent process (client)
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        RequestResponseClient client;
        if (!client.initialize()) {
            std::cerr << "Client: Failed to initialize\n";
            return;
        }
        
        std::cout << "Client sending requests:\n";
        
        std::vector<std::string> requests = {
            "Hello Server",
            "What time is it?",
            "Process this data",
            "Final request"
        };
        
        for (const auto& request : requests) {
            std::string response;
            if (client.sendRequest(request, response)) {
                std::cout << "Request: " << request << " -> Response: " << response << std::endl;
            } else {
                std::cout << "Failed to get response for: " << request << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        // Wait for server to complete
        int status;
        wait(&status);
        
    } else {
        perror("fork failed");
    }
}

void benchmarkMessageQueue() {
    std::cout << "\n=== Message Queue Performance Benchmark ===\n";
    
    const char* queue_name = "/benchmark_queue";
    const int iterations = 10000;
    
    MessageQueueManager queue(queue_name, true);
    if (!queue.initialize()) {
        std::cerr << "Failed to initialize queue for benchmark\n";
        return;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Send messages
    for (int i = 0; i < iterations; ++i) {
        Message msg(Message::DATA, i, "Benchmark data " + std::to_string(i));
        while (!queue.sendMessage(msg)) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
    
    auto send_end = std::chrono::high_resolution_clock::now();
    
    // Receive messages
    for (int i = 0; i < iterations; ++i) {
        Message msg;
        while (!queue.receiveMessage(msg)) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    
    auto send_duration = std::chrono::duration_cast<std::chrono::microseconds>(send_end - start);
    auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Sent " << iterations << " messages in " << send_duration.count() << " μs\n";
    std::cout << "Round-trip time: " << total_duration.count() << " μs\n";
    std::cout << "Average send time: " << static_cast<double>(send_duration.count()) / iterations << " μs\n";
    std::cout << "Messages per second: " << (iterations * 1000000.0) / total_duration.count() << "\n";
}

int main() {
    demonstrateBasicMessageQueue();
    demonstrateRequestResponse();
    benchmarkMessageQueue();
    
    std::cout << "\n=== Key Message Queue Concepts ===\n";
    std::cout << "1. Reliable message delivery with persistence\n";
    std::cout << "2. Priority-based message ordering\n";
    std::cout << "3. Asynchronous communication pattern\n";
    std::cout << "4. Built-in synchronization and buffering\n";
    std::cout << "5. Perfect for producer-consumer scenarios\n";
    
    return 0;
}
