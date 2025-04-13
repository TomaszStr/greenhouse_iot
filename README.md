# 🌿 Greenhouse IoT System

A modular and secure IoT platform designed to monitor and manage greenhouse environmental conditions in real time. The system combines **embedded IoT devices**, a **Java Spring Boot backend**, and an **MQTT broker with dynamic access control** to deliver scalable, real-time data tracking, alerting, and remote configuration.

---

## 🧩 Project Architecture

The system is composed of three main components:

1. **📦 Embedded System (ESP32)**  
   Real-time sensing and control unit running on an ESP32 microcontroller. Gathers environmental data (e.g., soil moisture, temperature) and communicates with the backend via MQTT.

2. **🌐 Backend Server (Java Spring Boot)**  
   Provides REST APIs for user/device management, sensor configuration, and secure MQTT communication. Handles alerting, pairing logic, data processing, and MQTT role/ACL provisioning.

3. **📡 MQTT Broker (Mosquitto with Dynamic Security Plugin)**  
   Lightweight message broker that facilitates communication between devices and backend. Manages per-device topics and enforces strict access control using dynamic security.

---

## 🔧 Key Features

- 📊 **Real-Time Monitoring** of greenhouse metrics (soil, air, light)
- 🔐 **Secure Pairing** and device provisioning via BLE + MQTT credentialing
- 🌱 **User-Configurable Thresholds** for temperature and soil moisture
- 🚨 **Dual Alert System** with physical (LED) actions and backend alerts
- 🛠️ **Remote Configuration** of sensors and device behavior
- 🧪 **Robust API & Admin Tools** for full system control
- 🧰 **Modular Architecture** for easy maintenance and expansion

---

## ⚙️ Technology Stack

| Layer        | Technologies Used                                      |
|--------------|--------------------------------------------------------|
| **Embedded** | ESP32, C, ESP-IDF, BLE, MQTT                           |
| **Backend**  | Java 21, Spring Boot 3.3.5, PostgreSQL, JWT, Paho MQTT |
| **Broker**   | Mosquitto, Dynamic Security Plugin, Docker             |

---

## 🔗 Component Readmes

| Component                                 | Description                                                                                         |
|-------------------------------------------|-----------------------------------------------------------------------------------------------------|
| [`/embedded`](./greenhouse_iot_ESP32)     | Firmware running on ESP32 to collect sensor data, handle BLE pairing, and publish to MQTT           |
| [`/backend`](./greenhouse_iot)            | Java backend with RESTful API, secure user/device management, MQTT integration, and admin interface |
| [`/broker`](./greenhouse_iot_mqtt_broker) | MQTT broker with secure dynamic role-based topic management (Dockerized)                            |

---

## 🚀 Getting Started

1. 🐳 Start the **MQTT Broker**  
   Docker container with dynamic security configuration.

2. 🧰 Run the **Backend Server**  
   Java Spring Boot app with PostgreSQL and MQTT integration.

3. 🌿 Flash and pair the **ESP32 Device**  
   Sensor readings are sent over MQTT after secure BLE pairing.

---

## 📦 Future Enhancements

- 📶 OTA firmware updates for ESP32
- 📲 Mobile app interface (BLE, WiFi config, alerts)
- 📈 Advanced analytics and data visualization
- 🌤 Offline fallback logic in embedded firmware

---

> For developer setup instructions, API reference, and build tools, see individual component READMEs.
