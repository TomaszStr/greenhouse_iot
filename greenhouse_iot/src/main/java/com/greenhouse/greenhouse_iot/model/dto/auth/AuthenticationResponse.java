package com.greenhouse.greenhouse_iot.model.dto.auth;

import com.greenhouse.greenhouse_iot.model.enums.Role;
import lombok.AllArgsConstructor;
import lombok.Data;

import java.util.Set;

@Data
@AllArgsConstructor
public class AuthenticationResponse {
    private String jwt;
    private Long userId;
    private String username;
    private Set<Role> userRoles;
}
