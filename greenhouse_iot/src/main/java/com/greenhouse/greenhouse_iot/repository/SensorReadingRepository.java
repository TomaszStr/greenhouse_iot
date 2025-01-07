package com.greenhouse.greenhouse_iot.repository;

import com.greenhouse.greenhouse_iot.model.dto.SensorReadingDto;
import com.greenhouse.greenhouse_iot.model.entity.SensorReading;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;

import java.time.LocalDateTime;
import java.util.List;

public interface SensorReadingRepository extends JpaRepository<SensorReading, Long> {
    // Custom query to find sensor readings by sensor ID within a time range
    @Query("SELECT s FROM sensor_readings s WHERE s.sensor.id = :sensorId AND s.timestamp BETWEEN :startTime AND :endTime")
    List<SensorReading> findSensorReadingsBySensorId(
            @Param("sensorId") Long sensorId,
            @Param("startTime") LocalDateTime startTime,
            @Param("endTime") LocalDateTime endTime);
}
