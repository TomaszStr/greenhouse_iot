package com.greenhouse.greenhouse_iot.model.dto.sensor;

import lombok.Data;

@Data
public class AssignSensorToUserDto {
    Long sensorId;
    String sensorName;
    String sensorCode;
    String username;
    String securityPin;
}
