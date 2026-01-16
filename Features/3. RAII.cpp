// Demonstrating RAII (Resource Acquisition Is Initialization) in C++
// ---------------------------------------------------------------
// RAII is a design technique where resource ownership is tied to
// object lifetime. Resources are acquired in constructors and released
// in destructors, guaranteeing cleanup even on exceptions or early returns.

#include <iostream>
#include <string>
#include <stdexcept>
#include <mutex>
#include <chrono>
#include <thread>

// POSIX headers (Linux) for file descriptor demo
#include <fcntl.h>      // open
#include <unistd.h>     // close, write
#include <sys/stat.h>   // file modes

// 1) A simple RAII wrapper around a POSIX file descriptor
//    - Acquires the file resource in the constructor
//    - Releases the resource (close(fd)) in the destructor
//    - Non-copyable but movable to avoid double-close
class FileDescriptor {
public:
	// Acquire: open a file and own its descriptor
	explicit FileDescriptor(const std::string& path,
							int flags = O_CREAT | O_WRONLY | O_TRUNC,
							mode_t mode = 0644)
		: fd_(-1), path_(path) {
		int newfd = ::open(path.c_str(), flags, mode);
		if (newfd == -1) {
			throw std::runtime_error("Failed to open file: " + path);
		}
		fd_ = newfd;
		// Invariant: fd_ >= 0 means we own an open file descriptor
	}

	// Non-copyable: owning a raw resource should not be copied
	FileDescriptor(const FileDescriptor&) = delete;
	FileDescriptor& operator=(const FileDescriptor&) = delete;

	// Movable: transfer ownership safely
	FileDescriptor(FileDescriptor&& other) noexcept
		: fd_(other.fd_), path_(std::move(other.path_)) {
		other.fd_ = -1;
	}
	FileDescriptor& operator=(FileDescriptor&& other) noexcept {
		if (this != &other) {
			closeIfNeeded();
			fd_ = other.fd_;
			path_ = std::move(other.path_);
			other.fd_ = -1;
		}
		return *this;
	}

	// Release: guaranteed cleanup
	~FileDescriptor() {
		closeIfNeeded();
	}

	// Convenience method: write a line of text
	void writeLine(const std::string& line) {
		if (fd_ < 0) throw std::runtime_error("Invalid file descriptor");
		std::string data = line + "\n";
		ssize_t n = ::write(fd_, data.c_str(), data.size());
		if (n < 0) throw std::runtime_error("write() failed");
	}

	int get() const noexcept { return fd_; }
	const std::string& path() const noexcept { return path_; }

private:
	int fd_;
	std::string path_;

	void closeIfNeeded() noexcept {
		if (fd_ >= 0) {
			::close(fd_);
			fd_ = -1;
		}
	}
};

// 2) RAII for synchronization: std::lock_guard automatically unlocks
//    the mutex when the guard goes out of scope, even on exceptions.
std::mutex g_mutex;
int g_sharedCounter = 0;

void incrementWithLockGuard() {
	std::lock_guard<std::mutex> lock(g_mutex); // Acquire
	// Critical section
	++g_sharedCounter;
	// lock_guard releases automatically at end of scope (Release)
}

// 3) A tiny RAII scope timer that measures how long a scope lasts
class ScopeTimer {
public:
	explicit ScopeTimer(const char* label)
		: label_(label), start_(std::chrono::steady_clock::now()) {}
	~ScopeTimer() {
		auto end = std::chrono::steady_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_).count();
		std::cout << label_ << " took " << ms << " ms\n";
	}
private:
	const char* label_;
	std::chrono::steady_clock::time_point start_;
};

// Demonstration of RAII behavior on early returns and exceptions
void writeLogsWithEarlyReturn(bool earlyReturn) {
	ScopeTimer t("writeLogsWithEarlyReturn"); // Starts timer; auto-stops on scope exit
	FileDescriptor fd("/tmp/raii_demo.txt"); // Opens file; auto-closes on scope exit
	fd.writeLine("RAII demo: begin");

	if (earlyReturn) {
		fd.writeLine("Returning early...");
		return; // FileDescriptor destructor will still run and close the file
	}

	fd.writeLine("Doing some work...");
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	fd.writeLine("RAII demo: end");
}

void writeLogsWithException() {
	ScopeTimer t("writeLogsWithException");
	FileDescriptor fd("/tmp/raii_demo_exception.txt");
	fd.writeLine("Start");

	// Throwing an exception here will unwind the stack and call destructors
	throw std::runtime_error("Simulated failure: an exception occurred");
	// fd would have been closed by its destructor before the exception propagates
}

int main() {
	try {
		std::cout << "=== RAII Concept Demonstration ===\n\n";

		// 1) Basic RAII: file resource is managed automatically
		std::cout << "1) FileDescriptor RAII wrapper\n";
		writeLogsWithEarlyReturn(false);
		writeLogsWithEarlyReturn(true); // early return still cleans up
		std::cout << "Written logs to /tmp/raii_demo.txt\n\n";

		// 2) RAII for synchronization
		std::cout << "2) std::lock_guard for mutex RAII\n";
		std::thread t1(incrementWithLockGuard);
		std::thread t2(incrementWithLockGuard);
		t1.join();
		t2.join();
		std::cout << "Shared counter after guarded increments: " << g_sharedCounter << "\n\n";

		// 3) RAII under exceptions
		std::cout << "3) RAII guarantees cleanup on exceptions\n";
		try {
			writeLogsWithException();
		} catch (const std::exception& e) {
			std::cout << "Caught exception: " << e.what() << "\n";
		}
		std::cout << "Exception path cleaned up resources automatically\n\n";

		std::cout << "=== Key Takeaways ===\n";
		std::cout << "- Acquire resources in constructors; release in destructors\n";
		std::cout << "- Make owning types non-copyable, but movable\n";
		std::cout << "- Prefer standard RAII types (unique_ptr, lock_guard, iostreams)\n";
		std::cout << "- RAII provides strong exception safety and deterministic cleanup\n";
		return 0;

	} catch (const std::exception& e) {
		std::cerr << "Fatal error: " << e.what() << "\n";
		return 1;
	}
}

/*
How to build and run (Linux):

  g++ -std=c++17 -Wall -Wextra -O2 "Features/3. RAII.cpp" -o raii_demo
  ./raii_demo

What you'll see:
  - Logs written to /tmp/raii_demo.txt
  - An exception path that still cleans up /tmp/raii_demo_exception.txt
  - A shared counter updated safely using std::lock_guard

Why this demonstrates RAII:
  - FileDescriptor owns a raw OS resource (file descriptor). It acquires
	ownership in its constructor and releases it in its destructor.
  - ScopeTimer owns a timing resource and reports duration automatically.
  - std::lock_guard owns a mutex lock, releasing it at scope end.
  - Cleanup happens deterministically upon scope exit (normal, return, or exception).
*/

