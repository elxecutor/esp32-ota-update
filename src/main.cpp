
/**
 * @file main.cpp
 * @brief Main application using backward compatible modular OTA
 * 
 * This demonstrates the backward compatibility layer that provides
 * a drop-in replacement for ElegantOTA while using the new modular
 * architecture underneath.
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ElegantOTACompat.h>
#include <NetworkManager.h>

const char *ssid = "Wokwi-GUEST";
const char *password = "";

WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32 Modular OTA Demo ===");
  
  // Initialize network manager with auto-reconnection
  Serial.println("Initializing Network Manager...");
  if (NetworkManager::begin(ssid, password, true)) {
    Serial.println("✅ Network Manager initialized");
  } else {
    Serial.println("❌ Network Manager initialization failed");
    return;
  }

  // Set network event callback
  NetworkManager::setCallback([](NetworkManager::Status status, const String& message) {
    switch (status) {
      case NetworkManager::Status::CONNECTED:
        Serial.println("✓ Network connected: " + message);
        Serial.println("📡 IP address: " + NetworkManager::getIPAddress());
        Serial.println("🔗 OTA URL: " + ElegantOTACompat::getOTAUrl());
        break;
      case NetworkManager::Status::DISCONNECTED:
        Serial.println("✗ Network disconnected: " + message);
        break;
      case NetworkManager::Status::RECONNECTING:
        Serial.println("🔄 Reconnecting: " + message);
        break;
      default:
        break;
    }
  });

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  if (NetworkManager::connect()) {
    Serial.println("✅ WiFi connected successfully!");
  } else {
    Serial.println("⚠️  Initial WiFi connection failed, will auto-retry...");
  }

  // Setup web server routes
  server.on("/", []() {
    String html = R"HTML(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Modular OTA Demo</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #f0f0f0; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; }
        .info { background: #e8f4fd; border: 1px solid #bee5eb; border-radius: 5px; padding: 15px; margin: 15px 0; }
        .ota-button { display: inline-block; background: #007bff; color: white; padding: 15px 30px; text-decoration: none; border-radius: 5px; margin: 10px 0; font-size: 16px; }
        .ota-button:hover { background: #0056b3; }
        .status { margin: 10px 0; }
        .success { color: #28a745; }
        .warning { color: #ffc107; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 ESP32 Modular OTA Demo</h1>
        
        <div class="info">
            <h3>Welcome to the Modular OTA System!</h3>
            <p>This demo shows the new modular OTA architecture that provides:</p>
            <ul>
                <li>✅ <strong>Persistent OTA logic</strong> - Survives firmware updates</li>
                <li>✅ <strong>Automatic WiFi reconnection</strong> - Never lose connectivity</li>
                <li>✅ <strong>Backward compatibility</strong> - Drop-in replacement for ElegantOTA</li>
                <li>✅ <strong>Decoupled components</strong> - OTA independent of main app</li>
                <li>✅ <strong>Memory management</strong> - Optimized for reliable updates</li>
            </ul>
        </div>
        
        <div class="info">
            <h3>📊 System Status</h3>
            <div class="status">
                <strong>Uptime:</strong> <span id="uptime">Loading...</span><br>
                <strong>Free Memory:</strong> <span id="memory">Loading...</span><br>
                <strong>WiFi Status:</strong> <span id="wifi">Loading...</span><br>
                <strong>OTA Status:</strong> <span id="ota">Loading...</span>
            </div>
        </div>
        
        <div class="info">
            <h3>🔧 OTA Update</h3>
            <p>Click the button below to access the OTA update interface:</p>
            <a href="/update" class="ota-button">🔄 Open OTA Interface</a>
            <p><small>The OTA interface runs on the same server but uses modular components underneath.</small></p>
        </div>
        
        <div class="info">
            <h3>📚 Learn More</h3>
            <p>This implementation demonstrates backward compatibility while using the new modular architecture.</p>
            <p>Check the <code>/docs</code> folder for complete documentation and additional examples.</p>
        </div>
    </div>
    
    <script>
        function updateStatus() {
            // This would normally fetch from API endpoints
            // For demo purposes, showing static content
            document.getElementById('uptime').innerHTML = Math.floor(Date.now() / 1000) + ' seconds';
            document.getElementById('memory').innerHTML = 'Available';
            document.getElementById('wifi').innerHTML = '<span class="success">Connected</span>';
            document.getElementById('ota').innerHTML = '<span class="success">Ready</span>';
        }
        
        updateStatus();
        setInterval(updateStatus, 5000);
    </script>
</body>
</html>
    )HTML";
    
    server.send(200, "text/html", html);
  });

  server.on("/info", []() {
    String info = "ESP32 Modular OTA System Information\n";
    info += "=====================================\n\n";
    info += "Hardware:\n";
    info += "  Chip Model: " + String(ESP.getChipModel()) + "\n";
    info += "  Chip Revision: " + String(ESP.getChipRevision()) + "\n";
    info += "  Flash Size: " + String(ESP.getFlashChipSize()) + " bytes\n";
    info += "  Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
    info += "  Min Free Heap: " + String(ESP.getMinFreeHeap()) + " bytes\n\n";
    
    info += "Network:\n";
    info += "  SSID: " + NetworkManager::getSSID() + "\n";
    info += "  IP Address: " + NetworkManager::getIPAddress() + "\n";
    info += "  Signal Strength: " + String(NetworkManager::getRSSI()) + " dBm\n";
    info += "  Auto-reconnect: " + String(NetworkManager::isAutoReconnectEnabled() ? "Enabled" : "Disabled") + "\n\n";
    
    info += "OTA Status:\n";
    info += "  System Ready: " + String(ElegantOTACompat::isRunning() ? "Yes" : "No") + "\n";
    info += "  Update Active: " + String(ElegantOTACompat::isUpdating() ? "Yes" : "No") + "\n";
    info += "  Progress: " + String(ElegantOTACompat::getProgress()) + "%\n";
    info += "  OTA URL: " + ElegantOTACompat::getOTAUrl() + "\n\n";
    
    info += "Modular Features:\n";
    info += "  ✅ Persistent OTA logic across updates\n";
    info += "  ✅ Automatic network reconnection\n";
    info += "  ✅ Decoupled OTA and web server\n";
    info += "  ✅ Backward compatibility with ElegantOTA\n";
    info += "  ✅ Memory-optimized upload handling\n";
    
    server.send(200, "text/plain", info);
  });

  // Initialize ElegantOTA compatibility layer
  // This is a drop-in replacement for ElegantOTA.begin(&server)
  Serial.println("Initializing ElegantOTA compatibility layer...");
  if (ElegantOTACompat::begin(&server, "/update")) {
    Serial.println("✅ ElegantOTA compatibility layer initialized!");
  } else {
    Serial.println("❌ Failed to initialize ElegantOTA compatibility layer!");
    return;
  }

  // Set OTA event callbacks (same API as original ElegantOTA)
  ElegantOTACompat::onStart([]() {
    Serial.println("🔄 OTA update started!");
  });

  ElegantOTACompat::onEnd([]() {
    Serial.println("✅ OTA update completed successfully!");
  });

  ElegantOTACompat::onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress * 100) / total;
    static int lastPercent = -1;
    if (percent != lastPercent && percent % 10 == 0) {
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
  Serial.println("\n🎉 Setup completed successfully!");
  Serial.println("📝 Available endpoints:");
  Serial.println("   http://" + NetworkManager::getIPAddress() + "/ - Main page");
  Serial.println("   http://" + NetworkManager::getIPAddress() + "/info - System information");
  Serial.println("   http://" + NetworkManager::getIPAddress() + "/update - OTA interface");
  Serial.println("\n⚡ The system now provides persistent OTA with automatic recovery!");
}

void loop() {
  // Handle network management (automatic reconnection, monitoring)
  NetworkManager::handle();
  
  // Handle web server requests
  server.handleClient();
  
  // Handle ElegantOTA (replaces ElegantOTA.loop() with modular implementation)
  ElegantOTACompat::loop();

  // Application status monitoring
  static unsigned long lastStatus = 0;
  if (millis() - lastStatus > 60000) { // Every minute
    lastStatus = millis();
    
    Serial.println("📊 === System Status Report ===");
    Serial.println("⏰ Uptime: " + String(millis() / 1000) + " seconds");
    Serial.println("💾 Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
    
    if (NetworkManager::isConnected()) {
      Serial.println("🌐 Network: Connected (" + NetworkManager::getIPAddress() + 
                    ", RSSI: " + String(NetworkManager::getRSSI()) + " dBm)");
    } else {
      Serial.println("🌐 Network: Disconnected (auto-reconnect active)");
    }
    
    if (ElegantOTACompat::isUpdating()) {
      Serial.println("🔄 OTA: Update in progress (" + String(ElegantOTACompat::getProgress()) + "%)");
    } else {
      Serial.println("🔧 OTA: Ready for updates");
    }
    
    Serial.println("===============================\n");
  }

  // Small delay to prevent watchdog issues
  delay(10);
}