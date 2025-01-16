package com.greenhouse.greenhouse_iot.model.mapper;

import com.greenhouse.greenhouse_iot.model.dto.sensor.AddSensorDto;
import com.greenhouse.greenhouse_iot.model.dto.sensor.SensorDetailDto;
import com.greenhouse.greenhouse_iot.model.dto.sensor.SensorDto;

import com.greenhouse.greenhouse_iot.model.entity.Sensor;
import org.mapstruct.Mapper;
import org.mapstruct.Mapping;
import org.mapstruct.Mappings;

@Mapper(componentModel = "spring")
public interface SensorMapper {
    @Mappings({
            @Mapping(target = "sensorId", source = "id"),
            @Mapping(target = "ownerId", source = "user.id")
    })
    SensorDto sensorToSensorDto(Sensor sensor);

    SensorDetailDto sensorToSensorDetailDto(Sensor sensor);

    @Mappings({
            @Mapping(target = "id", ignore = true),
            @Mapping(target = "user", ignore = true),
            @Mapping(target = "sensorMqttName", ignore = true),
            @Mapping(target = "sensorName", ignore = true),
            @Mapping(target = "height", ignore = true),
            @Mapping(target = "soilMoistureAlertThreshold", ignore = true),
            @Mapping(target = "soilMoistureActionThreshold", ignore = true),
            @Mapping(target = "temperatureAlertThreshold", ignore = true),
            @Mapping(target = "temperatureActionThreshold", ignore = true),
            @Mapping(target = "currentState", source = "currentState")
    })
    Sensor addSensorDtoToSensor(AddSensorDto addSensorDto);
}
