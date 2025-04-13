package com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.command;

import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.MqttCommand;
import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.MqttRole;
import lombok.Getter;
import lombok.Setter;

import java.util.List;

@Getter
@Setter
public class CreateClientCommand extends MqttCommand {
//    private String clientId;
//    private String roleName;
    private String username;
    private String password;
    private List<MqttRole> roles;

    public CreateClientCommand(String clientId, String roleName, String username, String password, List<MqttRole> roles) {
        super.command = "createClient";
//        this.clientId = clientId;
        this.username = username;
        this.password = password;
//        this.roleName = roleName;
        this.roles = roles;
    }
}
