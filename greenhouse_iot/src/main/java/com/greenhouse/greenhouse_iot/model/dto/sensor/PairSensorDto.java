package com.greenhouse.greenhouse_iot.model.dto.sensor;

import lombok.Data;

@Data
public class PairSensorDto {
    String mqttBrokerUrl;
    String mqttUsername;
    String mqttPassword;
}
