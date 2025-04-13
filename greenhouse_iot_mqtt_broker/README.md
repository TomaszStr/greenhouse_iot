## 🔐 MQTT Broker Integration

This module manages secure communication between devices and the backend server using the **Mosquitto MQTT broker** with the **Dynamic Security Plugin**.

On initialization, the service subscribes to:

- `devices/+/data` – Incoming sensor data from all devices
- `devices/+/alerts` – Alert messages from devices
- `$CONTROL/dynamic-security/v1` – Broker control channel for managing roles, ACLs, and clients dynamically

---

### 🔁 Topic Structure (Per Device)

Each device is assigned a unique ID (`sensorMqttName`) and has its own dedicated MQTT topic namespace:

- `devices/{sensorMqttName}/data` – Device publishes sensor measurements
- `devices/{sensorMqttName}/alerts` – Device publishes alert messages
- `devices/{sensorMqttName}/commands` – Backend sends control commands

---

### 🛡️ Access Control

- Every device gets a unique MQTT role with isolated publish/subscribe permissions.
- Access is managed dynamically via the control topic, ensuring secure pairing and operation.
- The backend handles topic registration, credential setup, and access rules during the device pairing process.

---

## 🐳 Running Mosquitto Broker with Docker

To run the MQTT broker locally with support for dynamic security, use the following setup.

### 📦 Build Docker Image

```bash
docker build -t mosquitto-broker-greenhouse-iot .
```

### 🚀 Run the Broker Container

```bash
docker run -d \
  --name mosquitto-broker-greenhouse-iot \
  -p 1883:1883 \
  -p 9001:9001 \
  -v ${PWD}/mosquitto/config:/mosquitto/config \
  -v ${PWD}/mosquitto/data:/mosquitto/data \
  -v ${PWD}/mosquitto/log:/mosquitto/log \
  mosquitto-broker-greenhouse-iot
```

### 📁 Folder Structure

- `mosquitto/config/` – Mosquitto configuration files (`mosquitto.conf`, `dynamic-security.json`)
- `mosquitto/data/` – Broker persistence data
- `mosquitto/log/` – Log output

---