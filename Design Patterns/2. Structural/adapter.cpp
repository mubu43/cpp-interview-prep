#include <iostream>
#include <string>
#include <memory>
#include <vector>

// Target Interface - What the client expects
class MediaPlayer {
public:
    virtual ~MediaPlayer() = default;
    virtual void play(const std::string& audioType, const std::string& fileName) = 0;
};

// Adaptee 1 - Mp3Player (already compatible)
class Mp3Player {
public:
    void playMp3(const std::string& fileName) {
        std::cout << "Playing MP3 file: " << fileName << std::endl;
    }
};

// Adaptee 2 - Mp4Player (incompatible interface)
class Mp4Player {
public:
    void playMp4File(const std::string& fileName) {
        std::cout << "Playing MP4 file: " << fileName << std::endl;
    }
};

// Adaptee 3 - VlcPlayer (incompatible interface)
class VlcPlayer {
public:
    void playVlcMedia(const std::string& fileName) {
        std::cout << "Playing VLC media file: " << fileName << std::endl;
    }
};

// Object Adapter - Adapts Mp4Player and VlcPlayer to MediaPlayer interface
class MediaAdapter : public MediaPlayer {
private:
    std::unique_ptr<Mp4Player> mp4Player_;
    std::unique_ptr<VlcPlayer> vlcPlayer_;

public:
    MediaAdapter() 
        : mp4Player_(std::make_unique<Mp4Player>())
        , vlcPlayer_(std::make_unique<VlcPlayer>()) {}
    
    void play(const std::string& audioType, const std::string& fileName) override {
        if (audioType == "mp4") {
            mp4Player_->playMp4File(fileName);
        } else if (audioType == "vlc" || audioType == "avi") {
            vlcPlayer_->playVlcMedia(fileName);
        } else {
            std::cout << audioType << " format not supported by adapter" << std::endl;
        }
    }
};

// Client - AudioPlayer that uses the adapter
class AudioPlayer : public MediaPlayer {
private:
    std::unique_ptr<Mp3Player> mp3Player_;
    std::unique_ptr<MediaAdapter> adapter_;

public:
    AudioPlayer() 
        : mp3Player_(std::make_unique<Mp3Player>())
        , adapter_(std::make_unique<MediaAdapter>()) {}
    
    void play(const std::string& audioType, const std::string& fileName) override {
        if (audioType == "mp3") {
            mp3Player_->playMp3(fileName);
        } else {
            adapter_->play(audioType, fileName);
        }
    }
};

// Example 2: Class Adapter using Multiple Inheritance
// Target Interface for drawing
class Shape {
public:
    virtual ~Shape() = default;
    virtual void draw() = 0;
    virtual void getBounds() = 0;
};

// Legacy drawing library (Adaptee)
class LegacyLine {
public:
    void drawLine(int x1, int y1, int x2, int y2) {
        std::cout << "Drawing legacy line from (" << x1 << "," << y1 
                  << ") to (" << x2 << "," << y2 << ")" << std::endl;
    }
};

class LegacyRectangle {
public:
    void drawRectangle(int x, int y, int width, int height) {
        std::cout << "Drawing legacy rectangle at (" << x << "," << y 
                  << ") with size " << width << "x" << height << std::endl;
    }
};

// Class Adapter using inheritance
class LineAdapter : public Shape, private LegacyLine {
private:
    int x1_, y1_, x2_, y2_;

public:
    LineAdapter(int x1, int y1, int x2, int y2) 
        : x1_(x1), y1_(y1), x2_(x2), y2_(y2) {}
    
    void draw() override {
        drawLine(x1_, y1_, x2_, y2_);
    }
    
    void getBounds() override {
        std::cout << "Line bounds: (" << x1_ << "," << y1_ 
                  << ") to (" << x2_ << "," << y2_ << ")" << std::endl;
    }
};

class RectangleAdapter : public Shape, private LegacyRectangle {
private:
    int x_, y_, width_, height_;

public:
    RectangleAdapter(int x, int y, int width, int height) 
        : x_(x), y_(y), width_(width), height_(height) {}
    
    void draw() override {
        drawRectangle(x_, y_, width_, height_);
    }
    
    void getBounds() override {
        std::cout << "Rectangle bounds: (" << x_ << "," << y_ 
                  << ") with size " << width_ << "x" << height_ << std::endl;
    }
};

// Example 3: Template Adapter Pattern
template<typename Adaptee>
class GenericAdapter {
private:
    Adaptee adaptee_;

public:
    template<typename... Args>
    GenericAdapter(Args&&... args) : adaptee_(std::forward<Args>(args)...) {}
    
    // Adapt specific methods based on Adaptee type
    void execute() {
        if constexpr (std::is_same_v<Adaptee, Mp4Player>) {
            adaptee_.playMp4File("default.mp4");
        } else if constexpr (std::is_same_v<Adaptee, VlcPlayer>) {
            adaptee_.playVlcMedia("default.vlc");
        } else {
            static_assert(sizeof(Adaptee) == 0, "Unsupported adaptee type");
        }
    }
    
    Adaptee& getAdaptee() { return adaptee_; }
};

// Example 4: Function Adapter using std::function
class FunctionAdapter {
private:
    std::function<void(const std::string&)> playFunction_;

public:
    template<typename T>
    FunctionAdapter(T& player) {
        if constexpr (std::is_same_v<T, Mp3Player>) {
            playFunction_ = [&player](const std::string& file) {
                player.playMp3(file);
            };
        } else if constexpr (std::is_same_v<T, Mp4Player>) {
            playFunction_ = [&player](const std::string& file) {
                player.playMp4File(file);
            };
        } else if constexpr (std::is_same_v<T, VlcPlayer>) {
            playFunction_ = [&player](const std::string& file) {
                player.playVlcMedia(file);
            };
        }
    }
    
    void play(const std::string& fileName) {
        if (playFunction_) {
            playFunction_(fileName);
        } else {
            std::cout << "No player function set" << std::endl;
        }
    }
};

// Example 5: Two-way Adapter
class ModernPrinter {
public:
    virtual ~ModernPrinter() = default;
    virtual void print(const std::string& document) = 0;
    virtual void printColor(const std::string& document, const std::string& color) = 0;
};

class LegacyPrinter {
public:
    void oldPrint(const char* text) {
        std::cout << "Legacy printer: " << text << std::endl;
    }
};

// Two-way adapter
class PrinterAdapter : public ModernPrinter {
private:
    LegacyPrinter& legacyPrinter_;

public:
    explicit PrinterAdapter(LegacyPrinter& printer) : legacyPrinter_(printer) {}
    
    void print(const std::string& document) override {
        legacyPrinter_.oldPrint(document.c_str());
    }
    
    void printColor(const std::string& document, const std::string& color) override {
        std::string coloredDoc = "[" + color + "] " + document;
        legacyPrinter_.oldPrint(coloredDoc.c_str());
    }
    
    // Additional method to expose legacy functionality
    void printLegacy(const char* text) {
        legacyPrinter_.oldPrint(text);
    }
};

// Example 6: Interface Segregation with Adapter
class FileReader {
public:
    virtual ~FileReader() = default;
    virtual std::string readFile(const std::string& filename) = 0;
};

class NetworkReader {
public:
    virtual ~NetworkReader() = default;
    virtual std::string readFromUrl(const std::string& url) = 0;
};

// Unified interface that clients want
class DataReader {
public:
    virtual ~DataReader() = default;
    virtual std::string readData(const std::string& source) = 0;
};

// Concrete implementations
class LocalFileReader : public FileReader {
public:
    std::string readFile(const std::string& filename) override {
        return "File content from: " + filename;
    }
};

class HttpReader : public NetworkReader {
public:
    std::string readFromUrl(const std::string& url) override {
        return "HTTP content from: " + url;
    }
};

// Adapters for unified interface
class FileReaderAdapter : public DataReader {
private:
    std::unique_ptr<FileReader> fileReader_;

public:
    explicit FileReaderAdapter(std::unique_ptr<FileReader> reader) 
        : fileReader_(std::move(reader)) {}
    
    std::string readData(const std::string& source) override {
        return fileReader_->readFile(source);
    }
};

class NetworkReaderAdapter : public DataReader {
private:
    std::unique_ptr<NetworkReader> networkReader_;

public:
    explicit NetworkReaderAdapter(std::unique_ptr<NetworkReader> reader) 
        : networkReader_(std::move(reader)) {}
    
    std::string readData(const std::string& source) override {
        return networkReader_->readFromUrl(source);
    }
};

void demonstrateBasicAdapter() {
    std::cout << "=== Basic Adapter Pattern ===\n\n";
    
    std::cout << "1. Object Adapter Pattern:\n";
    AudioPlayer player;
    
    player.play("mp3", "song.mp3");
    player.play("mp4", "video.mp4");
    player.play("vlc", "movie.vlc");
    player.play("avi", "clip.avi");
    player.play("wav", "sound.wav");  // Unsupported format
    
    std::cout << "\n2. Class Adapter Pattern:\n";
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<LineAdapter>(0, 0, 10, 10));
    shapes.push_back(std::make_unique<RectangleAdapter>(5, 5, 20, 15));
    
    for (const auto& shape : shapes) {
        shape->draw();
        shape->getBounds();
    }
    std::cout << std::endl;
}

void demonstrateAdvancedAdapters() {
    std::cout << "3. Template Adapter Pattern:\n";
    GenericAdapter<Mp4Player> mp4Adapter;
    GenericAdapter<VlcPlayer> vlcAdapter;
    
    mp4Adapter.execute();
    vlcAdapter.execute();
    
    std::cout << "\n4. Function Adapter Pattern:\n";
    Mp3Player mp3Player;
    Mp4Player mp4Player;
    VlcPlayer vlcPlayer;
    
    FunctionAdapter mp3Adapter(mp3Player);
    FunctionAdapter mp4FuncAdapter(mp4Player);
    FunctionAdapter vlcFuncAdapter(vlcPlayer);
    
    mp3Adapter.play("song.mp3");
    mp4FuncAdapter.play("video.mp4");
    vlcFuncAdapter.play("movie.vlc");
    
    std::cout << "\n5. Two-way Adapter Pattern:\n";
    LegacyPrinter legacyPrinter;
    PrinterAdapter printerAdapter(legacyPrinter);
    
    printerAdapter.print("Modern document");
    printerAdapter.printColor("Colored document", "RED");
    printerAdapter.printLegacy("Direct legacy call");
    
    std::cout << "\n6. Interface Segregation with Adapters:\n";
    std::vector<std::unique_ptr<DataReader>> readers;
    
    readers.push_back(std::make_unique<FileReaderAdapter>(
        std::make_unique<LocalFileReader>()));
    readers.push_back(std::make_unique<NetworkReaderAdapter>(
        std::make_unique<HttpReader>()));
    
    std::vector<std::string> sources = {"config.txt", "https://api.example.com/data"};
    
    for (size_t i = 0; i < readers.size(); ++i) {
        std::cout << readers[i]->readData(sources[i]) << std::endl;
    }
    std::cout << std::endl;
}

void demonstrateAdapterUseCases() {
    std::cout << "7. Real-world Adapter Scenarios:\n";
    
    // Scenario 1: Third-party library integration
    std::cout << "Scenario 1: Integrating third-party media libraries\n";
    AudioPlayer universalPlayer;
    std::vector<std::pair<std::string, std::string>> playlist = {
        {"mp3", "song1.mp3"},
        {"mp4", "video1.mp4"},
        {"vlc", "movie1.vlc"},
        {"mp3", "song2.mp3"}
    };
    
    for (const auto& [format, filename] : playlist) {
        universalPlayer.play(format, filename);
    }
    
    std::cout << "\nScenario 2: Legacy system integration\n";
    LegacyPrinter oldPrinter;
    PrinterAdapter modernInterface(oldPrinter);
    
    // Modern application uses modern interface
    std::vector<std::string> documents = {
        "Report 1", "Invoice 2023", "Contract Draft"
    };
    
    for (const auto& doc : documents) {
        modernInterface.print(doc);
    }
    
    std::cout << "\nScenario 3: Data source abstraction\n";
    auto fileReader = std::make_unique<FileReaderAdapter>(
        std::make_unique<LocalFileReader>());
    auto webReader = std::make_unique<NetworkReaderAdapter>(
        std::make_unique<HttpReader>());
    
    // Client code doesn't need to know about specific reader types
    auto processData = [](DataReader& reader, const std::string& source) {
        std::string data = reader.readData(source);
        std::cout << "Processed: " << data << std::endl;
    };
    
    processData(*fileReader, "local_config.json");
    processData(*webReader, "https://api.service.com/config");
}

void demonstrateAdapterPattern() {
    std::cout << "=== Adapter Pattern Demonstration ===\n\n";
    
    demonstrateBasicAdapter();
    demonstrateAdvancedAdapters();
    demonstrateAdapterUseCases();
    
    std::cout << "=== Adapter Pattern Benefits ===\n";
    std::cout << "✓ Integrates incompatible interfaces\n";
    std::cout << "✓ Enables reuse of existing code\n";
    std::cout << "✓ Provides interface standardization\n";
    std::cout << "✓ Supports gradual system migration\n";
    std::cout << "✓ Decouples client from implementation details\n";
}

int main() {
    demonstrateAdapterPattern();
    return 0;
}
