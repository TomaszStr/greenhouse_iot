package com.greenhouse.greenhouse_iot.service;

import com.greenhouse.greenhouse_iot.exception.ResourceNotFoundException;
import com.greenhouse.greenhouse_iot.model.dto.SensorReadingDto;
import com.greenhouse.greenhouse_iot.model.entity.SensorAlert;
import com.greenhouse.greenhouse_iot.model.entity.User;
import com.greenhouse.greenhouse_iot.model.mapper.SensorAlertMapper;
import com.greenhouse.greenhouse_iot.model.mqtt.alert.MqttSensorAlert;
import com.greenhouse.greenhouse_iot.model.mqtt.sensor_reading.SensorReadingMqtt;
import com.greenhouse.greenhouse_iot.model.entity.Sensor;
import com.greenhouse.greenhouse_iot.model.entity.SensorReading;
import com.greenhouse.greenhouse_iot.model.mapper.SensorReadingMapper;
import com.greenhouse.greenhouse_iot.repository.SensorAlertRepository;
import com.greenhouse.greenhouse_iot.repository.SensorReadingRepository;
import com.greenhouse.greenhouse_iot.repository.SensorRepository;
import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.List;
import java.util.stream.Collectors;
@Slf4j
@Service
@AllArgsConstructor
public class SensorReadingServiceImpl implements SensorReadingService {

    private final SensorRepository sensorRepository;
    private final SensorReadingRepository sensorReadingRepository;
    private final SensorAlertRepository sensorAlertRepository;
    private final SensorReadingMapper sensorReadingMapper;
    private final SensorAlertMapper sensorAlertMapper;
    private final SecurityService securityService;

    @Override
    public List<SensorReadingDto> getSensorReadings() {
        log.info("Get all sensor readings");
        return sensorReadingRepository.findAll().stream().map(sensorReadingMapper::sensorReadingToSensorReadingDto).toList();
    }

    @Override
    public SensorReadingDto getSensorReadingById(Long id) {
        log.info("Get sensor reading with id: {}", id);
        return sensorReadingMapper.sensorReadingToSensorReadingDto(
                sensorReadingRepository.findById(id).orElse(null)
        );
    }

    @Override
    public Boolean addMqttReading(SensorReadingMqtt sensorReadingMqtt) {
        SensorReading reading = sensorReadingMapper.sensorReadingMqttToSensorReading(sensorReadingMqtt);
        Sensor sensor = sensorRepository.findByMqttName(sensorReadingMqtt.getSensorMqttName());
        reading.setSensor(sensor);
        reading = sensorReadingRepository.save(reading);
        return reading.getId() != null;
    }

    @Override
    public Boolean addMqttAlert(MqttSensorAlert mqttSensorAlert) {
        SensorAlert alert = sensorAlertMapper.mqttSensorAlertToSensorAlert(mqttSensorAlert);
        Sensor sensor = sensorRepository.findByMqttName(mqttSensorAlert.getSensorMqttName());
        alert.setSensor(sensor);
        alert.setUser(sensor.getUser());
        alert = sensorAlertRepository.save(alert);
        return alert.getId() != null;
    }

    @Override
    public List<SensorReadingDto> getReadingsBySensorId(Long sensorId, LocalDateTime startTime, LocalDateTime endTime) {
        User loggedInUser = securityService.getLoggedInUserOrThrow();

        Sensor sensor = sensorRepository.findById(sensorId)
                .orElseThrow(() -> new ResourceNotFoundException("Sensor not found"));

        securityService.checkSensorOwnership(loggedInUser, sensor);
        log.info("Get sensor reading for sensor: {}", sensorId);

        if (startTime == null) {
            startTime = LocalDateTime.now().minusDays(7);
        }
        if (endTime == null) {
            endTime = LocalDateTime.now();
        }
        List<SensorReading> sensorReadings = sensorReadingRepository.findSensorReadingsBySensorId(sensorId, startTime, endTime);

        return sensorReadings.stream().map(sensorReadingMapper::sensorReadingToSensorReadingDto).collect(Collectors.toList());
    }
}
