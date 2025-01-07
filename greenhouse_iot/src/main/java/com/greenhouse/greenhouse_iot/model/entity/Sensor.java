package com.greenhouse.greenhouse_iot.model.entity;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
@Entity(name = "sensors")
public class Sensor {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    Long id;

    @ManyToOne
    @JoinColumn(name = "user_id")
    User user;

    @Column(name = "mac_address", unique = true, nullable = false)
    String macAddress;

    @Column(name = "sensor_code", unique = true, nullable = false)
    String sensorCode;

    @Column(name = "sensor_mqtt_name", unique = true)
    String sensorMqttName;

    @Column(name = "sensor_name")
    String sensorName;

    @Column(name = "reading_period", nullable = false)
    Integer readingPeriod;

    @Column(name = "height")
    Integer height;

    @Column(name = "soil_moisture_alert_threshold")
    Integer soilMoistureAlertThreshold;

    @Column(name = "temperature_alert_threshold")
    Integer temperatureAlertThreshold;

    @Column(name = "current_state", nullable = false)
    String currentState;
}
