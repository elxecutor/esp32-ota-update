#include "ModularOTA.h"

// Static member definitions
ModularOTA::Config ModularOTA::_config;
ModularOTA::CallbackFunction ModularOTA::_callback = nullptr;
bool ModularOTA::_initialized = false;
bool ModularOTA::_networkEnabled = true;
bool ModularOTA::_otaEnabled = true;
bool ModularOTA::_serverEnabled = true;

bool ModularOTA::begin(const Config& config) {
    if (_initialized) {
        Serial.println("[ModularOTA] System already initialized");
        return true;
    }

    _config = config;

    Serial.println("\n=== Modular OTA System Initialization ===");
    logSystemInfo();

    // Validate configuration
    if (_config.ssid.length() == 0) {
        Serial.println("[ModularOTA] ERROR: SSID is required");
        return false;
    }

    // Initialize components
    if (!initializeComponents()) {
        Serial.println("[ModularOTA] ERROR: Failed to initialize components");
        return false;
    }

    _initialized = true;
    Serial.println("[ModularOTA] System initialized successfully");
    Serial.println("==========================================\n");

    sendEvent(Event::NETWORK_CONNECTED, "Modular OTA system ready");
    return true;
}

void ModularOTA::setCallback(CallbackFunction callback) {
    _callback = callback;
}

void ModularOTA::handle() {
    if (!_initialized) return;

    // Handle network manager
    if (_networkEnabled) {
        NetworkManager::handle();
    }

    // Handle OTA core
    if (_otaEnabled) {
        OTACore::handle();
    }

    // Handle web server
    if (_serverEnabled) {
        OTAWebServer::handle();
    }
}

void ModularOTA::stop() {
    if (!_initialized) return;

    Serial.println("[ModularOTA] Stopping system...");

    if (_serverEnabled) {
        OTAWebServer::stop();
    }

    if (_networkEnabled) {
        NetworkManager::disconnect();
    }

    _initialized = false;
    sendEvent(Event::SERVER_STOPPED, "Modular OTA system stopped");
    
    Serial.println("[ModularOTA] System stopped");
}

bool ModularOTA::isReady() {
    if (!_initialized) return false;

    bool networkReady = !_networkEnabled || NetworkManager::isConnected();
    bool otaReady = !_otaEnabled || (OTACore::getStatus() != OTACore::Status::ERROR);
    bool serverReady = !_serverEnabled || OTAWebServer::isRunning();

    return networkReady && otaReady && serverReady;
}

ModularOTA::Config ModularOTA::getConfig() {
    return _config;
}

bool ModularOTA::updateConfig(const Config& config) {
    bool needsRestart = false;

    // Check if network settings changed
    if (_config.ssid != config.ssid || _config.password != config.password) {
        needsRestart = true;
    }

    // Check if server settings changed
    if (_config.serverPort != config.serverPort || _config.otaPath != config.otaPath) {
        needsRestart = true;
    }

    _config = config;

    if (needsRestart && _initialized) {
        Serial.println("[ModularOTA] Configuration changed, restarting system...");
        stop();
        return begin(_config);
    }

    return true;
}

bool ModularOTA::getSystemStatus(NetworkManager::Status& networkStatus, 
                                OTACore::Status& otaStatus, 
                                bool& serverStatus) {
    if (!_initialized) return false;

    networkStatus = NetworkManager::getStatus();
    otaStatus = OTACore::getStatus();
    serverStatus = OTAWebServer::isRunning();

    return true;
}

String ModularOTA::getSystemInfoJSON() {
    String json = "{";
    
    // System info
    json += "\"system\":{";
    json += "\"initialized\":" + String(_initialized ? "true" : "false") + ",";
    json += "\"ready\":" + String(isReady() ? "true" : "false") + ",";
    json += "\"uptime\":" + String(millis()) + ",";
    json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"minFreeHeap\":" + String(ESP.getMinFreeHeap()) + ",";
    json += "\"chipModel\":\"" + String(ESP.getChipModel()) + "\",";
    json += "\"chipRevision\":" + String(ESP.getChipRevision()) + ",";
    json += "\"flashSize\":" + String(ESP.getFlashChipSize()) + ",";
    json += "\"sketchSize\":" + String(ESP.getSketchSize()) + ",";
    json += "\"freeSketchSpace\":" + String(ESP.getFreeSketchSpace());
    json += "},";

    // Network info
    json += "\"network\":{";
    json += "\"enabled\":" + String(_networkEnabled ? "true" : "false") + ",";
    json += "\"connected\":" + String(NetworkManager::isConnected() ? "true" : "false") + ",";
    json += "\"status\":" + String((int)NetworkManager::getStatus()) + ",";
    json += "\"ssid\":\"" + NetworkManager::getSSID() + "\",";
    json += "\"ip\":\"" + NetworkManager::getIPAddress() + "\",";
    json += "\"rssi\":" + String(NetworkManager::getRSSI()) + ",";
    json += "\"autoReconnect\":" + String(NetworkManager::isAutoReconnectEnabled() ? "true" : "false");
    json += "},";

    // OTA info
    json += "\"ota\":{";
    json += "\"enabled\":" + String(_otaEnabled ? "true" : "false") + ",";
    json += "\"status\":" + String((int)OTACore::getStatus()) + ",";
    json += "\"progress\":" + String(OTACore::getProgress()) + ",";
    json += "\"active\":" + String(OTACore::isActive() ? "true" : "false") + ",";
    json += "\"persistent\":" + String(OTACore::isPersistent() ? "true" : "false") + ",";
    json += "\"availableSize\":" + String(OTACore::getAvailableSize()) + ",";
    json += "\"lastError\":\"" + OTACore::getLastError() + "\"";
    json += "},";

    // Server info
    json += "\"server\":{";
    json += "\"enabled\":" + String(_serverEnabled ? "true" : "false") + ",";
    json += "\"running\":" + String(OTAWebServer::isRunning() ? "true" : "false") + ",";
    json += "\"port\":" + String(_config.serverPort) + ",";
    json += "\"path\":\"" + _config.otaPath + "\",";
    json += "\"otaUrl\":\"" + getOTAUrl() + "\",";
    json += "\"clientCount\":" + String(OTAWebServer::getClientCount()) + ",";
    json += "\"authEnabled\":" + String(_config.authUsername.length() > 0 ? "true" : "false");
    json += "}";

    json += "}";
    return json;
}

bool ModularOTA::setComponentsEnabled(bool enableNetwork, bool enableOTA, bool enableServer) {
    _networkEnabled = enableNetwork;
    _otaEnabled = enableOTA;
    _serverEnabled = enableServer;

    Serial.println("[ModularOTA] Components enabled - Network: " + String(enableNetwork) + 
                   ", OTA: " + String(enableOTA) + ", Server: " + String(enableServer));

    return true;
}

void ModularOTA::restart() {
    Serial.println("[ModularOTA] System restart requested");
    sendEvent(Event::OTA_COMPLETED, "System restarting...");
    
    delay(1000);
    ESP.restart();
}

String ModularOTA::getOTAUrl() {
    if (!NetworkManager::isConnected()) {
        return "";
    }
    
    return "http://" + NetworkManager::getIPAddress() + ":" + String(_config.serverPort) + _config.otaPath;
}

bool ModularOTA::addCustomEndpoint(const String& path, std::function<void()> handler) {
    if (_serverEnabled) {
        OTAWebServer::addCustomEndpoint(path, handler);
        return true;
    }
    return false;
}

bool ModularOTA::getMemoryInfo(size_t& freeHeap, size_t& totalHeap, size_t& minFreeHeap) {
    freeHeap = ESP.getFreeHeap();
    totalHeap = ESP.getHeapSize();
    minFreeHeap = ESP.getMinFreeHeap();
    return true;
}

void ModularOTA::onNetworkEvent(NetworkManager::Status status, const String& message) {
    switch (status) {
        case NetworkManager::Status::CONNECTED:
            Serial.println("[ModularOTA] Network connected: " + message);
            sendEvent(Event::NETWORK_CONNECTED, message);
            break;
            
        case NetworkManager::Status::DISCONNECTED:
            Serial.println("[ModularOTA] Network disconnected: " + message);
            sendEvent(Event::NETWORK_DISCONNECTED, message);
            break;
            
        default:
            break;
    }
}

void ModularOTA::onOTAEvent(OTACore::Status status, int progress, const String& message) {
    switch (status) {
        case OTACore::Status::RECEIVING:
            if (progress == 0) {
                Serial.println("[ModularOTA] OTA started: " + message);
                sendEvent(Event::OTA_STARTED, message);
            } else {
                sendEvent(Event::OTA_PROGRESS, message, progress);
            }
            break;
            
        case OTACore::Status::COMPLETE:
            Serial.println("[ModularOTA] OTA completed: " + message);
            sendEvent(Event::OTA_COMPLETED, message, 100);
            break;
            
        case OTACore::Status::ERROR:
            Serial.println("[ModularOTA] OTA failed: " + message);
            sendEvent(Event::OTA_FAILED, message);
            break;
            
        default:
            break;
    }
}

void ModularOTA::onServerEvent(OTAWebServer::Event event, const String& message, int value) {
    switch (event) {
        case OTAWebServer::Event::STARTED:
            Serial.println("[ModularOTA] Server started: " + message);
            sendEvent(Event::SERVER_STARTED, message);
            break;
            
        case OTAWebServer::Event::STOPPED:
            Serial.println("[ModularOTA] Server stopped: " + message);
            sendEvent(Event::SERVER_STOPPED, message);
            break;
            
        default:
            break;
    }
}

void ModularOTA::sendEvent(Event event, const String& message, int value) {
    if (_callback) {
        _callback(event, message, value);
    }
}

bool ModularOTA::initializeComponents() {
    // Initialize OTA Core
    if (_otaEnabled) {
        if (!OTACore::begin(_config.enablePersistence)) {
            Serial.println("[ModularOTA] Failed to initialize OTA Core");
            return false;
        }
        OTACore::setCallback(onOTAEvent);
        Serial.println("[ModularOTA] OTA Core initialized");
    }

    // Initialize Network Manager
    if (_networkEnabled) {
        if (!NetworkManager::begin(_config.ssid.c_str(), _config.password.c_str(), _config.autoReconnect)) {
            Serial.println("[ModularOTA] Failed to initialize Network Manager");
            return false;
        }
        NetworkManager::setCallback(onNetworkEvent);
        NetworkManager::setReconnectInterval(_config.reconnectInterval);
        Serial.println("[ModularOTA] Network Manager initialized");

        // Connect to WiFi
        if (!NetworkManager::connect()) {
            Serial.println("[ModularOTA] Warning: Failed to connect to WiFi initially");
        }
    }

    // Initialize OTA Web Server
    if (_serverEnabled) {
        OTAWebServer::Config serverConfig;
        serverConfig.port = _config.serverPort;
        serverConfig.path = _config.otaPath;
        serverConfig.username = _config.authUsername;
        serverConfig.password = _config.authPassword;
        serverConfig.enableCORS = _config.enableCORS;
        serverConfig.enableProgress = _config.enableProgress;
        serverConfig.maxUploadSize = _config.maxUploadSize;

        if (!OTAWebServer::begin(serverConfig)) {
            Serial.println("[ModularOTA] Failed to initialize OTA Web Server");
            return false;
        }
        OTAWebServer::setCallback(onServerEvent);
        Serial.println("[ModularOTA] OTA Web Server initialized");
    }

    return true;
}

void ModularOTA::logSystemInfo() {
    Serial.println("Chip Model: " + String(ESP.getChipModel()));
    Serial.println("Chip Revision: " + String(ESP.getChipRevision()));
    Serial.println("Flash Size: " + String(ESP.getFlashChipSize()) + " bytes");
    Serial.println("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
    Serial.println("Sketch Size: " + String(ESP.getSketchSize()) + " bytes");
    Serial.println("Free Sketch Space: " + String(ESP.getFreeSketchSpace()) + " bytes");
    
    Serial.println("\nConfiguration:");
    Serial.println("SSID: " + _config.ssid);
    Serial.println("OTA Port: " + String(_config.serverPort));
    Serial.println("OTA Path: " + _config.otaPath);
    Serial.println("Persistence: " + String(_config.enablePersistence ? "enabled" : "disabled"));
    Serial.println("Auto-reconnect: " + String(_config.autoReconnect ? "enabled" : "disabled"));
}
