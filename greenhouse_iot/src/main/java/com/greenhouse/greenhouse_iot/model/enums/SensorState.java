package com.greenhouse.greenhouse_iot.model.enums;

public enum SensorState {
    NORMAL("NORMAL"),
    FORCED("FORCED"),
    SLEEP("SLEEP");
    private final String state;
    SensorState(String state) {
        this.state = state;
    }

    public String getState() {
        return state;
    }
}
