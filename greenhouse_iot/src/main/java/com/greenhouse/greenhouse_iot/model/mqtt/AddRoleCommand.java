package com.greenhouse.greenhouse_iot.model.mqtt;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

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
