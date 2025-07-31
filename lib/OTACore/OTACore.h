#pragma once

#include <Arduino.h>
#include <Update.h>
#include <WiFi.h>

/**
 * @brief Core OTA functionality that persists across firmware updates
 * 
 * This class handles the low-level OTA operations and ensures that OTA capability
 * remains available even after firmware updates. It uses RTC memory and specific
 * memory layout to maintain persistence.
 */
class OTACore {
public:
    /**
     * @brief OTA update status
     */
    enum class Status {
        IDLE,
        RECEIVING,
        COMPLETE,
        ERROR,
        REBOOTING
    };

    /**
     * @brief OTA update callback function type
     */
    typedef std::function<void(Status status, int progress, const String& message)> CallbackFunction;

    /**
     * @brief Initialize OTA core functionality
     * @param enablePersistence Enable RTC memory persistence for OTA logic
     * @return true if initialization successful
     */
    static bool begin(bool enablePersistence = true);

    /**
     * @brief Set callback function for OTA events
     * @param callback Function to call on OTA events
     */
    static void setCallback(CallbackFunction callback);

    /**
     * @brief Start OTA update process
     * @param size Expected firmware size
     * @param md5 Expected MD5 hash (optional)
     * @return true if OTA start successful
     */
    static bool startUpdate(size_t size, const String& md5 = "");

    /**
     * @brief Write data chunk to OTA
     * @param data Data buffer
     * @param len Data length
     * @return Number of bytes written, -1 on error
     */
    static int writeData(uint8_t* data, size_t len);

    /**
     * @brief Finish OTA update
     * @return true if OTA finish successful
     */
    static bool finishUpdate();

    /**
     * @brief Abort current OTA update
     */
    static void abortUpdate();

    /**
     * @brief Get current OTA status
     * @return Current status
     */
    static Status getStatus();

    /**
     * @brief Get OTA progress percentage (0-100)
     * @return Progress percentage
     */
    static int getProgress();

    /**
     * @brief Get last error message
     * @return Error message string
     */
    static String getLastError();

    /**
     * @brief Check if OTA is currently active
     * @return true if OTA is in progress
     */
    static bool isActive();

    /**
     * @brief Process OTA tasks (call from loop)
     */
    static void handle();

    /**
     * @brief Enable/disable OTA persistence across reboots
     * @param enable true to enable persistence
     */
    static void setPersistence(bool enable);

    /**
     * @brief Check if OTA core is persistent
     * @return true if persistence is enabled
     */
    static bool isPersistent();

    /**
     * @brief Get available OTA partition size
     * @return Available size in bytes
     */
    static size_t getAvailableSize();

    /**
     * @brief Restart ESP32 after OTA completion
     */
    static void restart();

    /**
     * @brief RTC data structure for persistence
     */
    struct RTCData {
        uint32_t magic;
        bool otaEnabled;
        Status status;
        int progress;
        uint32_t crc;
    };

private:

    static Status _status;
    static int _progress;
    static String _lastError;
    static CallbackFunction _callback;
    static bool _persistent;
    static RTCData _rtcData;
    static const uint32_t RTC_MAGIC = 0xDEADBEEF;

    static void saveToRTC();
    static bool loadFromRTC();
    static void updateCallback(Status status, int progress, const String& message);
    static uint32_t calculateCRC();
};
