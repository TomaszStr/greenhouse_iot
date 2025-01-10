package com.greenhouse.greenhouse_iot.model.enums;

import lombok.Getter;

@Getter
public enum SensorState {
    NORMAL(0),
    FORCED(1),
    SLEEP(2);

    private final Integer code;

    private SensorState(Integer code) {
        this.code = code;
    }
}
