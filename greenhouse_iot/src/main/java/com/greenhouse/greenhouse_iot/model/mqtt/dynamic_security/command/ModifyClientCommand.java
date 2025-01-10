package com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.command;

import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.MqttCommand;
import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class ModifyClientCommand extends MqttCommand {
    String clientId;
    String password;
    public ModifyClientCommand(String clientId, String newPassword) {
        super.command = "modifyClient";
        this.clientId = clientId;
        this.password = newPassword;
    }
}

