package com.greenhouse.greenhouse_iot.model.dto.sensor;

import com.greenhouse.greenhouse_iot.model.entity.User;
import jakarta.persistence.*;
import lombok.Data;

@Data
public class AddSensorDto {

    private String macAddress;

    private String sensorCode;

    private String currentState;

    private Long readingPeriod;
}