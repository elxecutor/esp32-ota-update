# ESP32 Modular OTA Architecture Documentation

## Overview

This project provides a refactored, modular OTA (Over-The-Air) update system for ESP32 that decouples OTA logic from web servers and main application code. The modular design ensures OTA functionality persists across firmware updates and provides reliable, repeatable updates.

## Architecture

### Core Components

1. **OTACore** - Persistent OTA functionality that survives firmware updates
2. **NetworkManager** - WiFi connectivity and auto-reconnection management  
3. **OTAWebServer** - Dedicated web server for OTA operations
4. **ElegantOTACompat** - Backward compatibility layer for existing ElegantOTA code
5. **ModularOTA** - Main orchestrator that coordinates all components

### Key Features

- **Persistence**: OTA logic persists across firmware updates using RTC memory
- **Decoupling**: OTA functionality is completely separate from application code
- **Auto-reconnection**: Automatic WiFi reconnection with configurable intervals
- **Backward Compatibility**: Drop-in replacement for ElegantOTA
- **Memory Management**: Intelligent memory layout and usage monitoring
- **Security**: Optional HTTP authentication for OTA endpoints
- **Progress Monitoring**: Real-time upload progress tracking
- **Custom Endpoints**: Ability to add custom API endpoints to OTA server

## Memory Layout and Persistence

### RTC Memory Usage

The OTA system uses RTC (Real-Time Clock) memory to maintain state across reboots:

```cpp
struct RTCData {
    uint32_t magic;        // Magic number for validation
    bool otaEnabled;       // OTA enabled flag
    Status status;         // Current OTA status
    int progress;          // Upload progress
    uint32_t crc;         // Data integrity check
};
```

### Memory Layout Guidance

```
ESP32 Memory Layout for OTA Persistence:
┌─────────────────────────────────────┐
│ Application Code (Flash)            │
├─────────────────────────────────────┤
│ OTA Partition (Flash)               │
├─────────────────────────────────────┤
│ SPIFFS/LittleFS (Flash)             │
├─────────────────────────────────────┤
│ RAM (Heap)                          │
├─────────────────────────────────────┤
│ RTC Memory (Persistent)             │ ← OTA State Storage
└─────────────────────────────────────┘
```

### Recommended Partition Scheme

```ini
# platformio.ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
board_build.partitions = min_spiffs.csv  ; or custom partition table
```

## Integration Examples

### 1. Complete Modular Setup

```cpp
#include <ModularOTA.h>

ModularOTA::Config config = {
    .ssid = "YourWiFi",
    .password = "YourPassword",
    .enablePersistence = true,
    .serverPort = 3232,
    .otaPath = "/update"
};

void setup() {
    ModularOTA::begin(config);
}

void loop() {
    ModularOTA::handle();
}
```

### 2. Backward Compatibility (Drop-in Replacement)

```cpp
#include <ElegantOTACompat.h>
#include <NetworkManager.h>

void setup() {
    NetworkManager::begin("WiFiSSID", "WiFiPassword");
    NetworkManager::connect();
    
    // Drop-in replacement for ElegantOTA.begin(&server)
    ElegantOTACompat::begin(&server);
}

void loop() {
    NetworkManager::handle();
    server.handleClient();
    ElegantOTACompat::loop();  // Replaces ElegantOTA.loop()
}
```

### 3. Individual Module Usage

```cpp
#include <OTACore.h>
#include <NetworkManager.h>
#include <OTAWebServer.h>

void setup() {
    // Initialize components individually
    OTACore::begin(true);  // Enable persistence
    NetworkManager::begin("WiFi", "Password", true);
    
    OTAWebServer::Config serverConfig;
    serverConfig.port = 8080;
    serverConfig.path = "/firmware";
    OTAWebServer::begin(serverConfig);
}

void loop() {
    OTACore::handle();
    NetworkManager::handle();
    OTAWebServer::handle();
}
```

### 4. AsyncWebServer Integration

```cpp
#include <ESPAsyncWebServer.h>
#include <ModularOTA.h>

AsyncWebServer server(80);
ModularOTA::Config otaConfig = {
    .serverPort = 3232,  // Separate port for OTA
};

void setup() {
    ModularOTA::begin(otaConfig);
    
    server.on("/", [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "Main App");
    });
    
    server.begin();
}

void loop() {
    ModularOTA::handle();
}
```

## API Reference

### ModularOTA Class

#### Configuration Structure
```cpp
struct Config {
    String ssid;                    // WiFi SSID
    String password;                // WiFi password
    bool autoReconnect = true;      // Enable auto-reconnection
    bool enablePersistence = true;  // Enable OTA persistence
    int serverPort = 3232;          // OTA server port
    String otaPath = "/update";     // OTA endpoint path
    String authUsername = "";       // HTTP auth username
    String authPassword = "";       // HTTP auth password
    size_t maxUploadSize = 1MB;     // Maximum upload size
};
```

#### Main Methods
```cpp
bool begin(const Config& config);           // Initialize system
void handle();                              // Handle all components
void stop();                                // Stop OTA system
bool isReady();                             // Check if system ready
String getOTAUrl();                         // Get OTA URL
String getSystemInfoJSON();                 // Get system status
```

### OTACore Class

#### Status Enumeration
```cpp
enum class Status {
    IDLE,       // Ready for update
    RECEIVING,  // Update in progress
    COMPLETE,   // Update completed
    ERROR,      // Update failed
    REBOOTING   // System rebooting
};
```

#### Core Methods
```cpp
bool begin(bool enablePersistence = true);              // Initialize
bool startUpdate(size_t size, const String& md5 = "");  // Start update
int writeData(uint8_t* data, size_t len);               // Write data
bool finishUpdate();                                     // Complete update
void abortUpdate();                                      // Abort update
Status getStatus();                                      // Get status
int getProgress();                                       // Get progress %
bool isActive();                                         // Check if active
```

### NetworkManager Class

#### Network Status
```cpp
enum class Status {
    DISCONNECTED, CONNECTING, CONNECTED, FAILED, RECONNECTING
};
```

#### Network Methods
```cpp
bool begin(const char* ssid, const char* password, bool autoReconnect = true);
bool connect(unsigned long timeout = 10000);
void disconnect();
bool isConnected();
String getIPAddress();
int getRSSI();
void setAutoReconnect(bool enable);
```

### OTAWebServer Class

#### Server Configuration
```cpp
struct Config {
    int port = 3232;
    String path = "/update";
    String username = "";
    String password = "";
    bool enableCORS = true;
    bool enableProgress = true;
    size_t maxUploadSize = 1048576;
};
```

#### Server Methods
```cpp
bool begin(const Config& config = Config());
void stop();
void handle();
bool isRunning();
String getOTAUrl();
void addCustomEndpoint(const String& path, std::function<void()> handler);
```

## Memory Management

### Memory Usage Guidelines

1. **Heap Memory**: Reserve at least 100KB free heap during OTA
2. **RTC Memory**: Uses ~32 bytes for persistence data
3. **Flash Memory**: Requires OTA partition equal to application size
4. **Upload Buffer**: Configurable, typically 1KB-4KB chunks

### Memory Monitoring

```cpp
// Get current memory status
size_t freeHeap, totalHeap, minFreeHeap;
ModularOTA::getMemoryInfo(freeHeap, totalHeap, minFreeHeap);

// Memory status in JSON
String systemInfo = ModularOTA::getSystemInfoJSON();
```

### Memory Optimization Tips

1. **Free unused resources** before starting OTA
2. **Avoid large allocations** during update
3. **Monitor minimum free heap** to prevent crashes
4. **Use streaming** for large data processing

## Security Considerations

### Authentication

```cpp
// Enable HTTP Basic Authentication
ModularOTA::Config config;
config.authUsername = "admin";
config.authPassword = "secure123";
```

### Network Security

1. **Use strong WiFi passwords**
2. **Consider VPN** for remote updates
3. **Implement rate limiting** if needed
4. **Monitor failed attempts**

### Firmware Validation

```cpp
// Optional MD5 validation
OTACore::startUpdate(firmwareSize, "md5hash");
```

## Troubleshooting

### Common Issues

1. **OTA fails with "insufficient space"**
   - Check partition table configuration
   - Verify OTA partition size matches application size

2. **WiFi disconnections during update**
   - Enable auto-reconnection
   - Increase reconnection interval
   - Check signal strength (RSSI)

3. **Memory errors during upload**
   - Free heap before starting OTA
   - Reduce upload chunk size
   - Monitor memory usage

4. **Persistence not working**
   - Verify RTC memory support on your board
   - Check power supply stability
   - Validate magic number and CRC

### Debug Output

Enable verbose logging:
```cpp
// All components provide detailed serial output
Serial.begin(115200);
// Check serial monitor for detailed status information
```

### Memory Debugging

```cpp
void printMemoryStatus() {
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Min Free Heap: %d bytes\n", ESP.getMinFreeHeap());
    Serial.printf("Largest Free Block: %d bytes\n", ESP.getMaxAllocHeap());
}
```

## Migration Guide

### From ElegantOTA

1. **Replace include**:
   ```cpp
   // Old
   #include <ElegantOTA.h>
   
   // New
   #include <ElegantOTACompat.h>
   #include <NetworkManager.h>
   ```

2. **Initialize network separately**:
   ```cpp
   // Add before ElegantOTA
   NetworkManager::begin(ssid, password);
   NetworkManager::connect();
   ```

3. **Replace method calls**:
   ```cpp
   // Old
   ElegantOTA.begin(&server);
   ElegantOTA.loop();
   
   // New
   ElegantOTACompat::begin(&server);
   ElegantOTACompat::loop();
   ```

### To Full Modular System

1. **Replace ElegantOTACompat** with ModularOTA
2. **Configure all components** in single config structure
3. **Remove manual WiFi handling** (handled by NetworkManager)
4. **Update callback functions** to new event system

## Performance Characteristics

### Upload Performance

- **Typical upload speed**: 50-200 KB/s (depends on WiFi signal)
- **Memory overhead**: ~10-20KB during upload
- **CPU usage**: Minimal impact on application performance

### Persistence Overhead

- **RTC memory**: 32 bytes storage
- **Boot time**: +10-50ms additional initialization
- **Flash writes**: Minimal (only on state changes)

### Network Performance

- **Connection time**: 2-10 seconds (depending on network)
- **Reconnection**: 5-30 seconds (configurable interval)
- **Keep-alive**: Automatic monitoring and recovery

## Future Enhancements

### Planned Features

1. **Rollback support** - Automatic rollback on failed boot
2. **Delta updates** - Incremental firmware updates
3. **Multiple firmware slots** - A/B partition scheme
4. **Remote configuration** - Over-the-air configuration updates
5. **Update scheduling** - Scheduled automatic updates
6. **Cryptographic signatures** - Signed firmware validation

### Contributing

This modular architecture is designed to be extensible. Key extension points:

1. **Custom OTA sources** (HTTP, MQTT, Bluetooth)
2. **Additional persistence mechanisms** (EEPROM, external flash)
3. **Enhanced security features** (TLS, certificates)
4. **Custom web interfaces** (React, Vue.js frontends)

## License and Credits

Based on the ElegantOTA library but completely rewritten for modularity and persistence. Maintains backward compatibility while providing modern, robust OTA capabilities for ESP32 projects.
