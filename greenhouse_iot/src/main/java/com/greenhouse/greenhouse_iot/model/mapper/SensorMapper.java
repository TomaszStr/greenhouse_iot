package com.greenhouse.greenhouse_iot.model.mapper;

import com.greenhouse.greenhouse_iot.model.dto.sensor.AddSensorDto;
import com.greenhouse.greenhouse_iot.model.dto.sensor.SensorDto;

import com.greenhouse.greenhouse_iot.model.entity.Sensor;
import org.mapstruct.Mapper;

@Mapper(componentModel = "spring")
public interface SensorMapper {
    SensorDto sensorToSensorDto(Sensor sensor);
    Sensor sensorDtoToSensor(SensorDto sensorDto);

    Sensor addSensorDtoToSensor(AddSensorDto addSensorDto);
}
