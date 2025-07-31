#include "OTACore.h"
#include <esp_system.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>

// Static member definitions
OTACore::Status OTACore::_status = Status::IDLE;
int OTACore::_progress = 0;
String OTACore::_lastError = "";
OTACore::CallbackFunction OTACore::_callback = nullptr;
bool OTACore::_persistent = false;
OTACore::RTCData OTACore::_rtcData = {0};

// RTC memory allocation for persistence
RTC_DATA_ATTR OTACore::RTCData rtc_ota_data = {0};

bool OTACore::begin(bool enablePersistence) {
    _persistent = enablePersistence;
    _status = Status::IDLE;
    _progress = 0;
    _lastError = "";

    if (_persistent) {
        if (loadFromRTC()) {
            Serial.println("[OTACore] Restored OTA state from RTC memory");
        } else {
            Serial.println("[OTACore] Initializing fresh OTA state");
            _rtcData.magic = RTC_MAGIC;
            _rtcData.otaEnabled = true;
            _rtcData.status = Status::IDLE;
            _rtcData.progress = 0;
            saveToRTC();
        }
    }

    // Initialize Update library
    Update.onProgress([](size_t progress, size_t total) {
        int percent = (progress * 100) / total;
        _progress = percent;
        updateCallback(Status::RECEIVING, percent, "Receiving update...");
    });

    Serial.println("[OTACore] OTA Core initialized successfully");
    return true;
}

void OTACore::setCallback(CallbackFunction callback) {
    _callback = callback;
}

bool OTACore::startUpdate(size_t size, const String& md5) {
    if (_status != Status::IDLE) {
        _lastError = "OTA already in progress";
        return false;
    }

    if (size == 0) {
        _lastError = "Invalid update size";
        return false;
    }

    // Check available space
    size_t availableSize = getAvailableSize();
    if (size > availableSize) {
        _lastError = "Update size exceeds available space";
        return false;
    }

    // Start update
    bool success = false;
    if (md5.length() > 0) {
        // ESP32 Update library doesn't support MD5 directly in begin()
        // We'll validate MD5 after the update is complete
        success = Update.begin(size, U_FLASH);
        if (success) {
            Update.setMD5(md5.c_str());
        }
    } else {
        success = Update.begin(size, U_FLASH);
    }

    if (!success) {
        _lastError = "Failed to start update: " + String(Update.errorString());
        _status = Status::ERROR;
        updateCallback(_status, 0, _lastError);
        return false;
    }

    _status = Status::RECEIVING;
    _progress = 0;
    _lastError = "";
    
    if (_persistent) {
        _rtcData.status = _status;
        _rtcData.progress = _progress;
        saveToRTC();
    }

    updateCallback(_status, _progress, "Starting OTA update...");
    Serial.println("[OTACore] OTA update started, size: " + String(size));
    return true;
}

int OTACore::writeData(uint8_t* data, size_t len) {
    if (_status != Status::RECEIVING) {
        _lastError = "OTA not in receiving state";
        return -1;
    }

    if (!data || len == 0) {
        _lastError = "Invalid data buffer";
        return -1;
    }

    size_t written = Update.write(data, len);
    
    if (written != len) {
        _lastError = "Write error: " + String(Update.errorString());
        _status = Status::ERROR;
        updateCallback(_status, _progress, _lastError);
        return -1;
    }

    if (_persistent) {
        _rtcData.progress = _progress;
        saveToRTC();
    }

    return written;
}

bool OTACore::finishUpdate() {
    if (_status != Status::RECEIVING) {
        _lastError = "OTA not in receiving state";
        return false;
    }

    bool success = Update.end(true);
    
    if (!success) {
        _lastError = "Failed to finish update: " + String(Update.errorString());
        _status = Status::ERROR;
        updateCallback(_status, _progress, _lastError);
        return false;
    }

    _status = Status::COMPLETE;
    _progress = 100;
    
    if (_persistent) {
        _rtcData.status = _status;
        _rtcData.progress = _progress;
        saveToRTC();
    }

    updateCallback(_status, _progress, "OTA update completed successfully");
    Serial.println("[OTACore] OTA update completed successfully");
    return true;
}

void OTACore::abortUpdate() {
    if (_status == Status::RECEIVING) {
        Update.abort();
        Serial.println("[OTACore] OTA update aborted");
    }
    
    _status = Status::IDLE;
    _progress = 0;
    _lastError = "Update aborted";
    
    if (_persistent) {
        _rtcData.status = _status;
        _rtcData.progress = _progress;
        saveToRTC();
    }
    
    updateCallback(_status, _progress, _lastError);
}

OTACore::Status OTACore::getStatus() {
    return _status;
}

int OTACore::getProgress() {
    return _progress;
}

String OTACore::getLastError() {
    return _lastError;
}

bool OTACore::isActive() {
    return _status == Status::RECEIVING;
}

void OTACore::handle() {
    // Handle any background OTA tasks
    if (_status == Status::COMPLETE) {
        // Auto-restart after successful update
        delay(1000);
        _status = Status::REBOOTING;
        updateCallback(_status, 100, "Rebooting...");
        restart();
    }
}

void OTACore::setPersistence(bool enable) {
    _persistent = enable;
    if (enable) {
        saveToRTC();
    }
}

bool OTACore::isPersistent() {
    return _persistent;
}

size_t OTACore::getAvailableSize() {
    const esp_partition_t* partition = esp_ota_get_next_update_partition(NULL);
    if (partition) {
        return partition->size;
    }
    return 0;
}

void OTACore::restart() {
    Serial.println("[OTACore] Restarting ESP32...");
    Serial.flush();
    ESP.restart();
}

void OTACore::saveToRTC() {
    if (!_persistent) return;
    
    _rtcData.crc = calculateCRC();
    rtc_ota_data = _rtcData;
}

bool OTACore::loadFromRTC() {
    if (!_persistent) return false;
    
    _rtcData = rtc_ota_data;
    
    // Validate magic number and CRC
    if (_rtcData.magic != RTC_MAGIC) {
        return false;
    }
    
    uint32_t storedCRC = _rtcData.crc;
    uint32_t calculatedCRC = calculateCRC();
    
    if (storedCRC != calculatedCRC) {
        return false;
    }
    
    // Restore state
    _status = _rtcData.status;
    _progress = _rtcData.progress;
    
    return true;
}

void OTACore::updateCallback(Status status, int progress, const String& message) {
    if (_callback) {
        _callback(status, progress, message);
    }
}

uint32_t OTACore::calculateCRC() {
    // Simple CRC calculation for RTC data validation
    uint32_t crc = 0;
    crc ^= _rtcData.magic;
    crc ^= (uint32_t)_rtcData.otaEnabled;
    crc ^= (uint32_t)_rtcData.status;
    crc ^= (uint32_t)_rtcData.progress;
    return crc;
}
