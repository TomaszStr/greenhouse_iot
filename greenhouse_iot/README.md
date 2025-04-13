## 🌐 Backend — Java Spring Boot Server

This is the backend component of the **Greenhouse IoT System**, implemented as a monolithic RESTful web service. It handles user management, device pairing, secure MQTT communication, sensor data processing, and alert configuration. Built using modern Java and Spring technologies, the backend ensures a scalable, secure, and maintainable infrastructure.

---

### ⚙️ Technology Stack

- **Language**: Java 21
- **Framework**: Spring Boot 3.3.5
- **Authentication**: JWT (JSON Web Tokens)
- **MQTT Client**: Eclipse Paho MQTT library
- **Database**: PostgreSQL
- **API Documentation**: Swagger/OpenAPI
- **Spring Modules Used**:
    - `spring-boot-starter-web`
    - `spring-boot-starter-security`
    - `spring-boot-starter-data-jpa`
    - `spring-boot-starter-validation`

---

### 🧱 Architecture

Follows the **Controller–Service–Repository** pattern for clean separation of concerns and maintainability. Exception handling is centralized using `@ControllerAdvice` to ensure consistent and informative API error responses.

The application is organized as a **monolith**, with clearly separated modules for user management, device handling, MQTT integration, configuration management, and sensor data operations.

---

### 🔐 Authentication & Security

- Uses **JWT tokens** for stateless, secure authentication.
- Role-based access control (user/admin).
- Secure endpoints with token validation and user context extraction.
- Passwords are hashed using a secure algorithm (e.g., BCrypt).

---

### 🌍 REST API Overview

#### 📥 Public Endpoints
- `POST /auth/login` – User login
- `POST /users/register` – User registration
- `POST /auth/reset-password` – Change password

#### 🔐 Private Endpoints (JWT Required)

**User and Device Management**
- `PUT /pair/users/{userId}/sensors` - Assign sensor to user(sent by device)
- `PUT /sensors/{sensorId}/reading_period` - Update sensor reading period
- `PUT /sensors/{sensorId}/name` - Update sensor name
- `PUT /sensors/{sensorId}/state` - Update sensor state
- `PUT /sensors/{sensorId}/height` - Update sensor height above sea level(for air pressure measurements)
- `GET /sensors/{sensorId}` – Get device information
- `PUT /users/{userId}/password` - Change password
- `GET /users/{userId}/sensors` - Get user's sensors list
- `PUT /users/{userId}/alerts/{alertId}` - Read alert
- `DELETE /users/{userId}/sensors/{sensorId}` - Disconnect sensor from user's account

**Measurements and Alerts**
- `GET /users/{userId}/alerts` - Get user's alerts list
- `GET /sensors/{sensorId}/readings` – Get device measurements
- `PUT /sensors/{sensorId}/soil_moisture_alert_threshold` - Update soil moisture alert threshold for device
- `PUT /sensors/{sensorId}/soil_moisture_action_threshold` - Update soil moisture action threshold for device
- `PUT /sensors/{sensorId}/temperature_alert_threshold` - Update temperature alert threshold for device
- `PUT /sensors/{sensorId}/temperature_action_threshold` - Update temperature action threshold for device


**Admin-Only**
- `POST /sensors` – Add new device (generates secret key, saves hash, prepares MQTT topics)
- `GET /sensors` – Retrieve all sensors
- `GET /users` – Get all users list
---

### 🔄 MQTT Integration

The backend interfaces with the **Mosquitto MQTT broker** using the **Eclipse Paho client**. Responsibilities include:

- Managing device-specific MQTT topics
- Sending configuration updates and commands to devices
- Processing data received from sensors via MQTT
- Resetting MQTT credentials securely during pairing
- Updating ACLs and credentials dynamically (e.g., topic permissions)

All MQTT interactions are secured and linked to device ownership to prevent spoofing or hijacking.

---

### ✅ Key Features

- **Robust Device Pairing Flow**: Ensures a secure handoff between user and device using secret keys and MQTT credential regeneration.
- **Sensor Management**: Easily view, rename, and remove sensors from a user's account.
- **Alert System**: Configure dual-threshold alerts for key environmental metrics.
- **Configuration Management**: Devices can be configured remotely, including:
    - Reading frequency
    - Device naming
    - Height setting (used to normalize pressure measurements)
- **Admin Interface**: Lightweight API for provisioning new hardware units with secure credentials.

---

### 🧼 Code Quality & Best Practices

- Modular, clean, and testable code following SOLID principles
- Centralized exception handling and validation feedback
- Descriptive Swagger UI with clear endpoint documentation
- Logging with context for easier debugging
- Sensitive information never exposed in responses

---

### 📂 Project Structure

```
//src/main/java/com/greenhouse/greenhouse_iot/
├── controller/         # REST API endpoints
├── service/            # Business logic
├── repository/         # Spring Data JPA interfaces
├── security/           # JWT and role setup
├── mqtt/               # MQTT client and message handlers
├── model/              # Entity and DTO classes
├── config/             # Security, Swagger, MQTT configs
├── exception/          # Custom exception handlers
├── utils/              # Utils classes
└── GreenhouseIotApplication.java  # Main Spring Boot application
```

---

### 🧪 Development & Running Locally

> Prerequisites: Java 21, PostgreSQL, Maven

```bash
# Clone the repo
# Run PostgreSQL and Mosquitto locally
# Update application.properties with DB and broker settings

# Build the project
./gradlew build

# Run the application
./gradlew bootRun
```

Swagger UI available at:  
`http://localhost:8080/swagger-ui/index.html`

---