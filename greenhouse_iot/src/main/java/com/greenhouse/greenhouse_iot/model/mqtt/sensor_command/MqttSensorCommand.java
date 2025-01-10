package com.greenhouse.greenhouse_iot.model.mqtt.sensor_command;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.Data;

@Data
public class MqttSensorCommand {
    @JsonProperty("command_code")
    private Integer commandCode;
    @JsonProperty("value")
    private Integer value;

    public MqttSensorCommand(MqttSensorCommandType commandType, Integer value) {
        this.commandCode = commandType.getCode();
        this.value = value;
    }
}
