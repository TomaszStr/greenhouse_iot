package com.greenhouse.greenhouse_iot.model.mqtt.alert;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.Data;

@Data
public class MqttSensorAlert {
    private String sensorMqttName;
    @JsonProperty("timestamp")
    private Long timestamp;
    @JsonProperty("alertType")
    private MqttSensorAlertType alertType;
    @JsonProperty("value")
    private Double value;
}
