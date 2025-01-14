package com.greenhouse.greenhouse_iot.service;

import com.greenhouse.greenhouse_iot.exception.GreenhouseAccessDeniedException;
import com.greenhouse.greenhouse_iot.model.dto.MqttCredentials;
import com.greenhouse.greenhouse_iot.model.dto.sensor.*;
import com.greenhouse.greenhouse_iot.model.entity.Sensor;
import com.greenhouse.greenhouse_iot.model.entity.User;
import com.greenhouse.greenhouse_iot.exception.ResourceNotFoundException;
import com.greenhouse.greenhouse_iot.model.mapper.SensorMapper;
import com.greenhouse.greenhouse_iot.repository.SensorRepository;
import com.greenhouse.greenhouse_iot.repository.UserRepository;
import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Objects;
import java.util.stream.Collectors;

@Slf4j
@Service
@AllArgsConstructor
public class SensorServiceImpl implements SensorService{
    private final SensorRepository sensorRepository;
    private final UserRepository userRepository;
    private final SensorMapper sensorMapper;
    private final MqttService mqttService;
    private final SecurityService securityService;
    private final PasswordEncoder passwordEncoder;
    @Override
    public List<SensorDto> getSensors() {
        return sensorRepository.findAll().stream()
                .map(sensorMapper::sensorToSensorDto).toList();
    }

    @Override
    public SensorDto getSensorById(Long sensorId) {
        User loggedInUser = securityService.getLoggedInUserOrThrow();

        Sensor sensor = sensorRepository.findById(sensorId)
                .orElseThrow(() -> new ResourceNotFoundException("Sensor not found"));

        securityService.checkSensorOwnership(loggedInUser, sensor);

        return sensorMapper.sensorToSensorDto(sensor);
    }

    @Override
    public SensorDto addSensor(AddSensorDto addSensorDto) {
        Sensor sensor = sensorMapper.addSensorDtoToSensor(addSensorDto);
        sensor.setUser(null);
        sensor.setSensorCode(passwordEncoder.encode(addSensorDto.getSensorCode()));
        sensor.setSensorName("default-name");

        sensor = sensorRepository.save(sensor);
        sensor.setSensorMqttName("device"+sensor.getId());
        sensor = sensorRepository.save(sensor);

        MqttCredentials mqttCredentials = mqttService.configureMqttForNewSensor(sensor);
        if(mqttCredentials != null){
            return sensorMapper.sensorToSensorDto(sensor);
        }
        else {
            sensorRepository.delete(sensor);
            throw new RuntimeException("Failed to configure MQTT for the new sensor");
        }
    }

//    @Override
//    public SensorDto assignSensorToUser(Long userId, AssignSensorToUserDto assignSensorToUserDto) {
//        User loggedInUser = securityService.getLoggedInUserOrThrow();
//
//        securityService.checkUserIdMatch(loggedInUser.getId(), userId);
//
//        Sensor sensor = sensorRepository.findById(assignSensorToUserDto.getSensorId())
//                .orElseThrow(() -> new ResourceNotFoundException("Sensor not found"));
//
//        if(sensor.getUser() != null){
//            log.error("This sensor already has an owner");
//            throw new RuntimeException("This sensor already has an owner");
//        }
//
//        if(!passwordEncoder.matches(assignSensorToUserDto.getSensorCode(), sensor.getSensorCode())) {
//            log.error("Incorrect sensor code");
//            throw new RuntimeException("Incorrect sensor code");
//        }
//
//        User owner = userRepository.findById(userId)
//                .orElseThrow(() -> new ResourceNotFoundException("User not found"));
//
//        sensor.setUser(owner);
//        sensor.setSensorName(assignSensorToUserDto.getSensorName());
//        sensor = sensorRepository.save(sensor);
//
//        if(sensor.getUser().equals(owner)){
//            return sensorMapper.sensorToSensorDto(sensor);
//        }
//        else {
//            throw new RuntimeException("Failed");
//        }
//    }

    @Override
    public PairSensorDto assignSensorToUser(Long userId, AssignSensorToUserDto assignSensorToUserDto) {
        log.info("Pairing request from sensor with ID: {} to user with ID: {}", assignSensorToUserDto.getSensorId(), userId);
        Sensor sensor = sensorRepository.findById(assignSensorToUserDto.getSensorId())
                .orElseThrow(() -> new ResourceNotFoundException("Sensor not found"));

        if(!passwordEncoder.matches(assignSensorToUserDto.getSensorCode(), sensor.getSensorCode())) {
            log.error("Incorrect sensor code");
            throw new GreenhouseAccessDeniedException("Incorrect sensor code");
        }

        User owner = userRepository.findById(userId)
                .orElseThrow(() -> new ResourceNotFoundException("User not found"));

        sensor.setUser(owner);
//        sensor.setSensorName(assignSensorToUserDto.getSensorName());
        sensor = sensorRepository.save(sensor);

        if(sensor.getUser().equals(owner)){
            return pairSensor(owner.getId(), sensor.getId());
        }
        else {
            throw new RuntimeException("Failed");
        }
    }

    @Override
    public PairSensorDto pairSensor(Long userId, Long sensorId) {
//        User loggedInUser = securityService.getLoggedInUserOrThrow();
//
//        securityService.checkUserIdMatch(loggedInUser.getId(), userId);

        Sensor sensor = sensorRepository.findById(sensorId)
                .orElseThrow(() -> new ResourceNotFoundException("Sensor not found"));

//        securityService.checkSensorOwnership(loggedInUser, sensor);

        MqttCredentials mqttCredentials = mqttService.resetMqttPasswordForSensor(sensor);
        PairSensorDto pairSensorDto = new PairSensorDto();
        pairSensorDto.setMqttBrokerUrl(mqttService.getMqttBrokerUrl());
        pairSensorDto.setMqttUsername(mqttCredentials.getMqttUsername());
        pairSensorDto.setMqttPassword(mqttCredentials.getMqttPassword());

        return pairSensorDto;
    }

    @Override
    public Boolean disconnectSensorFromUser(Long userId, Long sensorId, DisconnectSensorFromUserDto disconnectSensorFromUserDto) {

        User loggedInUser = securityService.getLoggedInUserOrThrow();

        securityService.checkUserIdMatch(loggedInUser.getId(), userId);

        Sensor sensor = sensorRepository.findById(sensorId)
                .orElseThrow(() -> new ResourceNotFoundException("Sensor not found"));

        securityService.checkSensorOwnership(loggedInUser, sensor);

        sensor.setUser(null);
        sensor = sensorRepository.save(sensor);

        return sensor.getUser() == null;
    }

    @Override
    public Boolean changeSensorName(Long sensorId, ChangeSensorNameDto changeSensorNameDto) {
        User loggedInUser = securityService.getLoggedInUserOrThrow();

        Sensor sensor = sensorRepository.findById(sensorId)
                .orElseThrow(() -> new ResourceNotFoundException("Sensor not found"));

        securityService.checkSensorOwnership(loggedInUser, sensor);
        log.info("Change name for sensor: {}, value: {}", sensorId, changeSensorNameDto.getSensorName());
        sensor.setSensorName(changeSensorNameDto.getSensorName());
        sensor = sensorRepository.save(sensor);

        return Objects.equals(sensor.getSensorName(), changeSensorNameDto.getSensorName());
    }

    @Override
    public Boolean changeReadingPeriod(Long sensorId, ChangeReadingPeriodDto changeReadingPeriodDto) {

        User loggedInUser = securityService.getLoggedInUserOrThrow();

        Sensor sensor = sensorRepository.findById(sensorId)
                .orElseThrow(() -> new ResourceNotFoundException("Sensor not found"));

        securityService.checkSensorOwnership(loggedInUser, sensor);
        log.info("Change reading period for sensor: {}, value: {}", sensorId, changeReadingPeriodDto.getReadingPeriod());
        if(mqttService.setSensorReadingPeriod(sensor.getSensorMqttName(), changeReadingPeriodDto.getReadingPeriod())){
            sensor.setReadingPeriod(changeReadingPeriodDto.getReadingPeriod());
            sensor = sensorRepository.save(sensor);

            return Objects.equals(sensor.getReadingPeriod(), changeReadingPeriodDto.getReadingPeriod());
        }

        return false;
    }

    @Override
    public Boolean changeSensorState(Long sensorId, ChangeSensorStateDto changeSensorStateDto) {

        User loggedInUser = securityService.getLoggedInUserOrThrow();

        Sensor sensor = sensorRepository.findById(sensorId)
                .orElseThrow(() -> new ResourceNotFoundException("Sensor not found"));

        securityService.checkSensorOwnership(loggedInUser, sensor);
        log.info("Change state for sensor: {}, value: {}", sensorId, changeSensorStateDto.getSensorState());
        if(mqttService.setSensorState(sensor.getSensorMqttName(), changeSensorStateDto.getSensorState())) {
            sensor.setCurrentState(changeSensorStateDto.getSensorState());
            sensor = sensorRepository.save(sensor);
            return Objects.equals(sensor.getCurrentState(), changeSensorStateDto.getSensorState());
        }

        return false;
    }
    @Override
    public Boolean changeSensorHeight(Long sensorId, ChangeSensorHeightDto changeSensorHeightDto) {
        User loggedInUser = securityService.getLoggedInUserOrThrow();

        Sensor sensor = sensorRepository.findById(sensorId)
                .orElseThrow(() -> new ResourceNotFoundException("Sensor not found"));

        securityService.checkSensorOwnership(loggedInUser, sensor);
        log.info("Change height for sensor: {}, value: {}", sensorId, changeSensorHeightDto.getHeight());
        if(mqttService.setSensorHeight(sensor.getSensorMqttName(), changeSensorHeightDto.getHeight())) {
            sensor.setHeight(changeSensorHeightDto.getHeight());
            sensor = sensorRepository.save(sensor);
            return Objects.equals(sensor.getHeight(), changeSensorHeightDto.getHeight());
        }

        return false;
    }

    @Override
    public Boolean changeSoilMoistureAlertThreshold(Long sensorId, ChangeSoilMoistureThresholdDto changeSoilMoistureThresholdDto) {
        User loggedInUser = securityService.getLoggedInUserOrThrow();

        Sensor sensor = sensorRepository.findById(sensorId)
                .orElseThrow(() -> new ResourceNotFoundException("Sensor not found"));

        securityService.checkSensorOwnership(loggedInUser, sensor);
        log.info("Change soil moisture alert threshold for sensor: {}, value: {}", sensorId, changeSoilMoistureThresholdDto.getThreshold());
        if(mqttService.setSoilMoistureAlertThreshold(sensor.getSensorMqttName(), changeSoilMoistureThresholdDto.getThreshold())) {
            sensor.setSoilMoistureAlertThreshold(changeSoilMoistureThresholdDto.getThreshold());
            sensor = sensorRepository.save(sensor);
            return Objects.equals(sensor.getSoilMoistureAlertThreshold(), changeSoilMoistureThresholdDto.getThreshold());
        }

        return false;
    }

    @Override
    public Boolean changeSoilMoistureActionThreshold(Long sensorId, ChangeSoilMoistureThresholdDto changeSoilMoistureThresholdDto) {
        User loggedInUser = securityService.getLoggedInUserOrThrow();

        Sensor sensor = sensorRepository.findById(sensorId)
                .orElseThrow(() -> new ResourceNotFoundException("Sensor not found"));

        securityService.checkSensorOwnership(loggedInUser, sensor);
        log.info("Change soil moisture action threshold for sensor: {}, value: {}", sensorId, changeSoilMoistureThresholdDto.getThreshold());
        if(mqttService.setSoilMoistureActionThreshold(sensor.getSensorMqttName(), changeSoilMoistureThresholdDto.getThreshold())) {
            sensor.setSoilMoistureActionThreshold(changeSoilMoistureThresholdDto.getThreshold());
            sensor = sensorRepository.save(sensor);
            return Objects.equals(sensor.getSoilMoistureActionThreshold(), changeSoilMoistureThresholdDto.getThreshold());
        }

        return false;
    }

    @Override
    public Boolean changeTemperatureAlertThreshold(Long sensorId, ChangeTemperatureThresholdDto changeTemperatureThresholdDto) {
        User loggedInUser = securityService.getLoggedInUserOrThrow();

        Sensor sensor = sensorRepository.findById(sensorId)
                .orElseThrow(() -> new ResourceNotFoundException("Sensor not found"));
        log.info("Change temperature alert threshold for sensor: {}, value: {}", sensorId, changeTemperatureThresholdDto.getThreshold());
        securityService.checkSensorOwnership(loggedInUser, sensor);

        if(mqttService.setTemperatureAlertThreshold(sensor.getSensorMqttName(), changeTemperatureThresholdDto.getThreshold())) {
            sensor.setTemperatureAlertThreshold(changeTemperatureThresholdDto.getThreshold());
            sensor = sensorRepository.save(sensor);
            return Objects.equals(sensor.getTemperatureAlertThreshold(), changeTemperatureThresholdDto.getThreshold());
        }

        return false;
    }

    @Override
    public Boolean changeTemperatureActionThreshold(Long sensorId, ChangeTemperatureThresholdDto changeTemperatureThresholdDto) {
        User loggedInUser = securityService.getLoggedInUserOrThrow();

        Sensor sensor = sensorRepository.findById(sensorId)
                .orElseThrow(() -> new ResourceNotFoundException("Sensor not found"));
        log.info("Change temperature alert threshold for sensor: {}, value: {}", sensorId, changeTemperatureThresholdDto.getThreshold());
        securityService.checkSensorOwnership(loggedInUser, sensor);

        if(mqttService.setTemperatureActionThreshold(sensor.getSensorMqttName(), changeTemperatureThresholdDto.getThreshold())) {
            sensor.setTemperatureActionThreshold(changeTemperatureThresholdDto.getThreshold());
            sensor = sensorRepository.save(sensor);
            return Objects.equals(sensor.getTemperatureActionThreshold(), changeTemperatureThresholdDto.getThreshold());
        }

        return false;
    }

    @Override
    public List<SensorDto> getSensorsByUserId(Long userId) {
        User loggedInUser = securityService.getLoggedInUserOrThrow();

        securityService.checkUserIdMatch(loggedInUser.getId(), userId);

        List<Sensor> sensors = sensorRepository.findSensorsByUserId(userId);
        return sensors.stream().map(sensorMapper::sensorToSensorDto).collect(Collectors.toList());
    }
}