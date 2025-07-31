#include "ElegantOTACompat.h"

// Static member definitions
WebServer* ElegantOTACompat::_externalServer = nullptr;
bool ElegantOTACompat::_useExternalServer = false;
bool ElegantOTACompat::_initialized = false;
String ElegantOTACompat::_path = "/update";
std::function<void()> ElegantOTACompat::_onStartCallback = nullptr;
std::function<void()> ElegantOTACompat::_onEndCallback = nullptr;
std::function<void(unsigned int, unsigned int)> ElegantOTACompat::_onProgressCallback = nullptr;
std::function<void(String)> ElegantOTACompat::_onErrorCallback = nullptr;

bool ElegantOTACompat::begin(WebServer* server, const String& path, 
                            const String& username, const String& password) {
    if (_initialized) {
        Serial.println("[ElegantOTACompat] Already initialized");
        return true;
    }

    _path = path;
    _externalServer = server;
    _useExternalServer = (server != nullptr);

    // Initialize OTA Core with persistence
    if (!OTACore::begin(true)) {
        Serial.println("[ElegantOTACompat] Failed to initialize OTA Core");
        return false;
    }

    // Set OTA Core callback
    OTACore::setCallback(onOTAEvent);

    if (_useExternalServer) {
        // Setup routes on external server
        setupExternalServerRoutes();
        Serial.println("[ElegantOTACompat] Using external web server for OTA");
    } else {
        // Use internal OTA web server
        OTAWebServer::Config config;
        config.path = path;
        config.username = username;
        config.password = password;
        config.port = 80; // Use standard HTTP port for compatibility

        if (!OTAWebServer::begin(config)) {
            Serial.println("[ElegantOTACompat] Failed to start OTA web server");
            return false;
        }

        OTAWebServer::setCallback(onServerEvent);
        Serial.println("[ElegantOTACompat] Using internal OTA web server");
    }

    _initialized = true;
    Serial.println("[ElegantOTACompat] ElegantOTA compatibility layer initialized");
    Serial.println("[ElegantOTACompat] OTA endpoint: " + path);
    
    return true;
}

void ElegantOTACompat::loop() {
    if (!_initialized) return;

    // Handle OTA core tasks
    OTACore::handle();

    // Handle web server if using internal server
    if (!_useExternalServer) {
        OTAWebServer::handle();
    }
}

void ElegantOTACompat::end() {
    if (!_initialized) return;

    if (!_useExternalServer) {
        OTAWebServer::stop();
    }

    _initialized = false;
    Serial.println("[ElegantOTACompat] ElegantOTA compatibility layer stopped");
}

void ElegantOTACompat::setAuth(const String& username, const String& password) {
    if (!_useExternalServer) {
        OTAWebServer::setAuthentication(username, password);
    }
    Serial.println("[ElegantOTACompat] Authentication set");
}

void ElegantOTACompat::removeAuth() {
    if (!_useExternalServer) {
        OTAWebServer::removeAuthentication();
    }
    Serial.println("[ElegantOTACompat] Authentication removed");
}

bool ElegantOTACompat::isRunning() {
    if (!_initialized) return false;
    
    if (_useExternalServer) {
        return true; // External server managed externally
    } else {
        return OTAWebServer::isRunning();
    }
}

int ElegantOTACompat::getProgress() {
    return OTACore::getProgress();
}

bool ElegantOTACompat::isUpdating() {
    return OTACore::isActive();
}

void ElegantOTACompat::onStart(std::function<void()> callback) {
    _onStartCallback = callback;
}

void ElegantOTACompat::onEnd(std::function<void()> callback) {
    _onEndCallback = callback;
}

void ElegantOTACompat::onProgress(std::function<void(unsigned int progress, unsigned int total)> callback) {
    _onProgressCallback = callback;
}

void ElegantOTACompat::onError(std::function<void(String error)> callback) {
    _onErrorCallback = callback;
}

String ElegantOTACompat::getOTAUrl() {
    if (_useExternalServer) {
        // For external server, we need network info
        if (NetworkManager::isConnected()) {
            return "http://" + NetworkManager::getIPAddress() + _path;
        }
        return "";
    } else {
        return OTAWebServer::getOTAUrl();
    }
}

void ElegantOTACompat::restart() {
    OTACore::restart();
}

void ElegantOTACompat::onOTAEvent(OTACore::Status status, int progress, const String& message) {
    switch (status) {
        case OTACore::Status::RECEIVING:
            if (progress == 0 && _onStartCallback) {
                _onStartCallback();
            }
            if (_onProgressCallback) {
                // Calculate total from available size for compatibility
                size_t total = OTACore::getAvailableSize();
                unsigned int received = (progress * total) / 100;
                _onProgressCallback(received, total);
            }
            break;
            
        case OTACore::Status::COMPLETE:
            if (_onEndCallback) {
                _onEndCallback();
            }
            break;
            
        case OTACore::Status::ERROR:
            if (_onErrorCallback) {
                _onErrorCallback(message);
            }
            break;
            
        default:
            break;
    }
}

void ElegantOTACompat::onServerEvent(OTAWebServer::Event event, const String& message, int value) {
    switch (event) {
        case OTAWebServer::Event::UPLOAD_START:
            if (_onStartCallback) {
                _onStartCallback();
            }
            break;
            
        case OTAWebServer::Event::UPLOAD_COMPLETE:
            if (_onEndCallback) {
                _onEndCallback();
            }
            break;
            
        case OTAWebServer::Event::UPLOAD_ERROR:
            if (_onErrorCallback) {
                _onErrorCallback(message);
            }
            break;
            
        default:
            break;
    }
}

void ElegantOTACompat::setupExternalServerRoutes() {
    if (!_externalServer) return;

    // Add OTA upload page route
    _externalServer->on(_path, HTTP_GET, []() {
        // Serve the same HTML page as OTAWebServer
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
        // Same JavaScript as in OTAWebServer
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
            
            xhr.open('POST', '');
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
        
        _externalServer->send(200, "text/html", html);
    });

    // Add POST handler for file upload
    _externalServer->on(_path, HTTP_POST, 
        []() {
            _externalServer->send(200, "text/plain", "Upload completed");
        },
        []() {
            HTTPUpload& upload = _externalServer->upload();
            
            if (upload.status == UPLOAD_FILE_START) {
                Serial.println("[ElegantOTACompat] External server upload started: " + upload.filename);
                
                if (!OTACore::startUpdate(upload.totalSize)) {
                    return;
                }
            } else if (upload.status == UPLOAD_FILE_WRITE) {
                if (OTACore::writeData(upload.buf, upload.currentSize) != upload.currentSize) {
                    return;
                }
            } else if (upload.status == UPLOAD_FILE_END) {
                if (OTACore::finishUpdate()) {
                    Serial.println("[ElegantOTACompat] External server upload completed");
                } else {
                    Serial.println("[ElegantOTACompat] External server upload failed");
                }
            }
        }
    );
}
