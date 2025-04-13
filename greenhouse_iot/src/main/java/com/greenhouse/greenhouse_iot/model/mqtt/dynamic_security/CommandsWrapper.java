package com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security;

import lombok.AllArgsConstructor;
import lombok.Data;

import java.util.List;

@Data
@AllArgsConstructor
public class CommandsWrapper {
    private List<MqttCommand> commands;
}
