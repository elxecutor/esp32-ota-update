#include "NetworkManager.h"

// Static member definitions
NetworkManager::Status NetworkManager::_status = Status::DISCONNECTED;
String NetworkManager::_ssid = "";
String NetworkManager::_password = "";
bool NetworkManager::_autoReconnect = true;
unsigned long NetworkManager::_reconnectInterval = 30000; // 30 seconds
unsigned long NetworkManager::_lastReconnectAttempt = 0;
NetworkManager::CallbackFunction NetworkManager::_callback = nullptr;
int NetworkManager::_reconnectAttempts = 0;

bool NetworkManager::begin(const char* ssid, const char* password, bool autoReconnect) {
    if (!ssid || strlen(ssid) == 0) {
        Serial.println("[NetworkManager] Invalid SSID");
        return false;
    }

    _ssid = String(ssid);
    _password = String(password);
    _autoReconnect = autoReconnect;
    _status = Status::DISCONNECTED;
    _reconnectAttempts = 0;

    // Set WiFi mode to station
    WiFi.mode(WIFI_STA);
    
    // Register WiFi event handler
    WiFi.onEvent(onWiFiEvent);

    Serial.println("[NetworkManager] Network manager initialized");
    Serial.println("[NetworkManager] SSID: " + _ssid);
    Serial.println("[NetworkManager] Auto-reconnect: " + String(_autoReconnect ? "enabled" : "disabled"));
    
    return true;
}

void NetworkManager::setCallback(CallbackFunction callback) {
    _callback = callback;
}

bool NetworkManager::connect(unsigned long timeout) {
    if (_status == Status::CONNECTED) {
        return true;
    }

    updateStatus(Status::CONNECTING, "Connecting to WiFi...");
    
    WiFi.begin(_ssid.c_str(), _password.c_str());
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
        delay(100);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        updateStatus(Status::CONNECTED, "Connected to " + _ssid + " (IP: " + WiFi.localIP().toString() + ")");
        _reconnectAttempts = 0;
        return true;
    } else {
        updateStatus(Status::FAILED, "Failed to connect to " + _ssid);
        return false;
    }
}

void NetworkManager::disconnect() {
    WiFi.disconnect(true);
    updateStatus(Status::DISCONNECTED, "Disconnected from WiFi");
}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED && _status == Status::CONNECTED;
}

NetworkManager::Status NetworkManager::getStatus() {
    return _status;
}

String NetworkManager::getIPAddress() {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

int NetworkManager::getRSSI() {
    if (isConnected()) {
        return WiFi.RSSI();
    }
    return -100; // No signal
}

String NetworkManager::getSSID() {
    if (isConnected()) {
        return WiFi.SSID();
    }
    return _ssid;
}

void NetworkManager::handle() {
    // Handle auto-reconnection
    if (_autoReconnect && !isConnected() && _status != Status::CONNECTING) {
        unsigned long currentTime = millis();
        
        if (currentTime - _lastReconnectAttempt >= _reconnectInterval) {
            if (_reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
                attemptReconnect();
            } else {
                // Reset attempts after extended wait
                if (currentTime - _lastReconnectAttempt >= (_reconnectInterval * 10)) {
                    _reconnectAttempts = 0;
                }
            }
        }
    }
}

void NetworkManager::setAutoReconnect(bool enable) {
    _autoReconnect = enable;
    Serial.println("[NetworkManager] Auto-reconnect " + String(enable ? "enabled" : "disabled"));
}

bool NetworkManager::isAutoReconnectEnabled() {
    return _autoReconnect;
}

void NetworkManager::setReconnectInterval(unsigned long interval) {
    _reconnectInterval = interval;
    Serial.println("[NetworkManager] Reconnect interval set to " + String(interval) + "ms");
}

bool NetworkManager::getNetworkInfo(IPAddress& ip, IPAddress& gateway, IPAddress& subnet) {
    if (!isConnected()) {
        return false;
    }
    
    ip = WiFi.localIP();
    gateway = WiFi.gatewayIP();
    subnet = WiFi.subnetMask();
    
    return true;
}

void NetworkManager::onWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("[NetworkManager] WiFi started");
            break;
            
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("[NetworkManager] WiFi connected");
            break;
            
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            updateStatus(Status::CONNECTED, "Got IP: " + WiFi.localIP().toString());
            break;
            
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            if (_status == Status::CONNECTED) {
                updateStatus(Status::DISCONNECTED, "WiFi disconnected");
            }
            break;
            
        default:
            break;
    }
}

void NetworkManager::updateStatus(Status newStatus, const String& message) {
    if (_status != newStatus) {
        _status = newStatus;
        
        String statusStr;
        switch (newStatus) {
            case Status::DISCONNECTED: statusStr = "DISCONNECTED"; break;
            case Status::CONNECTING: statusStr = "CONNECTING"; break;
            case Status::CONNECTED: statusStr = "CONNECTED"; break;
            case Status::FAILED: statusStr = "FAILED"; break;
            case Status::RECONNECTING: statusStr = "RECONNECTING"; break;
        }
        
        Serial.println("[NetworkManager] Status: " + statusStr + (message.length() > 0 ? " - " + message : ""));
        
        if (_callback) {
            _callback(newStatus, message);
        }
    }
}

void NetworkManager::attemptReconnect() {
    _reconnectAttempts++;
    _lastReconnectAttempt = millis();
    
    updateStatus(Status::RECONNECTING, "Reconnect attempt " + String(_reconnectAttempts) + "/" + String(MAX_RECONNECT_ATTEMPTS));
    
    WiFi.disconnect();
    delay(1000);
    WiFi.begin(_ssid.c_str(), _password.c_str());
}
