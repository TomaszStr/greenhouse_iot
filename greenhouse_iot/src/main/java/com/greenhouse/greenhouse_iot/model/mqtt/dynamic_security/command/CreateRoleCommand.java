package com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.command;

import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.MqttCommand;
import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.MqttAcl;
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
