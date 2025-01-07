package com.greenhouse.greenhouse_iot.model.mapper;

import com.greenhouse.greenhouse_iot.model.dto.SensorReadingDto;
import com.greenhouse.greenhouse_iot.model.dto.SensorReadingMqtt;
import com.greenhouse.greenhouse_iot.model.dto.UserDto;
import com.greenhouse.greenhouse_iot.model.entity.SensorReading;
import com.greenhouse.greenhouse_iot.model.entity.User;
import org.mapstruct.Mapper;
import org.mapstruct.Mapping;

@Mapper(componentModel = "spring")
public interface SensorReadingMapper {
    SensorReadingDto sensorReadingToSensorReadingDto(SensorReading sensorReading);
    SensorReading sensorReadingDtoToSensorReading(SensorReadingDto sensorReadingDto);

    @Mapping(target = "timestamp", expression = "java(java.time.Instant.ofEpochMilli(sensorReadingMqtt.getTimestamp()).atZone(java.time.ZoneId.systemDefault()).toLocalDateTime())")
    SensorReading sensorReadingMqttToSensorReading(SensorReadingMqtt sensorReadingMqtt);
}
