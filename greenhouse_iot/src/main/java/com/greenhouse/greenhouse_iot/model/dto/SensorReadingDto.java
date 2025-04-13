package com.greenhouse.greenhouse_iot.model.dto;

import lombok.Data;

import java.time.LocalDateTime;

@Data
public class SensorReadingDto {
    private LocalDateTime timestamp;
    private Double temperature;
    private Double humidity;
    private Double pressure;
    private Double lightIntensity;
    private Double soilMoisture;
}
