package com.greenhouse.greenhouse_iot.model.dto.sensor;

import jakarta.validation.constraints.Max;
import jakarta.validation.constraints.Min;
import jakarta.validation.constraints.NotNull;
import lombok.Data;

@Data
public class ChangeTemperatureThresholdDto {
    @NotNull
    @Min(value = -100, message = "Temperature threshold minimal value is -100")
    @Max(value = 100, message = "Temperature threshold maximum value is 100")
    Integer threshold;
}
