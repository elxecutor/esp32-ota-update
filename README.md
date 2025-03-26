# ESP32 ElegantOTA Updater

This project demonstrates an over-the-air (OTA) firmware updater for the ESP32 using the [ElegantOTA](https://github.com/ayushsharma82/ElegantOTA) library. The firmware connects to a Wi‑Fi network, sets up an HTTP web server, and enables firmware updates via OTA—without needing a physical USB connection.

## Features

- **Wi‑Fi Connectivity:** Connects to a specified SSID and obtains an IP address.
- **Web Server:** Runs an HTTP server on port 80 with a basic endpoint (`/`) for testing.
- **ElegantOTA Integration:** Leverages ElegantOTA for secure and easy OTA updates.
- **Serial Debugging:** Prints connection status and IP address details to the serial monitor.
- **PlatformIO & Arduino Framework:** Configured for use with PlatformIO and the Arduino framework for rapid prototyping.

## Requirements

- An ESP32 board (e.g., Feather ESP32, ESP32 DevKit, etc.)
- [PlatformIO](https://platformio.org/) installed on your development machine **with the ESP32 board package**.
- A Wi‑Fi network (ensure the credentials in the code match your network).

> **Note:** The board package is installed on your development machine via PlatformIO—it provides all necessary libraries, headers, and toolchain support to compile and flash firmware for the ESP32. You do not have to install the board package explicitly on the physical ESP32.

## Getting Started

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/noiz-x/ota-update
   cd ota-update
   ```

2. **Configure Wi‑Fi Credentials:**

   Open `src/main.cpp` and update the following lines with your network's SSID and password:

   ```cpp
   const char *ssid = "YourSSID";
   const char *password = "YourPassword";
   ```

3. **Build and Upload:**

   Use PlatformIO to build and flash the firmware onto your ESP32:

   ```bash
   pio run --target upload
   ```

4. **Monitor Serial Output:**

   Open the serial monitor to view connection status and IP address details:

   ```bash
   pio device monitor --monitor_speed 115200
   ```

5. **OTA Update:**

   Once connected, the ElegantOTA interface will be available via the web server running on the ESP32’s IP address. Open a browser, enter the IP address printed on the serial monitor, and follow the prompts to perform OTA updates.

## OTA Safety and Fallback

One of the key benefits of using OTA updates with ElegantOTA is safety:
- **Application-Only Updates:**  
  ElegantOTA updates only the application firmware, leaving the bootloader and critical system partitions untouched. This design ensures that even if a new firmware image is faulty, the core system remains intact.
  
- **Preventing Bricking:**  
  Under normal operation, this process prevents your ESP32 from becoming unusable ("bricked"). However, to further protect your board:
  - **Firmware Validation:**  
    Always implement integrity checks (such as SHA256 hashes or digital signatures) to validate the new firmware before committing to it.
  
By following these best practices, the OTA update process is designed to be safe and largely automated, reducing the risk of rendering your board useless.

## PlatformIO Configuration

Below is an example `platformio.ini` configuration for the ESP32 Feather board:

```ini
[env:featheresp32]
platform = espressif32
board = featheresp32
framework = arduino
lib_deps = 
    ayushsharma82/ElegantOTA@^3.1.7
lib_compat_mode = strict
```

> **Reminder:** This configuration relies on the ESP32 board package installed on your development machine (via PlatformIO). This package provides all the necessary files to compile and flash firmware for your ESP32 and is not something you install directly on your board.

## Troubleshooting

- **Wi‑Fi Connection Issues:**  
  Verify that the SSID and password are correct. If you're using an open network, try using `WiFi.begin(ssid);` instead of providing an empty password. Also, consider removing any forced channel parameters from `WiFi.begin()`.

- **Serial Monitor Issues:**  
  If you're running the project in a simulation environment like Wokwi, use the built‑in serial console provided by the simulator instead of PlatformIO's external monitor.

- **OTA Not Initiating:**  
  Confirm that the web server is running (check for the "HTTP server started" message in the serial output). Then access the ElegantOTA interface by entering the ESP32's IP address in your browser.

## License

This project is licensed under the [MIT License](LICENSE).