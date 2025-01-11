package com.greenhouse.greenhouse_iot.controller;

import com.greenhouse.greenhouse_iot.model.dto.SensorReadingDto;
import com.greenhouse.greenhouse_iot.model.dto.sensor.*;
import com.greenhouse.greenhouse_iot.service.SensorReadingService;
import com.greenhouse.greenhouse_iot.service.SensorService;
import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.ResponseEntity;
import org.springframework.security.access.prepost.PreAuthorize;
import org.springframework.web.bind.annotation.*;

import java.time.LocalDateTime;
import java.util.List;

@AllArgsConstructor
@Slf4j
@RestController
@RequestMapping("/sensors")
public class SensorController {
    private final SensorService sensorService;
    private final SensorReadingService sensorReadingService;

    @PreAuthorize("hasRole('ADMIN')")
    @GetMapping
    public ResponseEntity<List<SensorDto>> getSensors() {
        return ResponseEntity.ok(sensorService.getSensors());
    }

    @GetMapping("/{sensorId}")
    public ResponseEntity<SensorDto> getSensorById(@PathVariable Long sensorId) {
        return ResponseEntity.ok(sensorService.getSensorById(sensorId));
    }

    @PreAuthorize("hasRole('ADMIN')")
    @PostMapping
    public ResponseEntity<SensorDto> addSensor(@RequestBody AddSensorDto addSensorDto) {
        return ResponseEntity.ok(sensorService.addSensor(addSensorDto));
    }

    @PutMapping("/{sensorId}/reading_period")
    public ResponseEntity<Boolean> changeReadingPeriod(@PathVariable Long sensorId, @RequestBody ChangeReadingPeriodDto changeReadingPeriodDto) {
        return ResponseEntity.ok(sensorService.changeReadingPeriod(sensorId, changeReadingPeriodDto));
    }

    @PutMapping("/{sensorId}/name")
    public ResponseEntity<Boolean> changeSensorName(@PathVariable Long sensorId, @RequestBody ChangeSensorNameDto changeSensorNameDto) {
        return ResponseEntity.ok(sensorService.changeSensorName(sensorId, changeSensorNameDto));
    }

    @PutMapping("/{sensorId}/state")
    public ResponseEntity<Boolean> changeSensorState(@PathVariable Long sensorId, @RequestBody ChangeSensorStateDto changeSensorStateDto) {
        return ResponseEntity.ok(sensorService.changeSensorState(sensorId, changeSensorStateDto));
    }

    @PutMapping("/{sensorId}/height")
    public ResponseEntity<Boolean> changeSensorHeight(@PathVariable Long sensorId, @RequestBody ChangeSensorHeightDto changeSensorHeightDto) {
        return ResponseEntity.ok(sensorService.changeSensorHeight(sensorId, changeSensorHeightDto));
    }

    @PutMapping("/{sensorId}/soil_moisture_alert_threshold")
    public ResponseEntity<Boolean> changeSoilMoistureAlertThreshold(@PathVariable Long sensorId,
                                                    @RequestBody ChangeSoilMoistureAlertThresholdDto changeSoilMoistureAlertThresholdDto) {
        return ResponseEntity.ok(sensorService.changeSoilMoistureAlertThreshold(sensorId, changeSoilMoistureAlertThresholdDto));
    }

    @PutMapping("/{sensorId}/temperature_alert_threshold")
    public ResponseEntity<Boolean> changeTemperatureAlertThreshold(@PathVariable Long sensorId,
                                                   @RequestBody ChangeTemperatureThresholdDto changeTemperatureThresholdDto) {
        return ResponseEntity.ok(sensorService.changeTemperatureAlertThreshold(sensorId, changeTemperatureThresholdDto));
    }

    @PutMapping("/{sensorId}/temperature_action_threshold")
    public ResponseEntity<Boolean> changeTemperatureActionThreshold(@PathVariable Long sensorId,
                                                                   @RequestBody ChangeTemperatureThresholdDto changeTemperatureThresholdDto) {
        return ResponseEntity.ok(sensorService.changeTemperatureActionThreshold(sensorId, changeTemperatureThresholdDto));
    }

    @GetMapping("/{sensorId}/readings")
    public ResponseEntity<List<SensorReadingDto>> getSensorReadings(
            @PathVariable Long sensorId,
            @RequestParam(required = false) LocalDateTime startTime,
            @RequestParam(required = false) LocalDateTime endTime) {
        return ResponseEntity.ok(sensorReadingService.getReadingsBySensorId(sensorId, startTime, endTime));
    }

}
