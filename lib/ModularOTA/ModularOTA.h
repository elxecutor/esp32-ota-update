#pragma once

#include <Arduino.h>
#include "OTACore.h"
#include "NetworkManager.h"
#include "OTAWebServer.h"

/**
 * @brief Main orchestrator for modular OTA system
 * 
 * Coordinates all OTA components and provides a unified interface
 * for complete OTA functionality.
 */
class ModularOTA {
public:
    /**
     * @brief Complete OTA configuration
     */
    struct Config {
        // Network configuration
        String ssid;
        String password;
        bool autoReconnect;
        unsigned long reconnectInterval;
        
        // OTA Core configuration
        bool enablePersistence;
        
        // Web Server configuration
        int serverPort;
        String otaPath;
        String authUsername;
        String authPassword;
        bool enableCORS;
        bool enableProgress;
        size_t maxUploadSize;
        
        // Constructor with default values
        Config() : ssid(""), password(""), autoReconnect(true), reconnectInterval(30000),
                   enablePersistence(true), serverPort(3232), otaPath("/update"),
                   authUsername(""), authPassword(""), enableCORS(true), 
                   enableProgress(true), maxUploadSize(1048576) {}
    };

    /**
     * @brief System event types
     */
    enum class Event {
        NETWORK_CONNECTED,
        NETWORK_DISCONNECTED,
        OTA_STARTED,
        OTA_PROGRESS,
        OTA_COMPLETED,
        OTA_FAILED,
        SERVER_STARTED,
        SERVER_STOPPED
    };

    /**
     * @brief System event callback function type
     */
    typedef std::function<void(Event event, const String& message, int value)> CallbackFunction;

    /**
     * @brief Initialize complete modular OTA system
     * @param config System configuration
     * @return true if initialization successful
     */
    static bool begin(const Config& config);

    /**
     * @brief Set system event callback
     * @param callback Function to call on system events
     */
    static void setCallback(CallbackFunction callback);

    /**
     * @brief Handle all system tasks (call from loop)
     */
    static void handle();

    /**
     * @brief Stop OTA system
     */
    static void stop();

    /**
     * @brief Check if system is fully operational
     * @return true if all components are running
     */
    static bool isReady();

    /**
     * @brief Get system configuration
     * @return Current configuration
     */
    static Config getConfig();

    /**
     * @brief Update system configuration
     * @param config New configuration
     * @return true if update successful
     */
    static bool updateConfig(const Config& config);

    /**
     * @brief Get system status information
     * @param networkStatus Network connection status
     * @param otaStatus OTA operation status
     * @param serverStatus Web server status
     * @return true if status retrieved successfully
     */
    static bool getSystemStatus(NetworkManager::Status& networkStatus, 
                               OTACore::Status& otaStatus, 
                               bool& serverStatus);

    /**
     * @brief Get complete system information as JSON
     * @return JSON string with system information
     */
    static String getSystemInfoJSON();

    /**
     * @brief Enable/disable system components
     * @param enableNetwork Enable network manager
     * @param enableOTA Enable OTA core
     * @param enableServer Enable web server
     * @return true if components updated successfully
     */
    static bool setComponentsEnabled(bool enableNetwork, bool enableOTA, bool enableServer);

    /**
     * @brief Restart system after OTA update
     */
    static void restart();

    /**
     * @brief Get OTA URL for external use
     * @return Complete OTA URL
     */
    static String getOTAUrl();

    /**
     * @brief Add custom web server endpoint
     * @param path Endpoint path
     * @param handler Request handler function
     * @return true if endpoint added successfully
     */
    static bool addCustomEndpoint(const String& path, std::function<void()> handler);

    /**
     * @brief Get memory usage information
     * @param freeHeap Free heap memory
     * @param totalHeap Total heap memory
     * @param minFreeHeap Minimum free heap since boot
     * @return true if memory info retrieved
     */
    static bool getMemoryInfo(size_t& freeHeap, size_t& totalHeap, size_t& minFreeHeap);

private:
    static Config _config;
    static CallbackFunction _callback;
    static bool _initialized;
    static bool _networkEnabled;
    static bool _otaEnabled;
    static bool _serverEnabled;

    static void onNetworkEvent(NetworkManager::Status status, const String& message);
    static void onOTAEvent(OTACore::Status status, int progress, const String& message);
    static void onServerEvent(OTAWebServer::Event event, const String& message, int value);
    static void sendEvent(Event event, const String& message = "", int value = 0);
    static bool initializeComponents();
    static void logSystemInfo();
};
