#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <functional>
#include "OTACore.h"

/**
 * @brief Web server interface for OTA updates
 * 
 * Provides HTTP endpoints for OTA operations while being decoupled
 * from the main application web server.
 */
class OTAWebServer {
public:
    /**
     * @brief Server configuration structure
     */
    struct Config {
        int port;                          // OTA server port
        String path;                       // OTA endpoint path
        String username;                   // HTTP auth username (optional)
        String password;                   // HTTP auth password (optional)
        bool enableCORS;                   // Enable CORS headers
        bool enableProgress;               // Enable progress endpoint
        size_t maxUploadSize;              // Max upload size (1MB default)
        
        // Constructor with default values
        Config() : port(3232), path("/update"), username(""), password(""), 
                   enableCORS(true), enableProgress(true), maxUploadSize(1048576) {}
    };

    /**
     * @brief Server event types
     */
    enum class Event {
        STARTED,
        STOPPED,
        UPLOAD_START,
        UPLOAD_PROGRESS,
        UPLOAD_COMPLETE,
        UPLOAD_ERROR,
        CLIENT_CONNECTED,
        CLIENT_DISCONNECTED
    };

    /**
     * @brief Server event callback function type
     */
    typedef std::function<void(Event event, const String& message, int value)> CallbackFunction;

    /**
     * @brief Initialize OTA web server
     * @param config Server configuration
     * @return true if initialization successful
     */
    static bool begin(const Config& config = Config());

    /**
     * @brief Stop OTA web server
     */
    static void stop();

    /**
     * @brief Set event callback
     * @param callback Function to call on server events
     */
    static void setCallback(CallbackFunction callback);

    /**
     * @brief Handle web server requests (call from loop)
     */
    static void handle();

    /**
     * @brief Check if server is running
     * @return true if server is active
     */
    static bool isRunning();

    /**
     * @brief Get server configuration
     * @return Current configuration
     */
    static Config getConfig();

    /**
     * @brief Update server configuration
     * @param config New configuration
     * @return true if update successful
     */
    static bool updateConfig(const Config& config);

    /**
     * @brief Get OTA URL
     * @return Full OTA URL
     */
    static String getOTAUrl();

    /**
     * @brief Get connected clients count
     * @return Number of connected clients
     */
    static int getClientCount();

    /**
     * @brief Add custom endpoint to OTA server
     * @param path Endpoint path
     * @param handler Request handler function
     */
    static void addCustomEndpoint(const String& path, std::function<void()> handler);

    /**
     * @brief Enable/disable authentication
     * @param username HTTP auth username
     * @param password HTTP auth password
     */
    static void setAuthentication(const String& username, const String& password);

    /**
     * @brief Remove authentication
     */
    static void removeAuthentication();

private:
    static WebServer* _server;
    static Config _config;
    static CallbackFunction _callback;
    static bool _running;
    static int _clientCount;
    static unsigned long _uploadStartTime;
    static size_t _uploadSize;
    static size_t _uploadReceived;

    static void setupRoutes();
    static void handleUpdate();
    static void handleUpdatePost();
    static void handleProgress();
    static void handleStatus();
    static void handleReboot();
    static void handleNotFound();
    static void sendCORSHeaders();
    static bool authenticate();
    static void sendEvent(Event event, const String& message = "", int value = 0);
    static String getStatusJSON();
    static String getProgressJSON();
};
