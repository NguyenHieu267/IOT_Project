# IoT Project Summary Report

## 1. Project Overview

This project is an **Intelligent Environmental Monitoring and Control System** built on the ESP32 microcontroller (YOLO UNO board). The system integrates sensor monitoring, machine learning-based anomaly detection, automated actuator control, and cloud connectivity to create a comprehensive IoT solution for environmental management.

## 2. Hardware Components

### 2.1 Microcontroller
- **ESP32 (YOLO UNO Board)**: Main processing unit with Wi-Fi and Bluetooth capabilities
- **Framework**: Arduino framework on PlatformIO
- **File System**: LittleFS for web interface storage

### 2.2 Sensors
- **DHT20 Sensor**: Temperature and humidity monitoring via I2C bus (pins 11, 12)

### 2.3 Actuators
- **DC Motor (Fan)**: For temperature control/ventilation
- **Relay Module**: Controls water pump for humidity management
- **4 RGB LEDs (NeoPixel)**: Visual status indicators
- **Standard LED**: Status indicator (GPIO 13)
- **LCD Display (I2C)**: 16x2 character display for real-time data visualization

### 2.4 Communication
- **Wi-Fi**: For network connectivity and web server
- **MQTT**: For cloud connectivity to Core IoT platform

## 3. Software Architecture

### 3.1 Operating System
- **FreeRTOS**: Real-time operating system enabling multi-tasking
- **Task-based Architecture**: Modular design with independent tasks for each functionality

### 3.2 Core Tasks

1. **Temperature & Humidity Monitoring Task** (`temp_humi_monitor`)
   - Reads data from DHT20 sensor every 5 seconds
   - Classifies readings into three states: NORMAL, WARNING, CRITICAL
   - Thresholds:
     - Temperature: Warning (20-32°C), Critical (<15°C or >35°C)
     - Humidity: Warning (30-70%), Critical (<20% or >80%)
   - Publishes readings to sensor bus for other tasks

2. **LCD Display Task** (`lcd_display_task`)
   - Displays sensor readings on 16x2 LCD
   - Shows different display modes based on sensor state:
     - Normal: Standard temperature and humidity display
     - Warning: WARN prefix with values
     - Critical: CRIT prefix with max/min values
   - Updates every 2 seconds

3. **TinyML Anomaly Detection Task** (`tiny_ml_task`)
   - Implements TensorFlow Lite for edge AI inference
   - Uses pre-trained model (`dht_anomaly_model.h`) for anomaly detection
   - Input: Temperature and humidity values
   - Output: Binary classification (0 = Heat stress, 1 = Flood/Rain)
   - **Control Logic**:
     - **Mode 0 (result ≤ 0.5)**: Heat stress detected
       - Activates DC motor (fan) continuously
       - Deactivates relay (water pump)
       - LED pattern: Yellow → Red cycling
     - **Mode 1 (result > 0.5)**: Flood/Rain detected
       - Activates relay (water pump) continuously
       - Deactivates DC motor
       - LED pattern: Green → Orange blinking

4. **Core IoT Task** (`coreiot_task`)
   - MQTT client connection to Core IoT platform
   - Publishes telemetry data (temperature, humidity)
   - Sends device attributes (MAC address, local IP)
   - Subscribes to RPC callbacks for remote control
   - Handles shared attributes for device configuration
   - Reconnection logic for network resilience

5. **Web Server Task** (`task_webserver`)
   - AsyncWebServer on port 80
   - WebSocket support for real-time data streaming
   - Configuration interface (`test.html`):
     - Wi-Fi SSID and password configuration
     - Core IoT token, server, and port configuration
     - Elegant OTA (Over-The-Air) update support
   - Serves web interface from LittleFS

6. **Wi-Fi Management Task** (`task_wifi`)
   - Handles Wi-Fi connection and reconnection
   - Access Point mode fallback ("ESP32 LOCAL" with password "12345678")
   - Network status monitoring

7. **LED Control Tasks**
   - `led_blinky`: Standard LED blinking patterns
   - `neo_blinky`: NeoPixel RGB LED control with various patterns

### 3.3 Data Communication Architecture

**Sensor Bus System**:
- Queue-based communication between tasks
- Mutex-protected publishing to prevent race conditions
- Semaphore-based state signaling (NORMAL, WARNING, CRITICAL)
- Thread-safe data sharing across FreeRTOS tasks

## 4. Machine Learning Implementation

### 4.1 Model Details
- **Framework**: TensorFlow Lite for Microcontrollers
- **Model Type**: Binary classification for anomaly detection
- **Input Features**: 
  - Temperature (float)
  - Humidity (float)
- **Output**: Single float value (0.0-1.0)
  - ≤ 0.5: Heat stress condition
  - > 0.5: Flood/Rain condition

### 4.2 Training Labels
- **Label 0**: Heat stress (Temperature ≥ 30°C, Humidity > 70%)
- **Label 1**: Flood/Rain (Temperature ≤ 27°C, Humidity ≥ 80%)

### 4.3 Inference
- Runs every 5 seconds
- Uses sensor bus to get latest readings
- Tensor arena size: 8KB
- Edge inference (no cloud dependency)

## 5. Cloud Integration

### 5.1 Core IoT Platform
- **Protocol**: MQTT (Message Queuing Telemetry Transport)
- **Features**:
  - Telemetry data transmission
  - Attribute updates
  - RPC (Remote Procedure Call) support
  - Shared attributes for device configuration
  - Device provisioning and management

### 5.2 Data Flow
1. Sensor readings collected locally
2. Data processed by TinyML model
3. Telemetry sent to Core IoT platform
4. Remote commands received via RPC
5. Device attributes synchronized

## 6. User Interface

### 6.1 Web Interface
- **Access**: Via Wi-Fi connection to ESP32
- **Features**:
  - Modern, responsive design with glassmorphism effects
  - Configuration form for:
    - Wi-Fi credentials
    - Core IoT connection parameters (token, server, port)
  - Real-time status feedback
  - WebSocket support for live data updates

### 6.2 LCD Display
- Real-time sensor data visualization
- State-based display modes
- 2-second refresh rate

### 6.3 Visual Indicators
- RGB LEDs: Color-coded status based on ML predictions
- Status LED: Critical condition indicator

## 7. Key Features

1. **Real-time Environmental Monitoring**: Continuous temperature and humidity tracking
2. **Intelligent Anomaly Detection**: AI-powered classification of environmental conditions
3. **Automated Response**: Actuator control based on ML predictions
4. **Multi-state Alert System**: Normal, Warning, and Critical states with visual feedback
5. **Cloud Connectivity**: MQTT-based integration with Core IoT platform
6. **Remote Configuration**: Web-based setup interface
7. **OTA Updates**: Over-the-air firmware updates via ElegantOTA
8. **Thread-safe Architecture**: Mutex and semaphore-based task synchronization
9. **Network Resilience**: Automatic Wi-Fi reconnection and fallback AP mode
10. **Modular Design**: Task-based architecture for maintainability

## 8. Technical Specifications

### 8.1 Development Environment
- **Platform**: PlatformIO
- **Board**: ESP32 (YOLO UNO)
- **Framework**: Arduino
- **Serial Monitor**: 115200 baud

### 8.2 Libraries Used
- TensorFlowLite_ESP32 (v1.0.0)
- Adafruit NeoPixel (^1.15.1)
- DHT20 (custom)
- LCD I2C (custom)
- PubSubClient (MQTT)
- ESPAsyncWebServer
- ArduinoJson
- ThingsBoard SDK

### 8.3 Build Configuration
- USB CDC enabled for serial communication
- LittleFS filesystem for web assets
- Async WebServer support
- Custom build flags for Wi-Fi AP credentials

## 9. System Workflow

1. **Initialization**:
   - System boots and initializes sensor bus
   - Wi-Fi connection attempt
   - Web server starts (AP mode if Wi-Fi fails)
   - Core IoT connection attempt
   - TinyML model loading

2. **Operational Loop**:
   - Sensor reading (every 5 seconds)
   - Data classification (NORMAL/WARNING/CRITICAL)
   - ML inference (every 5 seconds)
   - Actuator control based on ML output
   - LCD display update (every 2 seconds)
   - Cloud data transmission (every 10 seconds)
   - WebSocket data broadcast (when clients connected)

3. **Response Mechanisms**:
   - Heat stress → Fan activation + Yellow/Red LED pattern
   - Flood/Rain → Water pump activation + Green/Orange LED pattern
   - Critical conditions → Status LED activation

## 10. Project Structure

```
IOT_Project/
├── src/                    # Source code files
│   ├── main.cpp           # Entry point and task creation
│   ├── temp_humi_monitor.cpp
│   ├── tinyml.cpp         # ML inference task
│   ├── task_core_iot.cpp  # Cloud connectivity
│   ├── task_webserver.cpp # Web interface
│   └── ...
├── include/               # Header files
├── lib/                   # External libraries
├── data/                  # Web interface files
├── test.html             # Configuration web page
└── platformio.ini        # Project configuration
```

## 11. Applications

This system can be applied to:
- **Smart Agriculture**: Automated greenhouse climate control
- **Home Automation**: Intelligent HVAC and humidity management
- **Industrial Monitoring**: Environmental condition tracking
- **Disaster Prevention**: Early warning systems for extreme weather
- **Research**: Environmental data collection and analysis

## 12. Future Enhancements

Potential improvements:
- Multi-device gateway architecture
- Additional sensor types (air quality, light, etc.)
- Enhanced ML model with more features
- Mobile application integration
- Historical data logging and analytics
- Energy consumption optimization
- Multi-zone monitoring support

## 13. Conclusion

This project demonstrates a complete IoT solution combining:
- **Edge Computing**: Local ML inference for real-time decision making
- **Cloud Integration**: Remote monitoring and control capabilities
- **Automated Control**: Intelligent actuator management
- **User Experience**: Intuitive web interface and visual feedback
- **Robustness**: Error handling, reconnection logic, and thread safety

The system successfully integrates hardware sensors, machine learning algorithms, cloud services, and user interfaces to create an intelligent environmental monitoring and control platform suitable for various real-world applications.

