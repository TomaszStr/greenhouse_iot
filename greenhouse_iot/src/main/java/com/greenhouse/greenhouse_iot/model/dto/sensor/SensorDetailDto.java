package com.greenhouse.greenhouse_iot.model.dto.sensor;

import lombok.Data;

@Data
public class SensorDetailDto {
    private Long sensorId;
    private Long ownerId;
    private String sensorName;
    private Integer readingPeriod;
    private Integer height;
    private Integer soilMoistureAlertThreshold;
    private Integer soilMoistureActionThreshold;
    private Integer temperatureAlertThreshold;
    private Integer temperatureActionThreshold;
}
