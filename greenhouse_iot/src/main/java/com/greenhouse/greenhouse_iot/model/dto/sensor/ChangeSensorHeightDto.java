package com.greenhouse.greenhouse_iot.model.dto.sensor;

import jakarta.validation.constraints.Max;
import jakarta.validation.constraints.Min;
import jakarta.validation.constraints.NotNull;
import lombok.Data;

@Data
public class ChangeSensorHeightDto {
    @NotNull
    @Min(value = -434, message = "Lowest depression on earth is 434 meters below sea level")
    @Max(value = 8849, message = "The most elevated point on earth is 8849 meters above sea level")
    Integer height;
}