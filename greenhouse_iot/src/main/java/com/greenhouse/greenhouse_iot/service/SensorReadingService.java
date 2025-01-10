package com.greenhouse.greenhouse_iot.service;

import com.greenhouse.greenhouse_iot.model.dto.SensorReadingDto;
import com.greenhouse.greenhouse_iot.model.mqtt.sensor_reading.SensorReadingMqtt;

import java.time.LocalDateTime;
import java.util.List;

public interface SensorReadingService {
    List<SensorReadingDto> getSensorReadings();
    SensorReadingDto getSensorReadingById(Long id);
    Boolean addMqttReading(SensorReadingMqtt sensorReadingMqtt);

    List<SensorReadingDto> getReadingsBySensorId(Long sensorId, LocalDateTime startTime, LocalDateTime endTime);
}
