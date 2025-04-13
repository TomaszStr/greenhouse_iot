package com.greenhouse.greenhouse_iot.repository;

import com.greenhouse.greenhouse_iot.model.dto.sensor.SensorDto;
import com.greenhouse.greenhouse_iot.model.entity.Sensor;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;

import java.util.List;

public interface SensorRepository extends JpaRepository<Sensor, Long> {
    @Query(value = "SELECT s FROM sensors s WHERE s.sensorMqttName = :mqttName", nativeQuery = false)
    Sensor findByMqttName(@Param("mqttName") String mqttName);

    @Query(value = "SELECT s FROM sensors s WHERE s.sensorCode = :sensorCode", nativeQuery = false)
    Sensor findBySensorCode(@Param("sensorCode") String sensorCode);

    @Query(value = "SELECT s from sensors s WHERE s.user.id = :userId", nativeQuery = false)
    List<Sensor> findSensorsByUserId(@Param("userId") Long userId);
}
