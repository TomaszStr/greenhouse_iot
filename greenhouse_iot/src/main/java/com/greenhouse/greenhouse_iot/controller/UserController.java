package com.greenhouse.greenhouse_iot.controller;

import com.greenhouse.greenhouse_iot.model.dto.ChangePasswordDto;
import com.greenhouse.greenhouse_iot.model.dto.SensorAlertDto;
import com.greenhouse.greenhouse_iot.model.dto.UserDto;
import com.greenhouse.greenhouse_iot.model.dto.auth.RegistrationRequest;
import com.greenhouse.greenhouse_iot.model.dto.sensor.*;
import com.greenhouse.greenhouse_iot.service.SensorService;
import com.greenhouse.greenhouse_iot.service.UserService;
import jakarta.validation.Valid;
import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.access.prepost.PreAuthorize;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@AllArgsConstructor
@Slf4j
@RestController
@RequestMapping("/users")
public class UserController {
    private final SensorService sensorService;
    private final UserService userService;

    @PreAuthorize("hasRole('ADMIN')")
    @GetMapping("")
    public ResponseEntity<List<UserDto>> getUsers(){
        return ResponseEntity.ok(userService.getUsers());
    }

    @PostMapping("/register")
    public ResponseEntity<UserDto> registerUser(@Valid @RequestBody RegistrationRequest registerUserDto) {
        try {
            UserDto user = userService.registerUser(registerUserDto);
            return ResponseEntity.status(HttpStatus.CREATED).body(user);
        } catch (IllegalArgumentException e) {
            return ResponseEntity.status(HttpStatus.BAD_REQUEST).body(null);
        }
    }

    @PutMapping("/{userId}/password")
    public ResponseEntity<Void> changePassword(@PathVariable Long userId,
                                               @RequestBody ChangePasswordDto changePasswordDto) {
        userService.changePassword(userId, changePasswordDto);
        return ResponseEntity.noContent().build();
    }

    @GetMapping("/{userId}/sensors")
    public ResponseEntity<List<SensorDto>> getUsersSensors(@PathVariable Long userId) {
        return ResponseEntity.ok(sensorService.getSensorsByUserId(userId));
    }

    @GetMapping("/{userId}/alerts")
    public ResponseEntity<List<SensorAlertDto>> getUsersAlerts(
            @PathVariable Long userId,
            @RequestParam(required = false) Boolean checked) {
        return ResponseEntity.ok(userService.getAlertsByUserId(userId, checked));
    }

    @PutMapping("/{userId}/alerts/{alertId}")
    public ResponseEntity<Void> readUserAlert(
            @PathVariable Long userId,
            @PathVariable Long alertId) {
        userService.readAlertByUser(userId, alertId);
        return ResponseEntity.noContent().build();
    }

    @PutMapping("/{userId}/sensors")
    public ResponseEntity<PairSensorDto> assignSensorToUser(
            @PathVariable Long userId,
            @RequestBody AssignSensorToUserDto assignSensorToUserDto) {
        return ResponseEntity.ok(sensorService.assignSensorToUser(userId, assignSensorToUserDto));
    }

//    @GetMapping("/{userId}/sensors/{sensorId}")
//    public ResponseEntity<PairSensorDto> pairSensor(
//            @PathVariable Long userId,
//            @PathVariable Long sensorId) {
//        return ResponseEntity.ok(sensorService.pairSensor(userId, sensorId));
//    }

    @DeleteMapping("/{userId}/sensors/{sensorId}")
    public ResponseEntity<Boolean> disconnectSensorFromUser(
            @PathVariable Long userId,
            @PathVariable Long sensorId,
            @RequestBody DisconnectSensorFromUserDto disconnectSensorFromUserDto) {
        return ResponseEntity.ok(sensorService.disconnectSensorFromUser(userId, sensorId, disconnectSensorFromUserDto));
    }
}
