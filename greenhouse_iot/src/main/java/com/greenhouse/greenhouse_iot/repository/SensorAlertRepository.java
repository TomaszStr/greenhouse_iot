package com.greenhouse.greenhouse_iot.repository;

import com.greenhouse.greenhouse_iot.model.entity.SensorAlert;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;

import java.util.List;

public interface SensorAlertRepository extends JpaRepository<SensorAlert, Long> {
    @Query("""
        SELECT sa
        FROM sensor_alerts sa
        WHERE sa.user.id = :userId AND (:checked IS NULL OR sa.checked = :checked)
    """)
    List<SensorAlert> getAlertsForUser(Long userId, Boolean checked);
}
