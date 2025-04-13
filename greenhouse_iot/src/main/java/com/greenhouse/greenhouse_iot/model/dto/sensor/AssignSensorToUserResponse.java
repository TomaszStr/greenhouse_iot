package com.greenhouse.greenhouse_iot.model.dto.sensor;

import com.greenhouse.greenhouse_iot.model.dto.MqttCredentials;
import lombok.Data;

@Data
public class AssignSensorToUserResponse {
    String mqttBrokerUrl;
    String mqttUsername;
    String mqttPassword;
}
