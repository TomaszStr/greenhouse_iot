## 🌿 Embedded System — ESP32 Microcontroller

This is the embedded software component of the **Greenhouse IoT System**, responsible for real-time monitoring, control logic, and secure communication with the backend. The system runs on an **ESP32** microcontroller and interacts with environmental sensors, simulates actuator control, and supports secure provisioning and configuration via BLE and MQTT.

---

### 🔍 Overview

The ESP32 device collects environmental data and performs actions based on user-defined thresholds. It uses **Wi-Fi** for internet connectivity and communicates with the backend system through an **MQTT (Mosquitto) broker**. Additionally, it provides feedback to the user and supports over-the-air configuration during a secure multi-step pairing process.

---

### 🌡️ Sensor Suite

The device continuously reads data from multiple connected sensors:

- 🌱 **Soil Moisture**
- 💧 **Air Humidity**
- 🌡️ **Air Temperature**
- 🌬️ **Air Pressure**
- ☀️ **Light Intensity**

These values are sent to the backend server via MQTT and evaluated against user-configured thresholds.

---

### 🚨 Threshold Logic & Actuation

The system supports **two alert thresholds** per sensor:

- **Action Threshold**: Triggers a physical indicator (LED), simulating activation of devices like a **heater** or **water pump**.
- **Alert Threshold**: Sends an alert to the user via the backend API, allowing for remote notification and visibility.

Threshold values are fully configurable by the device owner via the mobile app.

---

### 🔐 Secure Device Pairing Workflow

A robust pairing process ensures secure ownership transfer and prevents misuse by previous owners. The steps are:

1. **Pairing Mode Activation**: The device enters pairing mode manually or on first boot.
2. **BLE Provisioning**: The mobile app sends **Wi-Fi credentials** and the **user's ID** over **Bluetooth Low Energy**.
3. **Wi-Fi Connection**: The ESP32 connects to the local network using provided credentials.
4. **Auth Exchange**:
    - The device sends a **unique hardcoded authentication code** to the backend.
    - The backend validates the device and links it to the new user.
    - The backend generates a new **MQTT password**, stores its hash in the database, and returns the password to the device.
5. **MQTT Connection**: The device connects to the **Mosquitto broker** using the updated credentials and begins regular operation.
6. **Status Feedback**: Throughout the process, the device sends real-time BLE feedback to the user, indicating pairing progress, success, or errors.

This flow prevents unauthorized reuse and ensures only the current owner has control of the device.

---

### 🛠️ Technology Stack

- **Platform**: ESP32 microcontroller
- **Programming Language**: C
- **Framework**: [Espressif IoT Development Framework (ESP-IDF)](https://docs.espressif.com/projects/esp-idf/)
- **Connectivity**:
    - **Wi-Fi**: MQTT communication via [Mosquitto Broker](https://mosquitto.org/)
    - **BLE**: Provisioning and pairing communication
- **Protocols**:
    - MQTT (publish/subscribe messaging)
    - HTTP (via backend API, indirect through server)
    - BLE (Bluetooth Low Energy)

---

### 🚧 Future Improvements

- Implement OTA (Over-the-Air) firmware updates.
- Add support for additional sensor types.
- Improve sensor calibration routines.
- Integrate local fallback logic for offline scenarios.

---

### 📁 Repository Structure (Embedded)

```
/embedded/
├── main/                   # Application entry point and logic
│   ├── sensors.c           # Sensor initialization and readings
│   ├── mqtt_client.c       # MQTT connection and messaging
│   ├── ble_provisioning.c # BLE setup and pairing flow
│   └── control_logic.c     # Threshold handling and device actions
├── include/                # Header files
├── CMakeLists.txt          # Build configuration
└── sdkconfig               # ESP-IDF project configuration
```

---

### ⚙️ Build & Flash Instructions

To build and flash the firmware:

```bash
# Set up ESP-IDF environment

# Navigate to embedded project directory
cd embedded/

# Configure project
idf.py menuconfig

# Build the firmware
idf.py build

# Flash to connected ESP32
idf.py -p /dev/ttyUSB0 flash monitor
```

> 💡 Adjust `/dev/ttyUSB0` to match your system's serial port.

---