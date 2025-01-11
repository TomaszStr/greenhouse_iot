package com.greenhouse.greenhouse_iot.service;

import com.greenhouse.greenhouse_iot.model.dto.sensor.*;

import java.util.List;

public interface SensorService {
    List<SensorDto> getSensors();
    SensorDto getSensorById(Long id);
    SensorDto addSensor(AddSensorDto addSensorDto);
    PairSensorDto assignSensorToUser(Long userId, AssignSensorToUserDto assignSensorToUserDto);
    PairSensorDto pairSensor(Long userId, Long sensorId);
    Boolean disconnectSensorFromUser(Long userId, Long sensorId, DisconnectSensorFromUserDto disconnectSensorFromUserDto);
    Boolean changeSensorName(Long sensorId, ChangeSensorNameDto changeSensorNameDto);
    Boolean changeReadingPeriod(Long sensorId, ChangeReadingPeriodDto changeReadingPeriodDto);
    Boolean changeSensorState(Long sensorId, ChangeSensorStateDto changeSensorStateDto);
    Boolean changeSensorHeight(Long sensorId, ChangeSensorHeightDto changeSensorHeightDto);
    Boolean changeSoilMoistureAlertThreshold(Long sensorId, ChangeSoilMoistureAlertThresholdDto changeSoilMoistureAlertThresholdDto);
    Boolean changeTemperatureAlertThreshold(Long sensorId, ChangeTemperatureThresholdDto changeTemperatureThresholdDto);
    Boolean changeTemperatureActionThreshold(Long sensorId, ChangeTemperatureThresholdDto changeTemperatureThresholdDto);
    List<SensorDto> getSensorsByUserId(Long userId);
}
