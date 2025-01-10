package com.greenhouse.greenhouse_iot.model.dto.sensor;

import jakarta.validation.constraints.NotBlank;
import lombok.Data;

@Data
public class ChangeSensorNameDto {
    @NotBlank
    String sensorName;
}
