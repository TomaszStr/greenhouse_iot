package com.greenhouse.greenhouse_iot.model.dto.sensor;

import jakarta.validation.constraints.Max;
import jakarta.validation.constraints.Min;
import jakarta.validation.constraints.NotNull;
import lombok.Data;

@Data
public class ChangeSoilMoistureAlertThresholdDto {
    @NotNull
    @Min(value = 0, message = "Soil moisture alert threshold minimal value is 0")
    @Max(value = 100, message = "Soil moisture alert threshold maximum value is 100")
    Integer threshold;
}
