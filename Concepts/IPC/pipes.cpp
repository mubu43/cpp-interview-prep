#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <thread>
#include <chrono>
#include <vector>
#include <sstream>

class PipeManager {
private:
    int read_fd_;
    int write_fd_;
    bool is_named_;
    std::string pipe_name_;

public:
    // Anonymous pipe constructor
    PipeManager() : read_fd_(-1), write_fd_(-1), is_named_(false) {}
    
    // Named pipe constructor
    PipeManager(const std::string& pipe_name) 
        : read_fd_(-1), write_fd_(-1), is_named_(true), pipe_name_(pipe_name) {}
    
    ~PipeManager() {
        cleanup();
    }
    
    // Create anonymous pipe (for parent-child communication)
    bool createAnonymousPipe() {
        int pipe_fds[2];
        if (pipe(pipe_fds) == -1) {
            perror("pipe");
            return false;
        }
        
        read_fd_ = pipe_fds[0];
        write_fd_ = pipe_fds[1];
        
        std::cout << "Anonymous pipe created (read_fd=" << read_fd_ 
                  << ", write_fd=" << write_fd_ << ")" << std::endl;
        return true;
    }
    
    // Create named pipe (FIFO)
    bool createNamedPipe(mode_t mode = 0666) {
        if (!is_named_) {
            std::cerr << "This is not a named pipe manager" << std::endl;
            return false;
        }
        
        // Remove existing pipe if it exists
        unlink(pipe_name_.c_str());
        
        if (mkfifo(pipe_name_.c_str(), mode) == -1) {
            perror("mkfifo");
            return false;
        }
        
        std::cout << "Named pipe created: " << pipe_name_ << std::endl;
        return true;
    }
    
    // Open named pipe for writing
    bool openForWrite() {
        if (!is_named_) {
            std::cerr << "Cannot open anonymous pipe this way" << std::endl;
            return false;
        }
        
        write_fd_ = open(pipe_name_.c_str(), O_WRONLY);
        if (write_fd_ == -1) {
            perror("open for write");
            return false;
        }
        
        std::cout << "Opened pipe for writing: " << pipe_name_ << std::endl;
        return true;
    }
    
    // Open named pipe for reading
    bool openForRead() {
        if (!is_named_) {
            std::cerr << "Cannot open anonymous pipe this way" << std::endl;
            return false;
        }
        
        read_fd_ = open(pipe_name_.c_str(), O_RDONLY);
        if (read_fd_ == -1) {
            perror("open for read");
            return false;
        }
        
        std::cout << "Opened pipe for reading: " << pipe_name_ << std::endl;
        return true;
    }
    
    // Write data to pipe
    bool writeData(const std::string& data) {
        if (write_fd_ == -1) {
            std::cerr << "Write file descriptor not available" << std::endl;
            return false;
        }
        
        ssize_t bytes_written = write(write_fd_, data.c_str(), data.length());
        if (bytes_written == -1) {
            perror("write");
            return false;
        }
        
        std::cout << "Written " << bytes_written << " bytes: " << data << std::endl;
        return true;
    }
    
    // Read data from pipe
    bool readData(std::string& data, size_t max_size = 1024) {
        if (read_fd_ == -1) {
            std::cerr << "Read file descriptor not available" << std::endl;
            return false;
        }
        
        std::vector<char> buffer(max_size);
        ssize_t bytes_read = read(read_fd_, buffer.data(), max_size - 1);
        
        if (bytes_read == -1) {
            perror("read");
            return false;
        }
        
        if (bytes_read == 0) {
            std::cout << "End of file reached" << std::endl;
            return false;
        }
        
        buffer[bytes_read] = '\0';
        data = std::string(buffer.data());
        
        std::cout << "Read " << bytes_read << " bytes: " << data << std::endl;
        return true;
    }
    
    // Close write end (useful for signaling end of data)
    void closeWrite() {
        if (write_fd_ != -1) {
            close(write_fd_);
            write_fd_ = -1;
            std::cout << "Write end closed" << std::endl;
        }
    }
    
    // Close read end
    void closeRead() {
        if (read_fd_ != -1) {
            close(read_fd_);
            read_fd_ = -1;
            std::cout << "Read end closed" << std::endl;
        }
    }
    
    // Get file descriptors (for use with fork)
    int getReadFd() const { return read_fd_; }
    int getWriteFd() const { return write_fd_; }
    
    void cleanup() {
        closeRead();
        closeWrite();
        
        if (is_named_ && !pipe_name_.empty()) {
            unlink(pipe_name_.c_str());
            std::cout << "Named pipe removed: " << pipe_name_ << std::endl;
        }
    }
};

// Bidirectional pipe communication
class BidirectionalPipe {
private:
    PipeManager parent_to_child_;
    PipeManager child_to_parent_;

public:
    bool initialize() {
        return parent_to_child_.createAnonymousPipe() && 
               child_to_parent_.createAnonymousPipe();
    }
    
    // Setup for parent process
    void setupParent() {
        // Parent writes to parent_to_child, reads from child_to_parent
        parent_to_child_.closeRead();
        child_to_parent_.closeWrite();
    }
    
    // Setup for child process
    void setupChild() {
        // Child reads from parent_to_child, writes to child_to_parent
        parent_to_child_.closeWrite();
        child_to_parent_.closeRead();
    }
    
    // Parent methods
    bool parentWrite(const std::string& data) {
        return parent_to_child_.writeData(data);
    }
    
    bool parentRead(std::string& data) {
        return child_to_parent_.readData(data);
    }
    
    // Child methods
    bool childWrite(const std::string& data) {
        return child_to_parent_.writeData(data);
    }
    
    bool childRead(std::string& data) {
        return parent_to_child_.readData(data);
    }
};

// Pipe-based command execution
class PipeCommand {
public:
    static std::string executeCommand(const std::string& command) {
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            return "Error: Failed to execute command";
        }
        
        std::stringstream result;
        char buffer[256];
        
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result << buffer;
        }
        
        int exit_code = pclose(pipe);
        
        if (exit_code != 0) {
            result << "\nCommand exited with code: " << exit_code;
        }
        
        return result.str();
    }
    
    static bool executeCommandAsync(const std::string& command, 
                                   std::function<void(const std::string&)> callback) {
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            return false;
        }
        
        std::thread reader([pipe, callback]() {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                callback(std::string(buffer));
            }
            pclose(pipe);
        });
        
        reader.detach();
        return true;
    }
};

// Producer-Consumer pattern using pipes
class PipeProducerConsumer {
private:
    PipeManager pipe_;
    std::string pipe_name_;

public:
    PipeProducerConsumer(const std::string& pipe_name) 
        : pipe_(pipe_name), pipe_name_(pipe_name) {}
    
    // Producer process
    void producer(const std::vector<std::string>& data) {
        if (!pipe_.createNamedPipe()) {
            std::cerr << "Failed to create pipe" << std::endl;
            return;
        }
        
        if (!pipe_.openForWrite()) {
            std::cerr << "Failed to open pipe for writing" << std::endl;
            return;
        }
        
        std::cout << "Producer started, sending " << data.size() << " items" << std::endl;
        
        for (const auto& item : data) {
            pipe_.writeData(item + "\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Signal end of data
        pipe_.writeData("EOF\n");
        pipe_.closeWrite();
        
        std::cout << "Producer finished" << std::endl;
    }
    
    // Consumer process
    void consumer() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        if (!pipe_.openForRead()) {
            std::cerr << "Failed to open pipe for reading" << std::endl;
            return;
        }
        
        std::cout << "Consumer started, waiting for data" << std::endl;
        
        std::string data;
        while (pipe_.readData(data)) {
            // Remove newline
            if (!data.empty() && data.back() == '\n') {
                data.pop_back();
            }
            
            if (data == "EOF") {
                std::cout << "End of data received" << std::endl;
                break;
            }
            
            std::cout << "Consumed: " << data << std::endl;
            
            // Simulate processing time
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        std::cout << "Consumer finished" << std::endl;
    }
};

void demonstrateAnonymousPipes() {
    std::cout << "=== Anonymous Pipe Demonstration ===\n\n";
    
    PipeManager pipe;
    if (!pipe.createAnonymousPipe()) {
        std::cerr << "Failed to create pipe" << std::endl;
        return;
    }
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process (reader)
        pipe.closeWrite();  // Child only reads
        
        std::cout << "Child process reading from pipe:" << std::endl;
        
        std::string data;
        while (pipe.readData(data)) {
            std::cout << "Child received: " << data << std::endl;
        }
        
        exit(0);
        
    } else if (pid > 0) {
        // Parent process (writer)
        pipe.closeRead();   // Parent only writes
        
        std::cout << "Parent process writing to pipe:" << std::endl;
        
        std::vector<std::string> messages = {
            "Hello from parent!",
            "Pipes are simple IPC",
            "This is message 3",
            "Final message"
        };
        
        for (const auto& msg : messages) {
            pipe.writeData(msg);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        pipe.closeWrite();  // Signal end of data
        
        // Wait for child to complete
        int status;
        wait(&status);
        
        std::cout << "Child process completed" << std::endl;
        
    } else {
        perror("fork failed");
    }
}

void demonstrateBidirectionalPipes() {
    std::cout << "\n=== Bidirectional Pipe Demonstration ===\n\n";
    
    BidirectionalPipe biPipe;
    if (!biPipe.initialize()) {
        std::cerr << "Failed to initialize bidirectional pipe" << std::endl;
        return;
    }
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        biPipe.setupChild();
        
        std::cout << "Child process started" << std::endl;
        
        // Child echoes messages with modification
        std::string data;
        while (biPipe.childRead(data)) {
            std::cout << "Child received: " << data << std::endl;
            
            if (data == "quit") {
                biPipe.childWrite("Child quitting");
                break;
            }
            
            std::string response = "Echo: " + data;
            biPipe.childWrite(response);
        }
        
        exit(0);
        
    } else if (pid > 0) {
        // Parent process
        biPipe.setupParent();
        
        std::cout << "Parent process started" << std::endl;
        
        std::vector<std::string> messages = {
            "Hello child",
            "How are you?",
            "This is a test",
            "quit"
        };
        
        for (const auto& msg : messages) {
            biPipe.parentWrite(msg);
            
            std::string response;
            if (biPipe.parentRead(response)) {
                std::cout << "Parent received: " << response << std::endl;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        // Wait for child to complete
        int status;
        wait(&status);
        
        std::cout << "Communication completed" << std::endl;
        
    } else {
        perror("fork failed");
    }
}

void demonstrateNamedPipes() {
    std::cout << "\n=== Named Pipe (FIFO) Demonstration ===\n\n";
    
    const std::string pipe_name = "/tmp/demo_fifo";
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process (consumer)
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        PipeProducerConsumer consumer(pipe_name);
        consumer.consumer();
        
        exit(0);
        
    } else if (pid > 0) {
        // Parent process (producer)
        std::vector<std::string> data = {
            "Item 1: First data packet",
            "Item 2: Second data packet", 
            "Item 3: Third data packet",
            "Item 4: Fourth data packet",
            "Item 5: Final data packet"
        };
        
        PipeProducerConsumer producer(pipe_name);
        producer.producer(data);
        
        // Wait for child to complete
        int status;
        wait(&status);
        
        std::cout << "Producer-Consumer demonstration completed" << std::endl;
        
    } else {
        perror("fork failed");
    }
}

void demonstrateCommandPipes() {
    std::cout << "\n=== Command Execution with Pipes ===\n\n";
    
    // Simple command execution
    std::cout << "1. Synchronous command execution:" << std::endl;
    std::string result = PipeCommand::executeCommand("ls -la /tmp | head -5");
    std::cout << "Command output:\n" << result << std::endl;
    
    // Asynchronous command execution
    std::cout << "2. Asynchronous command execution:" << std::endl;
    
    PipeCommand::executeCommandAsync("ping -c 3 localhost", 
        [](const std::string& line) {
            std::cout << "Async output: " << line;
        });
    
    // Wait for async command to complete
    std::this_thread::sleep_for(std::chrono::seconds(4));
    
    // Process pipeline simulation
    std::cout << "\n3. Command pipeline:" << std::endl;
    std::string pipeline_result = PipeCommand::executeCommand(
        "echo 'apple\nbanana\ncherry' | sort | head -2");
    std::cout << "Pipeline output:\n" << pipeline_result << std::endl;
}

void benchmarkPipes() {
    std::cout << "\n=== Pipe Performance Benchmark ===\n";
    
    PipeManager pipe;
    if (!pipe.createAnonymousPipe()) {
        std::cerr << "Failed to create pipe for benchmark" << std::endl;
        return;
    }
    
    const int iterations = 10000;
    const std::string test_data = "Benchmark test data message";
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process (reader)
        pipe.closeWrite();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::string data;
        for (int i = 0; i < iterations; ++i) {
            pipe.readData(data);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Read " << iterations << " messages in " 
                  << duration.count() << " microseconds" << std::endl;
        std::cout << "Average read time: " 
                  << static_cast<double>(duration.count()) / iterations 
                  << " microseconds" << std::endl;
        
        exit(0);
        
    } else if (pid > 0) {
        // Parent process (writer)
        pipe.closeRead();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            pipe.writeData(test_data);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Wrote " << iterations << " messages in " 
                  << duration.count() << " microseconds" << std::endl;
        std::cout << "Average write time: " 
                  << static_cast<double>(duration.count()) / iterations 
                  << " microseconds" << std::endl;
        
        // Wait for child to complete
        int status;
        wait(&status);
        
    } else {
        perror("fork failed");
    }
}

int main() {
    demonstrateAnonymousPipes();
    demonstrateBidirectionalPipes();
    demonstrateNamedPipes();
    demonstrateCommandPipes();
    benchmarkPipes();
    
    std::cout << "\n=== Key Pipe Concepts ===\n";
    std::cout << "1. Simple and portable IPC mechanism\n";
    std::cout << "2. FIFO (First In, First Out) behavior\n";
    std::cout << "3. Unidirectional communication (anonymous pipes)\n";
    std::cout << "4. Can be used between unrelated processes (named pipes)\n";
    std::cout << "5. Automatic synchronization and buffering\n";
    std::cout << "6. Perfect for producer-consumer patterns\n";
    
    return 0;
}
