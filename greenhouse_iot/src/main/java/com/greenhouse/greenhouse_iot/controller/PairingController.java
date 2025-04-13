package com.greenhouse.greenhouse_iot.controller;

import com.greenhouse.greenhouse_iot.model.dto.sensor.AssignSensorToUserDto;
import com.greenhouse.greenhouse_iot.model.dto.sensor.PairSensorDto;
import com.greenhouse.greenhouse_iot.service.SensorService;
import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

@AllArgsConstructor
@Slf4j
@RestController
@RequestMapping("/pair")
public class PairingController {
    SensorService sensorService;
    @PutMapping("/users/{userId}/sensors")
    public ResponseEntity<PairSensorDto> assignSensorToUser(
            @PathVariable Long userId,
            @RequestBody AssignSensorToUserDto assignSensorToUserDto) {
        return ResponseEntity.ok(sensorService.assignSensorToUser(userId, assignSensorToUserDto));
    }
}
