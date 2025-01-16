package com.greenhouse.greenhouse_iot.model.dto.sensor.commands;

import jakarta.validation.constraints.Min;
import jakarta.validation.constraints.NotNull;
import lombok.Data;

@Data
public class ChangeReadingPeriodDto {
    @NotNull
    @Min(value = 60_000, message = "Minimal reading period is one minute")
    Integer readingPeriod;
}
