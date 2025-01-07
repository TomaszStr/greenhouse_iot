package com.greenhouse.greenhouse_iot.model.mqtt;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

@Getter
@Setter
public class CreateClientCommand extends MqttCommand {
    private String clientId;
    private String roleName;
    private String username;
    private String password;
    private List<String> roles;

    public CreateClientCommand(String clientId, String roleName, String username, String password, List<String> roles) {
        super.command = "createClient";
        this.clientId = clientId;
        this.roleName = roleName;
        this.username = username;
        this.password = password;
        this.roles = roles;
    }
}
