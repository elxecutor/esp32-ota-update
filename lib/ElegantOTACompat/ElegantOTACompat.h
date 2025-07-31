#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include "OTACore.h"
#include "NetworkManager.h"
#include "OTAWebServer.h"

/**
 * @brief Backward compatibility layer for ElegantOTA
 * 
 * Provides a drop-in replacement for ElegantOTA library while using
 * the new modular architecture underneath.
 */
class ElegantOTACompat {
public:
    /**
     * @brief Initialize ElegantOTA compatibility layer
     * @param server WebServer instance (optional, will create internal if not provided)
     * @param path OTA endpoint path
     * @param username HTTP auth username (optional)
     * @param password HTTP auth password (optional)
     * @return true if initialization successful
     */
    static bool begin(WebServer* server = nullptr, const String& path = "/update", 
                      const String& username = "", const String& password = "");

    /**
     * @brief Handle OTA requests (call from loop)
     */
    static void loop();

    /**
     * @brief Stop OTA service
     */
    static void end();

    /**
     * @brief Set authentication credentials
     * @param username HTTP auth username
     * @param password HTTP auth password
     */
    static void setAuth(const String& username, const String& password);

    /**
     * @brief Remove authentication
     */
    static void removeAuth();

    /**
     * @brief Check if OTA is running
     * @return true if OTA service is active
     */
    static bool isRunning();

    /**
     * @brief Get OTA progress (0-100)
     * @return Progress percentage
     */
    static int getProgress();

    /**
     * @brief Check if OTA update is in progress
     * @return true if update is active
     */
    static bool isUpdating();

    /**
     * @brief Set custom OTA callback
     * @param callback Function to call on OTA events
     */
    static void onStart(std::function<void()> callback);
    static void onEnd(std::function<void()> callback);
    static void onProgress(std::function<void(unsigned int progress, unsigned int total)> callback);
    static void onError(std::function<void(String error)> callback);

    /**
     * @brief Get OTA URL
     * @return Full OTA URL
     */
    static String getOTAUrl();

    /**
     * @brief Restart ESP32
     */
    static void restart();

private:
    static WebServer* _externalServer;
    static bool _useExternalServer;
    static bool _initialized;
    static String _path;
    static std::function<void()> _onStartCallback;
    static std::function<void()> _onEndCallback;
    static std::function<void(unsigned int, unsigned int)> _onProgressCallback;
    static std::function<void(String)> _onErrorCallback;

    static void onOTAEvent(OTACore::Status status, int progress, const String& message);
    static void onServerEvent(OTAWebServer::Event event, const String& message, int value);
    static void setupExternalServerRoutes();
};
