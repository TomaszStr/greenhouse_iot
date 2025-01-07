package com.greenhouse.greenhouse_iot.model.dto.sensor;

import lombok.Data;

@Data
public class SensorDto {
    private Long ownerId;
    private String sensorName;
    private String state;
    private Integer readingPeriod;
}
