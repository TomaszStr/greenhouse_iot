package com.greenhouse.greenhouse_iot.service;

import com.greenhouse.greenhouse_iot.model.dto.SensorReadingDto;
import com.greenhouse.greenhouse_iot.model.dto.SensorReadingMqtt;
import com.greenhouse.greenhouse_iot.model.entity.Sensor;
import com.greenhouse.greenhouse_iot.model.entity.SensorReading;
import com.greenhouse.greenhouse_iot.model.mapper.SensorReadingMapper;
import com.greenhouse.greenhouse_iot.repository.SensorReadingRepository;
import com.greenhouse.greenhouse_iot.repository.SensorRepository;
import lombok.AllArgsConstructor;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.List;
import java.util.stream.Collectors;

@Service
@AllArgsConstructor
public class SensorReadingServiceImpl implements SensorReadingService {

    private final SensorRepository sensorRepository;
    private final SensorReadingRepository sensorReadingRepository;
    private final SensorReadingMapper sensorReadingMapper;

    @Override
    public List<SensorReadingDto> getSensorReadings() {
        return sensorReadingRepository.findAll().stream().map(sensorReadingMapper::sensorReadingToSensorReadingDto).toList();
    }

    @Override
    public SensorReadingDto getSensorReadingById(Long id) {
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
    public List<SensorReadingDto> getReadingsBySensorId(Long sensorId, LocalDateTime startTime, LocalDateTime endTime) {
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
