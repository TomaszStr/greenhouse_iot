package com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.command;

import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.MqttCommand;
import com.greenhouse.greenhouse_iot.model.mqtt.dynamic_security.MqttAcl;
import lombok.Getter;
import lombok.Setter;

import java.util.List;

@Getter
@Setter
public class UpdateRoleCommand extends MqttCommand {
    private String roleName;
    private List<MqttAcl> acls;

    public UpdateRoleCommand(String roleName, List<MqttAcl> acls) {
        super.command = "updateRole";
        this.roleName = roleName;
        this.acls = acls;
    }
}
