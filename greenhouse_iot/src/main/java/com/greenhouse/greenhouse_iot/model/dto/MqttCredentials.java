package com.greenhouse.greenhouse_iot.model.dto;

import lombok.AllArgsConstructor;
import lombok.Data;

@Data
@AllArgsConstructor
public class MqttCredentials {
    String mqttUsername;
    String mqttPassword;
}
