/**
 * @file ModularCompleteExample.ino
 * @brief Complete example using the new modular OTA architecture
 * 
 * This example demonstrates how to use the new modular OTA system
 * with all components working together.
 */

#include <ModularOTA.h>

// Configuration for the modular OTA system
ModularOTA::Config otaConfig = {
    .ssid = "YourWiFiSSID",                // Replace with your WiFi SSID
    .password = "YourWiFiPassword",        // Replace with your WiFi password
    .autoReconnect = true,                 // Enable auto-reconnect
    .reconnectInterval = 30000,            // Reconnect every 30 seconds
    .enablePersistence = true,             // Enable OTA persistence
    .serverPort = 3232,                    // OTA server port
    .otaPath = "/update",                  // OTA endpoint path
    .authUsername = "",                    // Optional: HTTP auth username
    .authPassword = "",                    // Optional: HTTP auth password
    .enableCORS = true,                    // Enable CORS headers
    .enableProgress = true,                // Enable progress endpoint
    .maxUploadSize = 1048576               // Max upload size (1MB)
};

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Modular OTA Complete Example ===");
    
    // Set system event callback
    ModularOTA::setCallback([](ModularOTA::Event event, const String& message, int value) {
        switch (event) {
            case ModularOTA::Event::NETWORK_CONNECTED:
                Serial.println("✓ Network connected: " + message);
                Serial.println("✓ OTA URL: " + ModularOTA::getOTAUrl());
                break;
                
            case ModularOTA::Event::NETWORK_DISCONNECTED:
                Serial.println("✗ Network disconnected: " + message);
                break;
                
            case ModularOTA::Event::OTA_STARTED:
                Serial.println("🔄 OTA update started: " + message);
                break;
                
            case ModularOTA::Event::OTA_PROGRESS:
                Serial.println("📊 OTA progress: " + String(value) + "%");
                break;
                
            case ModularOTA::Event::OTA_COMPLETED:
                Serial.println("✅ OTA update completed: " + message);
                break;
                
            case ModularOTA::Event::OTA_FAILED:
                Serial.println("❌ OTA update failed: " + message);
                break;
                
            case ModularOTA::Event::SERVER_STARTED:
                Serial.println("🌐 OTA server started: " + message);
                break;
                
            case ModularOTA::Event::SERVER_STOPPED:
                Serial.println("🛑 OTA server stopped: " + message);
                break;
        }
    });
    
    // Initialize the modular OTA system
    if (ModularOTA::begin(otaConfig)) {
        Serial.println("✅ Modular OTA system initialized successfully!");
    } else {
        Serial.println("❌ Failed to initialize Modular OTA system!");
        return;
    }
    
    // Add a custom endpoint for system status
    ModularOTA::addCustomEndpoint("/status", []() {
        // This will be handled by the OTA web server
        // The response is handled internally
    });
    
    Serial.println("Setup completed. System is ready for OTA updates.");
    Serial.println("Visit the OTA URL to upload firmware.");
}

void loop() {
    // Handle all OTA system components
    ModularOTA::handle();
    
    // Your application code goes here
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 30000) { // Print status every 30 seconds
        lastPrint = millis();
        
        if (ModularOTA::isReady()) {
            Serial.println("📊 System Status: All components operational");
            Serial.println("🌐 OTA URL: " + ModularOTA::getOTAUrl());
            
            // Print memory usage
            size_t freeHeap, totalHeap, minFreeHeap;
            if (ModularOTA::getMemoryInfo(freeHeap, totalHeap, minFreeHeap)) {
                Serial.println("💾 Memory - Free: " + String(freeHeap) + 
                              ", Total: " + String(totalHeap) + 
                              ", Min Free: " + String(minFreeHeap));
            }
        } else {
            Serial.println("⚠️  System Status: Some components not ready");
        }
    }
    
    // Small delay to prevent watchdog issues
    delay(10);
}
