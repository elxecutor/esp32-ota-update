# ESP32 Modular OTA System

A refactored, modular Over-The-Air (OTA) update system for ESP32 that decouples OTA logic from web servers and main application code. This system ensures OTA functionality persists across firmware updates, enabling reliable and repeatable OTA updates in custom ESP32 setups.

## 🚀 Key Features

- **🔄 Persistent OTA Logic**: OTA functionality survives firmware updates using RTC memory
- **🔌 Decoupled Architecture**: Complete separation of OTA, network, and server components  
- **📶 Auto-Reconnection**: Automatic WiFi reconnection with configurable intervals
- **🔙 Backward Compatibility**: Drop-in replacement for ElegantOTA library
- **💾 Memory Optimized**: Intelligent memory management for reliable updates
- **🔐 Security**: Optional HTTP authentication and firmware validation
- **📊 Progress Monitoring**: Real-time upload progress and status tracking
- **⚙️ Modular Design**: Use individual components or complete system

## 📋 Requirements

- ESP32 board (ESP32, ESP32-S2, ESP32-S3, ESP32-C3)
- [PlatformIO](https://platformio.org/) or Arduino IDE
- WiFi network access

## 🏗️ Architecture

### Core Components

1. **OTACore** - Persistent OTA operations that survive reboots
2. **NetworkManager** - WiFi connectivity and auto-reconnection
3. **OTAWebServer** - Dedicated web interface for OTA operations  
4. **ElegantOTACompat** - Backward compatibility layer
5. **ModularOTA** - Main orchestrator coordinating all components

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Your App      │    │  NetworkManager │    │   OTACore       │
│                 │    │                 │    │  (Persistent)   │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                    ┌─────────────────┐
                    │  ModularOTA     │
                    │  (Orchestrator) │
                    └─────────────────┘
                                 │
                    ┌─────────────────┐
                    │  OTAWebServer   │
                    │  (Port 3232)    │
                    └─────────────────┘
```

## 🚀 Quick Start

### 1. Complete Modular Setup (Recommended)

```cpp
#include <ModularOTA.h>

// Configure the complete OTA system
ModularOTA::Config config = {
    .ssid = "YourWiFiSSID",
    .password = "YourWiFiPassword", 
    .enablePersistence = true,
    .serverPort = 3232,
    .otaPath = "/update"
};

void setup() {
    Serial.begin(115200);
    
    // Initialize complete modular OTA system
    if (ModularOTA::begin(config)) {
        Serial.println("✅ Modular OTA system ready!");
        Serial.println("🔗 OTA URL: " + ModularOTA::getOTAUrl());
    }
}

void loop() {
    ModularOTA::handle();  // Handle all components
    // Your application code here
}
```

### 2. Backward Compatibility (Drop-in Replacement)

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <ElegantOTACompat.h>  // Replace ElegantOTA include
#include <NetworkManager.h>

const char* ssid = "YourWiFi";
const char* password = "YourPassword";
WebServer server(80);

void setup() {
    Serial.begin(115200);
    
    // Initialize network with auto-reconnection
    NetworkManager::begin(ssid, password, true);
    NetworkManager::connect();
    
    // Your existing web server routes
    server.on("/", []() {
        server.send(200, "text/plain", "Hello World!");
    });
    
    // Drop-in replacement for ElegantOTA.begin(&server)
    ElegantOTACompat::begin(&server);
    
    server.begin();
}

void loop() {
    NetworkManager::handle();        // Auto-reconnection
    server.handleClient();
    ElegantOTACompat::loop();       // Replaces ElegantOTA.loop()
}
```

## 📦 Installation

### PlatformIO

1. **Clone this repository:**
   ```bash
   git clone https://github.com/elxecutor/ota-update
   cd ota-update
   ```

2. **Update WiFi credentials in `src/main.cpp`:**
   ```cpp
   const char *ssid = "YourWiFiSSID";
   const char *password = "YourWiFiPassword";
   ```

3. **Build and upload:**
   ```bash
   # Build and upload main example (backward compatible)
   pio run --target upload
   
   # Or build specific examples:
   pio run -e modular_complete --target upload
   pio run -e individual_modules --target upload
   pio run -e async_webserver --target upload
   ```

4. **Monitor output:**
   ```bash
   pio device monitor
   ```

### Arduino IDE

1. Copy the library folders from `lib/` to your Arduino `libraries` folder
2. Install required dependencies through Library Manager
3. Open any example from `examples/` folder
4. Update WiFi credentials and upload

## 🎯 Examples

### Available Examples

| Example | Description | Use Case |
|---------|-------------|----------|
| **ModularComplete** | Complete modular system | New projects, full control |
| **BackwardCompatibility** | Drop-in ElegantOTA replacement | Existing projects migration |
| **IndividualModules** | Use components separately | Custom implementations |
| **AsyncWebServer** | Integration with AsyncWebServer | High-performance web apps |

### Running Examples

```bash
# Complete modular system
pio run -e modular_complete --target upload

# Backward compatibility  
pio run -e backward_compatible --target upload

# Individual modules
pio run -e individual_modules --target upload

# AsyncWebServer integration
pio run -e async_webserver --target upload
```

## 🔧 Configuration

### Complete System Configuration

```cpp
ModularOTA::Config config = {
    // Network settings
    .ssid = "YourWiFiSSID",
    .password = "YourWiFiPassword", 
    .autoReconnect = true,
    .reconnectInterval = 30000,      // 30 seconds
    
    // OTA Core settings
    .enablePersistence = true,       // Survive firmware updates
    
    // Web Server settings
    .serverPort = 3232,              // OTA server port
    .otaPath = "/update",            // OTA endpoint path
    .authUsername = "",              // Optional HTTP auth
    .authPassword = "",
    .enableCORS = true,
    .enableProgress = true,
    .maxUploadSize = 1048576         // 1MB max upload
};
```

### Individual Component Configuration

```cpp
// Network Manager
NetworkManager::begin("SSID", "Password", true);  // auto-reconnect enabled
NetworkManager::setReconnectInterval(30000);      // 30 second intervals

// OTA Core  
OTACore::begin(true);                              // enable persistence
OTACore::setPersistence(true);                     // can be changed later

// OTA Web Server
OTAWebServer::Config serverConfig;
serverConfig.port = 8080;
serverConfig.path = "/firmware-update";
serverConfig.username = "admin";
serverConfig.password = "password123";
OTAWebServer::begin(serverConfig);
```

## 📊 System Status and Monitoring

### Get System Information

```cpp
// Check if system is ready
bool ready = ModularOTA::isReady();

// Get detailed system status
String systemInfo = ModularOTA::getSystemInfoJSON();

// Monitor memory usage
size_t freeHeap, totalHeap, minFreeHeap;
ModularOTA::getMemoryInfo(freeHeap, totalHeap, minFreeHeap);

// Network status
bool connected = NetworkManager::isConnected();
String ip = NetworkManager::getIPAddress();
int rssi = NetworkManager::getRSSI();

// OTA status
OTACore::Status otaStatus = OTACore::getStatus();
int progress = OTACore::getProgress();
bool active = OTACore::isActive();
```

### Event Callbacks

```cpp
// System-wide events
ModularOTA::setCallback([](ModularOTA::Event event, const String& message, int value) {
    switch (event) {
        case ModularOTA::Event::NETWORK_CONNECTED:
            Serial.println("Network connected: " + message);
            break;
        case ModularOTA::Event::OTA_STARTED:
            Serial.println("OTA started: " + message);
            break;
        case ModularOTA::Event::OTA_PROGRESS:
            Serial.printf("OTA progress: %d%%\n", value);
            break;
        // ... handle other events
    }
});

// Individual component callbacks
NetworkManager::setCallback([](NetworkManager::Status status, const String& message) {
    // Handle network events
});

OTACore::setCallback([](OTACore::Status status, int progress, const String& message) {
    // Handle OTA events  
});
```

## 🛡️ Security Features

### HTTP Authentication

```cpp
// Enable authentication for OTA interface
ModularOTA::Config config;
config.authUsername = "admin";
config.authPassword = "securePassword123";

// Or set authentication later
OTAWebServer::setAuthentication("admin", "securePassword123");
```

### Firmware Validation

```cpp
// Optional MD5 validation during upload
String expectedMD5 = "d41d8cd98f00b204e9800998ecf8427e";
OTACore::startUpdate(firmwareSize, expectedMD5);
```

## 🔧 Memory Management

### Memory Layout for OTA Persistence

```
ESP32 Memory Layout:
┌─────────────────────────────────────┐
│ Application Code (Flash)            │  ← Your main firmware
├─────────────────────────────────────┤
│ OTA Partition (Flash)               │  ← Update destination
├─────────────────────────────────────┤  
│ SPIFFS/LittleFS (Flash)            │  ← File system
├─────────────────────────────────────┤
│ RAM (Heap)                          │  ← Runtime memory
├─────────────────────────────────────┤
│ RTC Memory (Persistent)             │  ← OTA state storage
└─────────────────────────────────────┘
```

### Memory Optimization

```cpp
// Monitor memory during OTA
void checkMemory() {
    size_t freeHeap = ESP.getFreeHeap();
    size_t minFreeHeap = ESP.getMinFreeHeap();
    
    if (freeHeap < 50000) {  // Less than 50KB
        Serial.println("⚠️ Low memory warning!");
        // Free some resources or abort OTA
    }
}

// Free resources before OTA
void prepareForOTA() {
    // Close unnecessary connections
    // Free large buffers
    // Minimize heap fragmentation
}
```

## 🚀 Advanced Usage

### Custom API Endpoints

```cpp
// Add custom endpoints to OTA server
ModularOTA::addCustomEndpoint("/api/restart", []() {
    // Handle restart request
    ESP.restart();
});

ModularOTA::addCustomEndpoint("/api/info", []() {
    // Return custom JSON response
    String json = "{\"version\":\"1.0.0\",\"build\":\"" + String(__DATE__) + "\"}";
    // Response handled automatically
});
```

### Integration with AsyncWebServer

```cpp
#include <ESPAsyncWebServer.h>
#include <ModularOTA.h>

AsyncWebServer server(80);        // Main application server
ModularOTA::Config config = {
    .serverPort = 3232            // Separate OTA server port
};

void setup() {
    // Initialize modular OTA on separate port
    ModularOTA::begin(config);
    
    // Setup main application server
    server.on("/", [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Main App");
    });
    
    server.begin();
}

void loop() {
    ModularOTA::handle();  // Handles OTA on port 3232
    // AsyncWebServer handles requests automatically
}
```

### WebSocket Integration

```cpp
// Real-time OTA progress via WebSocket
AsyncWebSocket ws("/ws");

ModularOTA::setCallback([&ws](ModularOTA::Event event, const String& message, int value) {
    if (event == ModularOTA::Event::OTA_PROGRESS) {
        String wsMessage = "{\"type\":\"ota_progress\",\"progress\":" + String(value) + "}";
        ws.textAll(wsMessage);
    }
});

server.addHandler(&ws);
```

## 🔄 Migration from ElegantOTA

### Step-by-Step Migration

1. **Replace includes:**
   ```cpp
   // Old
   #include <ElegantOTA.h>
   
   // New  
   #include <ElegantOTACompat.h>
   #include <NetworkManager.h>
   ```

2. **Add network initialization:**
   ```cpp
   // Add before ElegantOTA setup
   NetworkManager::begin(ssid, password, true);
   NetworkManager::connect();
   ```

3. **Update method calls:**
   ```cpp
   // Old
   ElegantOTA.begin(&server);
   ElegantOTA.loop();
   
   // New (same API, modular implementation)
   ElegantOTACompat::begin(&server);  
   ElegantOTACompat::loop();
   ```

4. **Optional: Migrate to full modular system:**
   ```cpp
   // Replace everything with:
   #include <ModularOTA.h>
   
   ModularOTA::Config config = { /* ... */ };
   ModularOTA::begin(config);
   
   // In loop:
   ModularOTA::handle();
   ```

## 📚 API Reference

### ModularOTA Class

| Method | Description | Returns |
|--------|-------------|---------|
| `begin(config)` | Initialize complete system | `bool` |
| `handle()` | Process all components | `void` |
| `stop()` | Stop OTA system | `void` |
| `isReady()` | Check if system ready | `bool` |
| `getOTAUrl()` | Get OTA interface URL | `String` |
| `getSystemInfoJSON()` | Get system status as JSON | `String` |
| `addCustomEndpoint(path, handler)` | Add custom API endpoint | `bool` |
| `getMemoryInfo(free, total, min)` | Get memory usage info | `bool` |

### NetworkManager Class  

| Method | Description | Returns |
|--------|-------------|---------|
| `begin(ssid, password, autoReconnect)` | Initialize network manager | `bool` |
| `connect(timeout)` | Connect to WiFi | `bool` |
| `disconnect()` | Disconnect from WiFi | `void` |
| `isConnected()` | Check connection status | `bool` |
| `getIPAddress()` | Get local IP address | `String` |
| `getRSSI()` | Get signal strength | `int` |
| `setAutoReconnect(enable)` | Enable/disable auto-reconnect | `void` |

### OTACore Class

| Method | Description | Returns |
|--------|-------------|---------|  
| `begin(enablePersistence)` | Initialize OTA core | `bool` |
| `startUpdate(size, md5)` | Start OTA update | `bool` |
| `writeData(data, len)` | Write update data | `int` |
| `finishUpdate()` | Complete update | `bool` |
| `abortUpdate()` | Abort current update | `void` |
| `getStatus()` | Get current status | `Status` |
| `getProgress()` | Get progress percentage | `int` |
| `isActive()` | Check if update active | `bool` |

## 🐛 Troubleshooting

### Common Issues

| Issue | Symptoms | Solution |
|-------|----------|----------|
| **WiFi disconnections** | Frequent connection drops | Enable auto-reconnection, check signal strength |
| **OTA upload fails** | "Insufficient space" error | Check partition table, verify OTA partition size |
| **Memory errors** | Crashes during upload | Monitor heap usage, free resources before OTA |
| **Persistence not working** | State lost after reboot | Verify RTC memory support, check power supply |
| **Authentication fails** | 401 Unauthorized | Check username/password, verify credentials |

### Debug Output

Enable detailed logging:
```cpp
// Add to setup()
Serial.begin(115200);

// All components provide detailed serial output
// Check serial monitor for status information
```

### Memory Debugging

```cpp
void printMemoryInfo() {
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Min Free Heap: %d bytes\n", ESP.getMinFreeHeap());
    Serial.printf("Max Alloc Heap: %d bytes\n", ESP.getMaxAllocHeap());
    Serial.printf("Heap Size: %d bytes\n", ESP.getHeapSize());
}
```

### Network Debugging

```cpp
NetworkManager::setCallback([](NetworkManager::Status status, const String& message) {
    String statusStr;
    switch (status) {
        case NetworkManager::Status::CONNECTED: statusStr = "CONNECTED"; break;
        case NetworkManager::Status::DISCONNECTED: statusStr = "DISCONNECTED"; break;
        case NetworkManager::Status::RECONNECTING: statusStr = "RECONNECTING"; break;
        default: statusStr = "OTHER"; break;
    }
    Serial.println("[Network] " + statusStr + ": " + message);
});
```

## 📖 Documentation

- **[Complete API Documentation](docs/MODULAR_OTA_GUIDE.md)** - Detailed API reference and usage guide
- **[Architecture Overview](docs/MODULAR_OTA_GUIDE.md#architecture)** - System design and component interaction
- **[Memory Management](docs/MODULAR_OTA_GUIDE.md#memory-management)** - Memory layout and optimization
- **[Security Guide](docs/MODULAR_OTA_GUIDE.md#security-considerations)** - Security best practices
- **[Migration Guide](docs/MODULAR_OTA_GUIDE.md#migration-guide)** - Step-by-step migration from ElegantOTA

## 🤝 Contributing

Contributions are welcome! This modular architecture is designed to be extensible:

1. **Fork the repository**
2. **Create a feature branch**: `git checkout -b feature/amazing-feature`
3. **Commit changes**: `git commit -m 'Add amazing feature'`
4. **Push to branch**: `git push origin feature/amazing-feature`  
5. **Open a Pull Request**

### Extension Points

- **Custom OTA sources** (MQTT, Bluetooth, etc.)
- **Additional persistence mechanisms** (EEPROM, external flash)
- **Enhanced security features** (TLS, certificates)
- **Custom web interfaces** (React, Vue.js frontends)

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Based on the [ElegantOTA](https://github.com/ayushsharma82/ElegantOTA) library
- Completely rewritten for modularity and persistence
- Maintains backward compatibility while providing modern OTA capabilities

## 📞 Support

- **GitHub Issues**: [Report bugs or request features](https://github.com/elxecutor/ota-update/issues)
- **Documentation**: [Complete guide and examples](docs/MODULAR_OTA_GUIDE.md)
- **Examples**: Check the `examples/` directory for various use cases

---

**⚡ Enables reliable, repeatable OTA updates with persistent logic and automatic recovery!**
    
    // Your existing web server routes
    server.on("/", []() {
        server.send(200, "text/plain", "Hello World!");
    });
    
    // Drop-in replacement for ElegantOTA.begin(&server)
    ElegantOTACompat::begin(&server);
    
    server.begin();
}

void loop() {
    NetworkManager::handle();        // Auto-reconnection
    server.handleClient();
    ElegantOTACompat::loop();       // Replaces ElegantOTA.loop()
}
```

   ```bash
   pio device monitor --monitor_speed 115200
   ```

5. **OTA Update:**

   Once connected, the ElegantOTA interface will be available via the web server running on the ESP32’s IP address. Open a browser, enter the IP address printed on the serial monitor, and follow the prompts to perform OTA updates.

## OTA Safety and Fallback

One of the key benefits of using OTA updates with ElegantOTA is safety:
- **Application-Only Updates:**  
  ElegantOTA updates only the application firmware, leaving the bootloader and critical system partitions untouched. This design ensures that even if a new firmware image is faulty, the core system remains intact.
  
- **Preventing Bricking:**  
  Under normal operation, this process prevents your ESP32 from becoming unusable ("bricked"). However, to further protect your board:
  - **Firmware Validation:**  
    Always implement integrity checks (such as SHA256 hashes or digital signatures) to validate the new firmware before committing to it.
  
By following these best practices, the OTA update process is designed to be safe and largely automated, reducing the risk of rendering your board useless.

## PlatformIO Configuration

Below is an example `platformio.ini` configuration for the ESP32 Feather board:

```ini
[env:featheresp32]
platform = espressif32
board = featheresp32
framework = arduino
lib_deps = 
    ayushsharma82/ElegantOTA@^3.1.7
lib_compat_mode = strict
```

> **Reminder:** This configuration relies on the ESP32 board package installed on your development machine (via PlatformIO). This package provides all the necessary files to compile and flash firmware for your ESP32 and is not something you install directly on your board.

## Troubleshooting

- **Wi‑Fi Connection Issues:**  
  Verify that the SSID and password are correct. If you're using an open network, try using `WiFi.begin(ssid);` instead of providing an empty password. Also, consider removing any forced channel parameters from `WiFi.begin()`.

- **Serial Monitor Issues:**  
  If you're running the project in a simulation environment like Wokwi, use the built‑in serial console provided by the simulator instead of PlatformIO's external monitor.

- **OTA Not Initiating:**  
  Confirm that the web server is running (check for the "HTTP server started" message in the serial output). Then access the ElegantOTA interface by entering the ESP32's IP address in your browser.

## License

This project is licensed under the [MIT License](LICENSE).