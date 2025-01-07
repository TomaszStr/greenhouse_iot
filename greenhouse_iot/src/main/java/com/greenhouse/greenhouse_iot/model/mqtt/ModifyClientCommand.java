package com.greenhouse.greenhouse_iot.model.mqtt;

import com.greenhouse.greenhouse_iot.model.dto.MqttCredentials;
import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class ModifyClientCommand extends MqttCommand {
    String clientId;
    String newPassword;
    public ModifyClientCommand(String clientId, String newPassword) {
        super.command = "modifyClient";
        this.clientId = clientId;
        this.newPassword = newPassword;
    }
}
