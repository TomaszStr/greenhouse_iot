package com.greenhouse.greenhouse_iot.model.mqtt.sensor_reading;

import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.Data;

@Data
public class SensorReadingMqtt {
    private String sensorMqttName;
    @JsonProperty("timestamp")
    private Long timestamp;
    @JsonProperty("temperature")
    private Double temperature;
    @JsonProperty("humidity")
    private Double humidity;
    @JsonProperty("pressure")
    private Double pressure;
    @JsonProperty("light_intensity")
    private Integer lightIntensity;
    @JsonProperty("soil_moisture")
    private Integer soilMoisture;
}
