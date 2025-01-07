package com.greenhouse.greenhouse_iot.model.mqtt;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

@Getter
@Setter
public class CreateRoleCommand extends MqttCommand {
    private String roleName;
    private List<MqttAcl> acls;

    public CreateRoleCommand(String roleName, List<MqttAcl> acls) {
        super.command = "createRole";
        this.roleName = roleName;
        this.acls = acls;
    }
}
