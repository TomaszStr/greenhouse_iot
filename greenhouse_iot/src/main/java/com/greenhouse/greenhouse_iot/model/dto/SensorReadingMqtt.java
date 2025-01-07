package com.greenhouse.greenhouse_iot.model.dto;

import lombok.Data;

import java.time.LocalDateTime;

@Data
public class SensorReadingMqtt {
    private String sensorMqttName;
    private Long timestamp;
    private Double temperature;
    private Double humidity;
    private Double pressure;
    private Integer lightIntensity;
    private Integer soilMoisture;
}
