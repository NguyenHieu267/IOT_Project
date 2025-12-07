#  ESP32-S3 IoT Smart System with TinyML

A comprehensive IoT system built on ESP32-S3 featuring real-time sensor monitoring, TinyML anomaly detection, web-based control interface, and MQTT cloud connectivity.

##  Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Architecture](#software-architecture)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [System Components](#system-components)
- [Web Interface](#web-interface)
- [Configuration](#configuration)
- [API Reference](#api-reference)
- [Troubleshooting](#troubleshooting)

---

##  Features

### Core Capabilities
-  **Real-time Environmental Monitoring**: DHT20 temperature & humidity sensor with LCD display
-  **TinyML Anomaly Detection**: On-device AI inference using TensorFlow Lite Micro
-  **Web Control Interface**: Responsive web UI with WebSocket for real-time bidirectional communication
-  **Data Visualization**: JustGage gauges for temperature/humidity, auto-scrolling console logs
-  **Cloud Connectivity**: MQTT integration with CoreIOT platform
-  **OTA Updates**: Over-The-Air firmware updates via ElegantOTA
-  **Manual Override**: Web-based manual control for all actuators

### Hardware Control
- **LED Indicators**:
  - Onboard LED (GPIO 48): Temperature-based blinking patterns
  - NeoPixel (GPIO 45): Humidity level indicator
  - RGB Strip (GPIO 6): TinyML prediction visualization (4 LEDs)
  - Alert LED (GPIO 13): Warning/Critical state indicator
  
- **Actuators**:
  - DC Motor (GPIO 10): Fan control for cooling
  - Relay (GPIO 8): Water pump or emergency shutoff
  
- **Display**: 16x2 LCD I2C (Address 0x21) with 3-state display:
  - **NORMAL**: Shows current temp/humidity
  - **WARNING**: Flashing warning message + values
  - **CRITICAL**: Flashing critical alert + values

### Networking
- **WiFi Modes**: 
  - AP Mode (default): ESP32-YOUR NETWORK HERE!!! / 12345678
  - STA Mode: Connect to existing WiFi network
- **WebSocket**: Real-time data streaming and control
- **MQTT**: Cloud data publishing to CoreIOT platform
- **Configuration Portal**: Web-based WiFi & IoT credentials management

---

##  Hardware Requirements

### Main Components
| Component | Model | GPIO Pin | Interface | Purpose |
|-----------|-------|----------|-----------|---------|
| Microcontroller | ESP32-S3 | - | - | Main processor with WiFi |
| Temp/Humidity Sensor | DHT20 | SDA: 11, SCL: 12 | I2C | Environmental monitoring |
| LCD Display | 16x2 LCD I2C | SDA: 11, SCL: 12 | I2C (0x21) | Status display |
| Onboard LED | Built-in | 48 | Digital Out | Temperature indicator |
| NeoPixel LED | WS2812B | 45 | 1-Wire | Humidity indicator |
| RGB LED Strip | WS2812B x4 | 6 | 1-Wire | AI prediction display |
| Alert LED | Standard LED | 13 | Digital Out | Warning/Critical alerts |
| DC Motor | Generic | 10 | Digital Out | Cooling fan |
| Relay Module | 1-Channel | 8 | Digital Out | Water pump/shutoff |

---

##  Software Architecture

### FreeRTOS Task Structure

```

                      ESP32-S3 RTOS System                    

  Task Name            Stack  Priority  Function          

 led_blinky            2048      2      Onboard LED       
 neo_blinky            2048      2      NeoPixel control  
 temp_humi_monitor     8192      2      Sensor + LCD      
 main_server_task      8192      2      WebSocket sender  
 tiny_ml_task          8192      2      TensorFlow Lite   
 coreiot_task          4096      2      MQTT client       

```

### Synchronization Primitives

| Mutex/Semaphore | Purpose |
|-----------------|---------|
| xSensorDataMutex | Protects sharedSensorData (temp/humidity) |
| xSensorMinMaxMutex | Protects min/max value tracker |
| xSerialMutex | Thread-safe Serial.print() from multiple tasks |
| xBinarySemaphoreInternet | Signals WiFi connection ready |
| xDataReadySemaphore | Signals LCD display state change |

---

##  Project Structure

```
IOT_Proj/
 src/                          # Source code implementations
    main.cpp                  # Main entry point, task creation
    global.cpp                # Global variables initialization
    led_blinky.cpp            # Onboard LED temperature indicator
    neo_blinky.cpp            # NeoPixel humidity indicator
    temp_humi_monitor.cpp     # DHT20 sensor + LCD display
    tinyml.cpp                # TensorFlow Lite inference engine
    4_led_rgb.cpp             # RGB strip control (4 LEDs)
    DC_motor.cpp              # DC motor/fan control
    relay_control.cpp         # Relay module control
    coreiot.cpp               # CoreIOT MQTT client
    task_wifi.cpp             # WiFi STA/AP management
    task_webserver.cpp        # AsyncWebServer + WebSocket
    task_handler.cpp          # WebSocket message handler
    task_check_info.cpp       # LittleFS config file manager
    main_server_task.cpp      # Periodic sensor data WebSocket sender

 include/                      # Header files
    global.h                  # Global definitions, structs, mutexes
    project_includes.h        # Master include file
    dht_anomaly_model.h       # TinyML model data (binary array)
    ...                       # Individual module headers

 data/                         # Web interface files (uploaded to LittleFS)
    index.html                # Main web UI
    script.js                 # WebSocket client & UI logic
    styles.css                # Responsive styling

 lib/                          # External libraries
    ArduinoJson/              # JSON parsing/serialization
    ElegantOTA-master/        # OTA update framework
    ThingsBoard/              # IoT platform SDK
    PubSubClient/             # MQTT client
    DHT20/                    # DHT20 sensor driver
    LCD/                      # LiquidCrystal_I2C library

 boards/
    yolo_uno.json             # Custom board definition

 platformio.ini                # PlatformIO configuration
 README.md                     # This file
```

---

##  Getting Started

### Installation

1. **Clone Repository**
   ```bash
   git clone <repository-url>
   cd IOT_Proj
   ```

2. **Upload Filesystem** (web interface files)
   ```bash
   pio run -t uploadfs -e yolo_uno
   ```

3. **Compile and Upload Firmware**
   ```bash
   pio run -t upload -e yolo_uno
   ```

4. **Monitor Serial Output**
   ```bash
   pio device monitor -e yolo_uno
   ```

### First Boot

1. ESP32 starts in **AP Mode**:
   - SSID: ESP32-YOUR NETWORK HERE!!!
   - Password: 12345678

2. Connect to ESP32 WiFi network

3. Open browser to: http://192.168.4.1

4. Navigate to **Settings** tab:
   - Enter your WiFi SSID and password
   - (Optional) Enter CoreIOT credentials
   - Click **Save Configuration**
   - Device will restart and connect to your WiFi

---

##  System Components

### 1. Temperature/Humidity Monitoring

**File**: temp_humi_monitor.cpp

**Function**: Reads DHT20 sensor every 3 seconds, updates LCD display based on alert level

**Thresholds**:
- CRITICAL: Temp > 35C or < 15C | Humidity > 80% or < 20%
- WARNING: Temp > 30C or < 18C | Humidity > 70% or < 30%
- NORMAL: Within safe ranges

### 2. TinyML Anomaly Detection

**File**: tinyml.cpp

**Model Output**: 3-class Softmax probabilities
- **Label 0**: Hot/Vehicle  Turn ON fan
- **Label 1**: Rain/Flood  Turn ON pump
- **Label 2**: Normal  Turn OFF actuators

**RGB LED Visualization**:
- Label 0: Yellow  Red blinking
- Label 1: Green  Orange blinking
- Label 2: Blue static

### 3. LED Indicators

**Onboard LED**: Temperature-based patterns
- < 25C: 2 slow pulses
- 25-35C: 4 medium pulses
- > 35C: 10 fast pulses (alarm)

**NeoPixel**: Humidity-based colors
- < 30%: Yellow ON (dry)
- 30-50%: Yellow blinking
- 50-60%: Green ON (optimal)
- 60-70%: Red blinking
- > 70%: Red ON (humid)

---

##  Web Interface

**Access**: http://<device-ip>

**Tabs**:
1. **Dashboard**: Real-time temp/humidity gauges
2. **Device Control**: Toggle all GPIO devices
3. **Console**: Auto-scrolling logs (200 lines)
4. **System Info**: Hardware/firmware details
5. **Settings**: WiFi & CoreIOT config

**WebSocket Messages**:

Client  Server (control):
```json
{
  "page": "device",
  "value": {"gpio": 8, "status": "ON"}
}
```

Server  Client (data):
```json
{
  "temperature": 28.5,
  "humidity": 65.2
}
```

---

##  Configuration

### WiFi Configuration

Edit via web interface or modify /config.json in LittleFS

### Threshold Customization

Edit temp_humi_monitor.cpp:
```cpp
#define TEMP_CRITICAL_HIGH  35.0
#define TEMP_WARNING_HIGH   30.0
#define HUMI_CRITICAL_HIGH  80.0
```

---

##  API Reference

### Hardware Control
```cpp
void dc_motor_set(bool on);           // Control DC motor
void relay_set(bool on);              // Control relay
void set_all_leds(int r, int g, int b);  // Set RGB strip
```

### Thread-Safe Serial
```cpp
SERIAL_PRINT("Hello");               // Thread-safe print
SERIAL_PRINTLN("World");             // Thread-safe println
SERIAL_PRINTF("Temp: %.1f\n", temp); // Thread-safe printf
```

### WebSocket Logging
```cpp
WS_LOG("Status message");  // Send to Serial + web console
```

---

##  Troubleshooting

### LCD Shows Garbage Characters
- **Cause**: Relay EMI interference
- **Solution**: Add flyback diode (1N4007) across relay coil

### WebSocket Disconnects
- **Solution**: Increase task stack size to 8192+ bytes

### TinyML Not Working
- **Check**: Stack size  8192 for tiny_ml_task

### WiFi Fails
- Verify 2.4GHz network (not 5GHz)
- Reset via Settings  Reset WiFi

---

##  License

MIT License

---

**Version**: 1.0.0
**Status**: Production Ready 
**Platform**: ESP32-S3 with FreeRTOS + PlatformIO
