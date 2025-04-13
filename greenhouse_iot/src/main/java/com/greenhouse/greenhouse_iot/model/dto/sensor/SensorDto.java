package com.greenhouse.greenhouse_iot.model.dto.sensor;

import com.greenhouse.greenhouse_iot.model.enums.SensorState;
import lombok.Data;

@Data
public class SensorDto {
    private Long sensorId;
    private Long ownerId;
    private String sensorName;
    private SensorState currentState;
    private Integer readingPeriod;
}
