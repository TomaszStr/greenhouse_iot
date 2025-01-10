package com.greenhouse.greenhouse_iot.model.dto.sensor;

import com.greenhouse.greenhouse_iot.model.enums.SensorState;
import jakarta.validation.constraints.*;
import lombok.Data;

@Data
public class AddSensorDto {
    @NotBlank(message = "MAC address cannot be blank")
    @Pattern(regexp = "^([0-9A-Fa-f]{2}[.:-]){5}([0-9A-Fa-f]{2})$")
    private String macAddress;

    @NotBlank(message = "Sensor code cannot be blank")
    @Size(min = 3, max = 50, message = "Sensor code must be between 3 and 50 characters")
    private String sensorCode;

    @NotBlank(message = "Current state cannot be blank")
//    @Size(min = 3, max = 20, message = "Current state must be between 3 and 20 characters")
    private SensorState currentState;

    @NotNull(message = "Reading period cannot be null")
    @Min(value = 30, message = "Reading period must be at least 30 seconds")
    private Long readingPeriod;
}