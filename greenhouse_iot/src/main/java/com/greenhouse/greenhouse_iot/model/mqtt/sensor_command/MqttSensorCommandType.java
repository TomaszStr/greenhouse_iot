package com.greenhouse.greenhouse_iot.model.mqtt.sensor_command;

import lombok.Getter;

@Getter
public enum MqttSensorCommandType {
    SET_STATE(0),
    SET_READING_PERIOD(1),
    SET_HEIGHT(2),
    SET_SOIL_MOISTURE_ALERT_THRESHOLD(3),
    SET_SOIL_MOISTURE_ACTION_THRESHOLD(4),
    SET_TEMPERATURE_ALERT_THRESHOLD(5),
    SET_TEMPERATURE_ACTION_THRESHOLD(6);

    private final Integer code;

    MqttSensorCommandType(Integer code) {
        this.code = code;
    }
}
