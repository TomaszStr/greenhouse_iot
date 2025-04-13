package com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.command;

import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.MqttCommand;
import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class MqttSetClientPassword extends MqttCommand {
    String username;
    String password;
    public MqttSetClientPassword(String username, String newPassword) {
        super.command = "setClientPassword";
        this.username = username;
        this.password = newPassword;
    }
}
