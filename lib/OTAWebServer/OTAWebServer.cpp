#include "OTAWebServer.h"
#include "NetworkManager.h"

// Static member definitions
WebServer* OTAWebServer::_server = nullptr;
OTAWebServer::Config OTAWebServer::_config;
OTAWebServer::CallbackFunction OTAWebServer::_callback = nullptr;
bool OTAWebServer::_running = false;
int OTAWebServer::_clientCount = 0;
unsigned long OTAWebServer::_uploadStartTime = 0;
size_t OTAWebServer::_uploadSize = 0;
size_t OTAWebServer::_uploadReceived = 0;

bool OTAWebServer::begin(const Config& config) {
    if (_running) {
        Serial.println("[OTAWebServer] Server already running");
        return false;
    }

    _config = config;
    
    // Create web server instance
    _server = new WebServer(_config.port);
    if (!_server) {
        Serial.println("[OTAWebServer] Failed to create server instance");
        return false;
    }

    // Setup routes
    setupRoutes();

    // Start server
    _server->begin();
    _running = true;

    Serial.println("[OTAWebServer] OTA Web Server started on port " + String(_config.port));
    Serial.println("[OTAWebServer] OTA endpoint: " + _config.path);
    
    if (_config.username.length() > 0) {
        Serial.println("[OTAWebServer] Authentication enabled");
    }

    sendEvent(Event::STARTED, "OTA Web Server started on port " + String(_config.port));
    return true;
}

void OTAWebServer::stop() {
    if (!_running || !_server) {
        return;
    }

    _server->stop();
    delete _server;
    _server = nullptr;
    _running = false;

    Serial.println("[OTAWebServer] OTA Web Server stopped");
    sendEvent(Event::STOPPED, "OTA Web Server stopped");
}

void OTAWebServer::setCallback(CallbackFunction callback) {
    _callback = callback;
}

void OTAWebServer::handle() {
    if (_running && _server) {
        _server->handleClient();
    }
}

bool OTAWebServer::isRunning() {
    return _running;
}

OTAWebServer::Config OTAWebServer::getConfig() {
    return _config;
}

bool OTAWebServer::updateConfig(const Config& config) {
    if (_running) {
        // Need to restart server with new config
        stop();
        return begin(config);
    } else {
        _config = config;
        return true;
    }
}

String OTAWebServer::getOTAUrl() {
    if (!NetworkManager::isConnected()) {
        return "";
    }
    
    return "http://" + NetworkManager::getIPAddress() + ":" + String(_config.port) + _config.path;
}

int OTAWebServer::getClientCount() {
    return _clientCount;
}

void OTAWebServer::addCustomEndpoint(const String& path, std::function<void()> handler) {
    if (_server) {
        _server->on(path, handler);
        Serial.println("[OTAWebServer] Added custom endpoint: " + path);
    }
}

void OTAWebServer::setAuthentication(const String& username, const String& password) {
    _config.username = username;
    _config.password = password;
    Serial.println("[OTAWebServer] Authentication updated");
}

void OTAWebServer::removeAuthentication() {
    _config.username = "";
    _config.password = "";
    Serial.println("[OTAWebServer] Authentication removed");
}

void OTAWebServer::setupRoutes() {
    if (!_server) return;

    // Main OTA upload page
    _server->on(_config.path, HTTP_GET, handleUpdate);
    _server->on(_config.path, HTTP_POST, handleUpdatePost, []() {
        HTTPUpload& upload = _server->upload();
        
        if (upload.status == UPLOAD_FILE_START) {
            _uploadStartTime = millis();
            _uploadSize = upload.totalSize;
            _uploadReceived = 0;
            
            Serial.println("[OTAWebServer] Upload started: " + upload.filename);
            sendEvent(Event::UPLOAD_START, "Upload started: " + upload.filename, upload.totalSize);
            
            if (!OTACore::startUpdate(upload.totalSize)) {
                _server->send(500, "text/plain", "Failed to start OTA update");
                return;
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            _uploadReceived += upload.currentSize;
            
            if (OTACore::writeData(upload.buf, upload.currentSize) != upload.currentSize) {
                _server->send(500, "text/plain", "Write error: " + OTACore::getLastError());
                return;
            }
            
            int progress = (_uploadReceived * 100) / _uploadSize;
            sendEvent(Event::UPLOAD_PROGRESS, "Upload progress", progress);
        } else if (upload.status == UPLOAD_FILE_END) {
            if (OTACore::finishUpdate()) {
                Serial.println("[OTAWebServer] Upload completed successfully");
                sendEvent(Event::UPLOAD_COMPLETE, "Upload completed successfully", 100);
            } else {
                Serial.println("[OTAWebServer] Upload failed: " + OTACore::getLastError());
                sendEvent(Event::UPLOAD_ERROR, "Upload failed: " + OTACore::getLastError());
            }
        }
    });

    // Progress endpoint
    if (_config.enableProgress) {
        _server->on(_config.path + "/progress", HTTP_GET, handleProgress);
    }

    // Status endpoint
    _server->on(_config.path + "/status", HTTP_GET, handleStatus);

    // Reboot endpoint
    _server->on(_config.path + "/reboot", HTTP_POST, handleReboot);

    // 404 handler
    _server->onNotFound(handleNotFound);
}

void OTAWebServer::handleUpdate() {
    if (!authenticate()) return;

    sendCORSHeaders();

    String html = R"HTML(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 OTA Update</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background-color: #f0f0f0; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; }
        .upload-area { border: 2px dashed #ccc; border-radius: 10px; padding: 40px; text-align: center; margin: 20px 0; }
        .upload-area:hover { border-color: #007bff; }
        input[type="file"] { margin: 20px 0; }
        button { background-color: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }
        button:hover { background-color: #0056b3; }
        button:disabled { background-color: #ccc; cursor: not-allowed; }
        .progress { width: 100%; background-color: #f0f0f0; border-radius: 10px; margin: 20px 0; height: 20px; overflow: hidden; }
        .progress-bar { height: 100%; background-color: #007bff; width: 0%; transition: width 0.3s; }
        .status { margin: 20px 0; padding: 10px; border-radius: 5px; }
        .status.success { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .status.error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .status.info { background-color: #d1ecf1; color: #0c5460; border: 1px solid #bee5eb; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 OTA Update</h1>
        <div class="upload-area">
            <p>Select firmware file (.bin) to upload</p>
            <form method="POST" action="#" enctype="multipart/form-data" id="uploadForm">
                <input type="file" name="update" accept=".bin" id="fileInput" required>
                <br>
                <button type="submit" id="uploadBtn">Upload Firmware</button>
            </form>
        </div>
        <div class="progress" id="progressContainer" style="display: none;">
            <div class="progress-bar" id="progressBar"></div>
        </div>
        <div id="status"></div>
    </div>

    <script>
        document.getElementById('uploadForm').addEventListener('submit', function(e) {
            e.preventDefault();
            uploadFile();
        });

        function uploadFile() {
            const fileInput = document.getElementById('fileInput');
            const file = fileInput.files[0];
            
            if (!file) {
                showStatus('Please select a file', 'error');
                return;
            }

            const formData = new FormData();
            formData.append('update', file);

            const uploadBtn = document.getElementById('uploadBtn');
            const progressContainer = document.getElementById('progressContainer');
            const progressBar = document.getElementById('progressBar');

            uploadBtn.disabled = true;
            uploadBtn.textContent = 'Uploading...';
            progressContainer.style.display = 'block';
            progressBar.style.width = '0%';

            const xhr = new XMLHttpRequest();

            xhr.upload.addEventListener('progress', function(e) {
                if (e.lengthComputable) {
                    const percentComplete = (e.loaded / e.total) * 100;
                    progressBar.style.width = percentComplete + '%';
                }
            });

            xhr.addEventListener('load', function() {
                if (xhr.status === 200) {
                    progressBar.style.width = '100%';
                    showStatus('Upload successful! Device will reboot...', 'success');
                    setTimeout(function() {
                        window.location.reload();
                    }, 3000);
                } else {
                    showStatus('Upload failed: ' + xhr.responseText, 'error');
                }
                uploadBtn.disabled = false;
                uploadBtn.textContent = 'Upload Firmware';
            });

            xhr.addEventListener('error', function() {
                showStatus('Upload failed: Network error', 'error');
                uploadBtn.disabled = false;
                uploadBtn.textContent = 'Upload Firmware';
            });

            xhr.open('POST', '')
            xhr.send(formData);
        }

        function showStatus(message, type) {
            const status = document.getElementById('status');
            status.innerHTML = '<div class="status ' + type + '">' + message + '</div>';
        }
    </script>
</body>
</html>
)HTML";

    _server->send(200, "text/html", html);
}

void OTAWebServer::handleUpdatePost() {
    if (!authenticate()) return;

    sendCORSHeaders();
    _server->send(200, "text/plain", "Update completed");
}

void OTAWebServer::handleProgress() {
    if (!authenticate()) return;

    sendCORSHeaders();
    _server->send(200, "application/json", getProgressJSON());
}

void OTAWebServer::handleStatus() {
    sendCORSHeaders();
    _server->send(200, "application/json", getStatusJSON());
}

void OTAWebServer::handleReboot() {
    if (!authenticate()) return;

    sendCORSHeaders();
    _server->send(200, "text/plain", "Rebooting...");
    
    delay(1000);
    ESP.restart();
}

void OTAWebServer::handleNotFound() {
    sendCORSHeaders();
    _server->send(404, "text/plain", "Not found");
}

void OTAWebServer::sendCORSHeaders() {
    if (_config.enableCORS) {
        _server->sendHeader("Access-Control-Allow-Origin", "*");
        _server->sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        _server->sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    }
}

bool OTAWebServer::authenticate() {
    if (_config.username.length() == 0) {
        return true; // No authentication required
    }

    if (!_server->authenticate(_config.username.c_str(), _config.password.c_str())) {
        _server->requestAuthentication();
        return false;
    }

    return true;
}

void OTAWebServer::sendEvent(Event event, const String& message, int value) {
    if (_callback) {
        _callback(event, message, value);
    }
}

String OTAWebServer::getStatusJSON() {
    String json = "{";
    json += "\"status\":\"" + String((int)OTACore::getStatus()) + "\",";
    json += "\"progress\":" + String(OTACore::getProgress()) + ",";
    json += "\"error\":\"" + OTACore::getLastError() + "\",";
    json += "\"uptime\":" + String(millis()) + ",";
    json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"chipId\":\"" + String(ESP.getEfuseMac(), HEX) + "\",";
    json += "\"flashSize\":" + String(ESP.getFlashChipSize()) + ",";
    json += "\"network\":{";
    json += "\"connected\":" + String(NetworkManager::isConnected() ? "true" : "false") + ",";
    json += "\"ip\":\"" + NetworkManager::getIPAddress() + "\",";
    json += "\"ssid\":\"" + NetworkManager::getSSID() + "\",";
    json += "\"rssi\":" + String(NetworkManager::getRSSI());
    json += "}";
    json += "}";
    return json;
}

String OTAWebServer::getProgressJSON() {
    String json = "{";
    json += "\"status\":\"" + String((int)OTACore::getStatus()) + "\",";
    json += "\"progress\":" + String(OTACore::getProgress()) + ",";
    json += "\"active\":" + String(OTACore::isActive() ? "true" : "false");
    json += "}";
    return json;
}
