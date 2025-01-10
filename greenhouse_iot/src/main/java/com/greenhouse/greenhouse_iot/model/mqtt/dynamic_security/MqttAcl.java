package com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
@AllArgsConstructor
public class MqttAcl {
    String aclType;
    String topic;
    Boolean allow;
}
