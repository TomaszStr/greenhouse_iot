package com.greenhouse.greenhouse_iot.model.mapper;

import com.greenhouse.greenhouse_iot.model.dto.SensorReadingDto;
import com.greenhouse.greenhouse_iot.model.mqtt.sensor_reading.SensorReadingMqtt;
import com.greenhouse.greenhouse_iot.model.entity.SensorReading;
import org.mapstruct.Mapper;
import org.mapstruct.Mapping;
import org.mapstruct.Mappings;

@Mapper(componentModel = "spring")
public interface SensorReadingMapper {
    SensorReadingDto sensorReadingToSensorReadingDto(SensorReading sensorReading);

//    SensorReading sensorReadingDtoToSensorReading(SensorReadingDto sensorReadingDto);

    @Mappings({
            @Mapping(target = "id", ignore = true),
            @Mapping(target = "sensor", ignore = true),
            @Mapping(target = "timestamp", expression = "java(java.time.Instant.ofEpochSecond(sensorReadingMqtt.getTimestamp()).atZone(java.time.ZoneId.systemDefault()).toLocalDateTime())")
    })
    SensorReading sensorReadingMqttToSensorReading(SensorReadingMqtt sensorReadingMqtt);


}
