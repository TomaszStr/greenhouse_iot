package com.greenhouse.greenhouse_iot.model.mapper;

import com.greenhouse.greenhouse_iot.model.dto.SensorAlertDto;
import com.greenhouse.greenhouse_iot.model.entity.SensorAlert;
import com.greenhouse.greenhouse_iot.model.mqtt.alert.MqttSensorAlert;
import org.mapstruct.Mapper;
import org.mapstruct.Mapping;
import org.mapstruct.Mappings;

@Mapper(componentModel = "spring")
public interface SensorAlertMapper {
    SensorAlertDto sensorReadingToSensorReadingDto(SensorAlert alert);

    @Mappings({
            @Mapping(target = "id", ignore = true),
            @Mapping(target = "sensor", ignore = true),
            @Mapping(target = "user", ignore = true),
            @Mapping(target = "checked", expression = "java(false)"),
            @Mapping(target = "timestamp", expression = "java(java.time.Instant.ofEpochSecond(mqttSensorAlert.getTimestamp()).atZone(java.time.ZoneId.systemDefault()).toLocalDateTime())")
    })
    SensorAlert mqttSensorAlertToSensorAlert(MqttSensorAlert mqttSensorAlert);
}
