/**
 * @file BackwardCompatibilityExample.ino
 * @brief Backward compatibility example using ElegantOTACompat
 * 
 * This example shows how to use the new modular architecture while
 * maintaining compatibility with existing ElegantOTA code.
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ElegantOTACompat.h>
#include <NetworkManager.h>

const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

WebServer server(80);

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Backward Compatibility Example ===");
    
    // Initialize network manager (new modular approach)
    NetworkManager::begin(ssid, password, true);
    
    // Set network callback
    NetworkManager::setCallback([](NetworkManager::Status status, const String& message) {
        if (status == NetworkManager::Status::CONNECTED) {
            Serial.println("✓ Connected to WiFi: " + message);
            Serial.println("✓ OTA URL: " + ElegantOTACompat::getOTAUrl());
        } else if (status == NetworkManager::Status::DISCONNECTED) {
            Serial.println("✗ Disconnected from WiFi: " + message);
        }
    });
    
    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    if (NetworkManager::connect()) {
        Serial.println("✅ WiFi connected!");
        Serial.print("IP address: ");
        Serial.println(NetworkManager::getIPAddress());
    } else {
        Serial.println("❌ WiFi connection failed!");
    }
    
    // Setup web server routes (your existing application code)
    server.on("/", []() {
        server.send(200, "text/plain", "Hello! This is a backward compatibility demo.");
    });
    
    server.on("/info", []() {
        String info = "ESP32 System Information\n";
        info += "========================\n";
        info += "Chip Model: " + String(ESP.getChipModel()) + "\n";
        info += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
        info += "WiFi SSID: " + NetworkManager::getSSID() + "\n";
        info += "IP Address: " + NetworkManager::getIPAddress() + "\n";
        info += "RSSI: " + String(NetworkManager::getRSSI()) + " dBm\n";
        info += "OTA Status: " + String(ElegantOTACompat::isUpdating() ? "Updating" : "Ready") + "\n";
        info += "OTA Progress: " + String(ElegantOTACompat::getProgress()) + "%\n";
        server.send(200, "text/plain", info);
    });
    
    // Initialize ElegantOTA compatibility layer with existing server
    // This is a drop-in replacement for ElegantOTA.begin(&server)
    if (ElegantOTACompat::begin(&server, "/update")) {
        Serial.println("✅ ElegantOTA compatibility layer initialized!");
    } else {
        Serial.println("❌ Failed to initialize ElegantOTA compatibility layer!");
    }
    
    // Set OTA callbacks (same API as original ElegantOTA)
    ElegantOTACompat::onStart([]() {
        Serial.println("🔄 OTA update started!");
    });
    
    ElegantOTACompat::onEnd([]() {
        Serial.println("✅ OTA update completed!");
    });
    
    ElegantOTACompat::onProgress([](unsigned int progress, unsigned int total) {
        int percent = (progress * 100) / total;
        static int lastPercent = -1;
        if (percent != lastPercent) {
            Serial.printf("📊 OTA Progress: %d%% (%u/%u bytes)\n", percent, progress, total);
            lastPercent = percent;
        }
    });
    
    ElegantOTACompat::onError([](String error) {
        Serial.println("❌ OTA Error: " + error);
    });
    
    // Start the web server
    server.begin();
    Serial.println("🌐 HTTP server started on port 80");
    Serial.println("📝 Available endpoints:");
    Serial.println("   http://" + NetworkManager::getIPAddress() + "/");
    Serial.println("   http://" + NetworkManager::getIPAddress() + "/info");
    Serial.println("   http://" + NetworkManager::getIPAddress() + "/update (OTA)");
    
    Serial.println("Setup completed. System ready for OTA updates!");
}

void loop() {
    // Handle network management (automatic reconnection, etc.)
    NetworkManager::handle();
    
    // Handle web server requests
    server.handleClient();
    
    // Handle ElegantOTA (this replaces ElegantOTA.loop())
    ElegantOTACompat::loop();
    
    // Your existing application code goes here
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 60000) { // Print status every minute
        lastStatus = millis();
        
        if (NetworkManager::isConnected()) {
            Serial.println("📊 Status: WiFi connected, OTA ready");
            Serial.println("💾 Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
            if (ElegantOTACompat::isUpdating()) {
                Serial.println("🔄 OTA in progress: " + String(ElegantOTACompat::getProgress()) + "%");
            }
        } else {
            Serial.println("⚠️  Status: WiFi disconnected, attempting reconnection...");
        }
    }
    
    delay(10);
}
