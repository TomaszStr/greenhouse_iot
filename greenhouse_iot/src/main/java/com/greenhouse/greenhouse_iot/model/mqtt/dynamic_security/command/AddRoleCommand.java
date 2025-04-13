package com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.command;

import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.MqttCommand;
import lombok.Getter;
import lombok.Setter;

@Getter
@Setter
public class AddRoleCommand extends MqttCommand {
    private String roleName;
    private String username;

    public AddRoleCommand(String roleName, String username) {
        super.command = "addClientRole";
        this.roleName = roleName;
        this.username = username;
    }
}
