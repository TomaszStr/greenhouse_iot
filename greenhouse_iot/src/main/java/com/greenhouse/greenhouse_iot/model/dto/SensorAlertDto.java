package com.greenhouse.greenhouse_iot.model.dto;

import com.greenhouse.greenhouse_iot.model.mqtt.alert.MqttSensorAlertType;
import lombok.Data;

import java.time.LocalDateTime;

@Data
public class SensorAlertDto {
    private Long id;
    private LocalDateTime timestamp;
    private Double value;
    private Boolean checked;
    private MqttSensorAlertType alertType;
}
