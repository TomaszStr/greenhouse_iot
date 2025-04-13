package com.greenhouse.greenhouse_iot.model.dto.sensor;

import com.greenhouse.greenhouse_iot.model.enums.SensorState;
import jakarta.validation.constraints.*;
import lombok.Data;
import org.springframework.validation.annotation.Validated;

@Data
public class AddSensorDto {
    @NotBlank(message = "MAC address cannot be blank")
    @Pattern(regexp = "^([0-9A-Fa-f]{2}[-]){5}([0-9A-Fa-f]{2})$", message = "MAC has to be in correct format")
    private String macAddress;

    @NotBlank(message = "Sensor code cannot be blank")
    @Size(min = 3, max = 50, message = "Sensor code must be between 3 and 50 characters")
    private String sensorCode;

    @NotNull(message = "Current state cannot be blank")
//    @Size(min = 3, max = 20, message = "Current state must be between 3 and 20 characters")
    private SensorState currentState;

}