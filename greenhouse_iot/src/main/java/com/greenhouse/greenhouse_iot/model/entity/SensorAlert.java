package com.greenhouse.greenhouse_iot.model.entity;

import com.greenhouse.greenhouse_iot.model.mqtt.alert.MqttSensorAlertType;
import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

import java.time.LocalDateTime;

@Getter
@Setter
@Entity(name = "sensor_alerts")
public class SensorAlert {
    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long id;

    @Column(name = "timestamp")
    private LocalDateTime timestamp;

    @Column(name = "checked")
    private Boolean checked;

    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "sensor_id", referencedColumnName = "id", nullable = false)
    private Sensor sensor;

    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "user_id", referencedColumnName = "id", nullable = false)
    private User user;

    @Column(name = "value")
    private Double value;

    @Enumerated(EnumType.STRING)
    @Column(name = "alert_type", nullable = false)
    MqttSensorAlertType alertType;
}