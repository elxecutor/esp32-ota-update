/**
 * @file AsyncWebServerExample.ino
 * @brief Example using AsyncWebServer with modular OTA
 * 
 * This example demonstrates how to integrate the modular OTA system
 * with AsyncWebServer for high-performance web applications.
 */

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ModularOTA.h>

// Create AsyncWebServer instance
AsyncWebServer server(80);

// Configuration for the modular OTA system
ModularOTA::Config otaConfig = {
    .ssid = "YourWiFiSSID",
    .password = "YourWiFiPassword",
    .autoReconnect = true,
    .reconnectInterval = 30000,
    .enablePersistence = true,
    .serverPort = 3232,              // Different port for OTA server
    .otaPath = "/update",
    .authUsername = "admin",         // Optional authentication
    .authPassword = "ota123",
    .enableCORS = true,
    .enableProgress = true,
    .maxUploadSize = 2097152         // 2MB max upload
};

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== AsyncWebServer + Modular OTA Example ===");
    
    // Set modular OTA system callback
    ModularOTA::setCallback([](ModularOTA::Event event, const String& message, int value) {
        switch (event) {
            case ModularOTA::Event::NETWORK_CONNECTED:
                Serial.println("✓ Network connected: " + message);
                Serial.println("🌐 Main Server: http://" + WiFi.localIP().toString());
                Serial.println("🔧 OTA Server: " + ModularOTA::getOTAUrl());
                break;
                
            case ModularOTA::Event::OTA_STARTED:
                Serial.println("🔄 OTA update started: " + message);
                // You could notify connected WebSocket clients here
                break;
                
            case ModularOTA::Event::OTA_PROGRESS:
                Serial.printf("📊 OTA progress: %d%%\n", value);
                // You could broadcast progress to WebSocket clients here
                break;
                
            case ModularOTA::Event::OTA_COMPLETED:
                Serial.println("✅ OTA completed: " + message);
                break;
                
            case ModularOTA::Event::OTA_FAILED:
                Serial.println("❌ OTA failed: " + message);
                break;
                
            default:
                break;
        }
    });
    
    // Initialize modular OTA system
    if (ModularOTA::begin(otaConfig)) {
        Serial.println("✅ Modular OTA system initialized!");
    } else {
        Serial.println("❌ Failed to initialize Modular OTA system!");
        return;
    }
    
    // Setup AsyncWebServer routes
    setupWebServer();
    
    // Start AsyncWebServer
    server.begin();
    Serial.println("🌐 AsyncWebServer started on port 80");
    
    Serial.println("Setup completed!");
    Serial.println("📝 Available endpoints on main server:");
    Serial.println("   / - Main page");
    Serial.println("   /api/status - System status API");
    Serial.println("   /api/ota-info - OTA information API");
    Serial.println("   /ws - WebSocket for real-time updates");
}

void loop() {
    // Handle modular OTA system
    ModularOTA::handle();
    
    // Your async application logic here
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 10000) { // Every 10 seconds
        lastUpdate = millis();
        
        // Example: Check system health
        if (ModularOTA::isReady()) {
            // System is healthy
            size_t freeHeap, totalHeap, minFreeHeap;
            ModularOTA::getMemoryInfo(freeHeap, totalHeap, minFreeHeap);
            
            if (freeHeap < 50000) { // Less than 50KB free
                Serial.println("⚠️  Low memory warning: " + String(freeHeap) + " bytes free");
            }
        }
    }
    
    delay(10);
}

void setupWebServer() {
    // Serve static main page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = R"HTML(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 AsyncWebServer + Modular OTA</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background-color: #f0f0f0; }
        .container { max-width: 800px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; }
        .status-card { background: #e8f4fd; border: 1px solid #bee5eb; border-radius: 5px; padding: 15px; margin: 10px 0; }
        .ota-link { display: inline-block; background: #007bff; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; margin: 10px 0; }
        .ota-link:hover { background: #0056b3; }
        #status { margin: 20px 0; }
        .metric { display: inline-block; margin: 10px 20px 10px 0; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 AsyncWebServer + Modular OTA</h1>
        
        <div class="status-card">
            <h3>System Status</h3>
            <div id="status">Loading...</div>
        </div>
        
        <div class="status-card">
            <h3>OTA Update</h3>
            <p>Use the dedicated OTA server for firmware updates:</p>
            <a href="#" id="otaLink" class="ota-link" target="_blank">Open OTA Interface</a>
            <div id="otaStatus">Loading OTA status...</div>
        </div>
        
        <div class="status-card">
            <h3>API Endpoints</h3>
            <ul>
                <li><a href="/api/status">/api/status</a> - System status JSON</li>
                <li><a href="/api/ota-info">/api/ota-info</a> - OTA information JSON</li>
            </ul>
        </div>
    </div>
    
    <script>
        function updateStatus() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('status').innerHTML = `
                        <div class="metric"><strong>Uptime:</strong> ${Math.floor(data.uptime / 1000)} seconds</div>
                        <div class="metric"><strong>Free Heap:</strong> ${data.freeHeap} bytes</div>
                        <div class="metric"><strong>WiFi:</strong> ${data.network.connected ? 'Connected' : 'Disconnected'}</div>
                        <div class="metric"><strong>IP:</strong> ${data.network.ip}</div>
                        <div class="metric"><strong>RSSI:</strong> ${data.network.rssi} dBm</div>
                    `;
                })
                .catch(error => {
                    document.getElementById('status').innerHTML = 'Error loading status';
                });
            
            fetch('/api/ota-info')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('otaLink').href = data.otaUrl;
                    document.getElementById('otaStatus').innerHTML = `
                        <div class="metric"><strong>OTA Status:</strong> ${data.status}</div>
                        <div class="metric"><strong>Progress:</strong> ${data.progress}%</div>
                        <div class="metric"><strong>URL:</strong> <a href="${data.otaUrl}" target="_blank">${data.otaUrl}</a></div>
                    `;
                })
                .catch(error => {
                    document.getElementById('otaStatus').innerHTML = 'Error loading OTA info';
                });
        }
        
        // Update status immediately and then every 5 seconds
        updateStatus();
        setInterval(updateStatus, 5000);
    </script>
</body>
</html>
        )HTML";
        
        request->send(200, "text/html", html);
    });
    
    // API endpoint for system status
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"uptime\":" + String(millis()) + ",";
        json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
        json += "\"minFreeHeap\":" + String(ESP.getMinFreeHeap()) + ",";
        json += "\"chipModel\":\"" + String(ESP.getChipModel()) + "\",";
        json += "\"network\":{";
        json += "\"connected\":" + String(WiFi.isConnected() ? "true" : "false") + ",";
        json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI());
        json += "}";
        json += "}";
        
        request->send(200, "application/json", json);
    });
    
    // API endpoint for OTA information
    server.on("/api/ota-info", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"otaUrl\":\"" + ModularOTA::getOTAUrl() + "\",";
        json += "\"status\":\"" + String((int)OTACore::getStatus()) + "\",";
        json += "\"progress\":" + String(OTACore::getProgress()) + ",";
        json += "\"ready\":" + String(ModularOTA::isReady() ? "true" : "false") + ",";
        json += "\"persistent\":" + String(OTACore::isPersistent() ? "true" : "false");
        json += "}";
        
        request->send(200, "application/json", json);
    });
    
    // WebSocket handler for real-time updates (optional)
    AsyncWebSocket ws("/ws");
    ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, 
                  AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.println("🔌 WebSocket client connected: " + client->remoteIP().toString());
            
            // Send initial status
            String status = "{\"type\":\"status\",\"message\":\"Connected to ESP32\"}";
            client->text(status);
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.println("🔌 WebSocket client disconnected");
        } else if (type == WS_EVT_DATA) {
            // Handle incoming WebSocket data if needed
            String message = String((char*)data, len);
            Serial.println("📨 WebSocket message: " + message);
        }
    });
    
    server.addHandler(&ws);
    
    // Handle CORS for API endpoints
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
    
    // 404 handler
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });
}
