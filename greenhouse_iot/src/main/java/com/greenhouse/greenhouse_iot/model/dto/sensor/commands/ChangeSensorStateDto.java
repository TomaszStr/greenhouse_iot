package com.greenhouse.greenhouse_iot.model.dto.sensor.commands;

import com.greenhouse.greenhouse_iot.model.enums.SensorState;
import lombok.Data;

@Data
public class ChangeSensorStateDto {
    SensorState sensorState;
}
