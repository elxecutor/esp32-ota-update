#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <functional>

/**
 * @brief Network management for OTA functionality
 * 
 * Handles WiFi connectivity, reconnection, and network status monitoring
 * for reliable OTA operations.
 */
class NetworkManager {
public:
    /**
     * @brief WiFi connection status
     */
    enum class Status {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        FAILED,
        RECONNECTING
    };

    /**
     * @brief Network event callback function type
     */
    typedef std::function<void(Status status, const String& message)> CallbackFunction;

    /**
     * @brief Initialize network manager
     * @param ssid WiFi SSID
     * @param password WiFi password
     * @param autoReconnect Enable automatic reconnection
     * @return true if initialization successful
     */
    static bool begin(const char* ssid, const char* password, bool autoReconnect = true);

    /**
     * @brief Set network event callback
     * @param callback Function to call on network events
     */
    static void setCallback(CallbackFunction callback);

    /**
     * @brief Connect to WiFi network
     * @param timeout Connection timeout in milliseconds
     * @return true if connection successful
     */
    static bool connect(unsigned long timeout = 10000);

    /**
     * @brief Disconnect from WiFi network
     */
    static void disconnect();

    /**
     * @brief Check if connected to WiFi
     * @return true if connected
     */
    static bool isConnected();

    /**
     * @brief Get current WiFi status
     * @return Current network status
     */
    static Status getStatus();

    /**
     * @brief Get local IP address
     * @return IP address string
     */
    static String getIPAddress();

    /**
     * @brief Get WiFi signal strength
     * @return RSSI value
     */
    static int getRSSI();

    /**
     * @brief Get connected SSID
     * @return SSID string
     */
    static String getSSID();

    /**
     * @brief Handle network tasks (call from loop)
     */
    static void handle();

    /**
     * @brief Enable/disable auto-reconnection
     * @param enable true to enable auto-reconnection
     */
    static void setAutoReconnect(bool enable);

    /**
     * @brief Check if auto-reconnection is enabled
     * @return true if auto-reconnection is enabled
     */
    static bool isAutoReconnectEnabled();

    /**
     * @brief Set reconnection interval
     * @param interval Interval in milliseconds
     */
    static void setReconnectInterval(unsigned long interval);

    /**
     * @brief Get network configuration for OTA
     * @param ip Local IP address
     * @param gateway Gateway IP address
     * @param subnet Subnet mask
     * @return true if network info available
     */
    static bool getNetworkInfo(IPAddress& ip, IPAddress& gateway, IPAddress& subnet);

private:
    static Status _status;
    static String _ssid;
    static String _password;
    static bool _autoReconnect;
    static unsigned long _reconnectInterval;
    static unsigned long _lastReconnectAttempt;
    static CallbackFunction _callback;
    static int _reconnectAttempts;
    static const int MAX_RECONNECT_ATTEMPTS = 5;

    static void onWiFiEvent(WiFiEvent_t event);
    static void updateStatus(Status newStatus, const String& message = "");
    static void attemptReconnect();
};
