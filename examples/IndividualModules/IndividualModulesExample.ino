/**
 * @file IndividualModulesExample.ino
 * @brief Example using individual modules separately
 * 
 * This example demonstrates how to use the individual modular components
 * separately for custom implementations.
 */

#include <OTACore.h>
#include <NetworkManager.h>
#include <OTAWebServer.h>

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Individual Modules Example ===");
    
    // Initialize OTA Core with persistence
    Serial.println("Initializing OTA Core...");
    if (OTACore::begin(true)) {
        Serial.println("✅ OTA Core initialized with persistence enabled");
    } else {
        Serial.println("❌ Failed to initialize OTA Core");
        return;
    }
    
    // Set OTA Core callback
    OTACore::setCallback([](OTACore::Status status, int progress, const String& message) {
        String statusStr;
        switch (status) {
            case OTACore::Status::IDLE: statusStr = "IDLE"; break;
            case OTACore::Status::RECEIVING: statusStr = "RECEIVING"; break;
            case OTACore::Status::COMPLETE: statusStr = "COMPLETE"; break;
            case OTACore::Status::ERROR: statusStr = "ERROR"; break;
            case OTACore::Status::REBOOTING: statusStr = "REBOOTING"; break;
        }
        Serial.println("🔧 OTA Core [" + statusStr + "] " + String(progress) + "% - " + message);
    });
    
    // Initialize Network Manager
    Serial.println("Initializing Network Manager...");
    if (NetworkManager::begin("YourWiFiSSID", "YourWiFiPassword", true)) {
        Serial.println("✅ Network Manager initialized");
    } else {
        Serial.println("❌ Failed to initialize Network Manager");
        return;
    }
    
    // Set Network Manager callback
    NetworkManager::setCallback([](NetworkManager::Status status, const String& message) {
        String statusStr;
        switch (status) {
            case NetworkManager::Status::DISCONNECTED: statusStr = "DISCONNECTED"; break;
            case NetworkManager::Status::CONNECTING: statusStr = "CONNECTING"; break;
            case NetworkManager::Status::CONNECTED: statusStr = "CONNECTED"; break;
            case NetworkManager::Status::FAILED: statusStr = "FAILED"; break;
            case NetworkManager::Status::RECONNECTING: statusStr = "RECONNECTING"; break;
        }
        Serial.println("🌐 Network [" + statusStr + "] " + message);
    });
    
    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    if (NetworkManager::connect(15000)) { // 15 second timeout
        Serial.println("✅ Connected to WiFi!");
        Serial.println("📡 IP Address: " + NetworkManager::getIPAddress());
        Serial.println("📶 Signal Strength: " + String(NetworkManager::getRSSI()) + " dBm");
    } else {
        Serial.println("⚠️  Initial WiFi connection failed, will auto-retry...");
    }
    
    // Configure and initialize OTA Web Server
    Serial.println("Initializing OTA Web Server...");
    OTAWebServer::Config serverConfig;
    serverConfig.port = 8080;                    // Custom port
    serverConfig.path = "/firmware-update";      // Custom path
    serverConfig.username = "admin";             // Optional authentication
    serverConfig.password = "password123";
    serverConfig.enableCORS = true;
    serverConfig.enableProgress = true;
    serverConfig.maxUploadSize = 2 * 1024 * 1024; // 2MB max upload
    
    if (OTAWebServer::begin(serverConfig)) {
        Serial.println("✅ OTA Web Server initialized on port " + String(serverConfig.port));
    } else {
        Serial.println("❌ Failed to initialize OTA Web Server");
        return;
    }
    
    // Set OTA Web Server callback
    OTAWebServer::setCallback([](OTAWebServer::Event event, const String& message, int value) {
        String eventStr;
        switch (event) {
            case OTAWebServer::Event::STARTED: eventStr = "STARTED"; break;
            case OTAWebServer::Event::STOPPED: eventStr = "STOPPED"; break;
            case OTAWebServer::Event::UPLOAD_START: eventStr = "UPLOAD_START"; break;
            case OTAWebServer::Event::UPLOAD_PROGRESS: eventStr = "UPLOAD_PROGRESS"; break;
            case OTAWebServer::Event::UPLOAD_COMPLETE: eventStr = "UPLOAD_COMPLETE"; break;
            case OTAWebServer::Event::UPLOAD_ERROR: eventStr = "UPLOAD_ERROR"; break;
            case OTAWebServer::Event::CLIENT_CONNECTED: eventStr = "CLIENT_CONNECTED"; break;
            case OTAWebServer::Event::CLIENT_DISCONNECTED: eventStr = "CLIENT_DISCONNECTED"; break;
        }
        Serial.println("🌐 Server [" + eventStr + "] " + message + 
                      (value != 0 ? " (" + String(value) + ")" : ""));
    });
    
    // Add custom endpoints to the OTA server
    OTAWebServer::addCustomEndpoint("/api/system", []() {
        // Custom API endpoint for system information
        String json = "{";
        json += "\"uptime\":" + String(millis()) + ",";
        json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
        json += "\"chipModel\":\"" + String(ESP.getChipModel()) + "\",";
        json += "\"wifiConnected\":" + String(NetworkManager::isConnected() ? "true" : "false") + ",";
        json += "\"otaStatus\":" + String((int)OTACore::getStatus()) + ",";
        json += "\"otaProgress\":" + String(OTACore::getProgress());
        json += "}";
        
        // This would be handled by the web server - response is sent automatically
        Serial.println("📊 System info requested via API");
    });
    
    Serial.println("🎉 All modules initialized successfully!");
    Serial.println("📝 Available features:");
    Serial.println("   - Persistent OTA core (survives firmware updates)");
    Serial.println("   - Auto-reconnecting WiFi");
    Serial.println("   - Custom OTA web interface with authentication");
    Serial.println("   - Custom API endpoints");
    Serial.println("   - Progress monitoring");
    
    // Print final status
    printSystemStatus();
}

void loop() {
    // Handle each module individually
    NetworkManager::handle();      // WiFi auto-reconnection
    OTACore::handle();            // OTA operations
    OTAWebServer::handle();       // Web server requests
    
    // Custom application logic
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 30000) { // Every 30 seconds
        lastStatus = millis();
        printSystemStatus();
    }
    
    // Monitor for manual OTA operations (example)
    static unsigned long lastOTACheck = 0;
    if (millis() - lastOTACheck > 5000) { // Every 5 seconds
        lastOTACheck = millis();
        
        if (OTACore::isActive()) {
            Serial.println("🔄 OTA in progress: " + String(OTACore::getProgress()) + "%");
        }
    }
    
    delay(100);
}

void printSystemStatus() {
    Serial.println("\n📊 === System Status ===");
    Serial.println("⏰ Uptime: " + String(millis() / 1000) + " seconds");
    Serial.println("💾 Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
    
    // Network status
    if (NetworkManager::isConnected()) {
        Serial.println("🌐 Network: Connected to " + NetworkManager::getSSID());
        Serial.println("📡 IP: " + NetworkManager::getIPAddress() + 
                      " (RSSI: " + String(NetworkManager::getRSSI()) + " dBm)");
        Serial.println("🔗 OTA URL: " + OTAWebServer::getOTAUrl());
    } else {
        Serial.println("🌐 Network: Disconnected (auto-reconnect enabled)");
    }
    
    // OTA status
    String otaStatus;
    switch (OTACore::getStatus()) {
        case OTACore::Status::IDLE: otaStatus = "Ready"; break;
        case OTACore::Status::RECEIVING: otaStatus = "Updating (" + String(OTACore::getProgress()) + "%)"; break;
        case OTACore::Status::COMPLETE: otaStatus = "Complete"; break;
        case OTACore::Status::ERROR: otaStatus = "Error: " + OTACore::getLastError(); break;
        case OTACore::Status::REBOOTING: otaStatus = "Rebooting"; break;
    }
    Serial.println("🔧 OTA: " + otaStatus + " (Persistent: " + 
                  String(OTACore::isPersistent() ? "Yes" : "No") + ")");
    
    // Server status
    Serial.println("🌐 Server: " + String(OTAWebServer::isRunning() ? "Running" : "Stopped") + 
                  " (Clients: " + String(OTAWebServer::getClientCount()) + ")");
    
    Serial.println("========================\n");
}
