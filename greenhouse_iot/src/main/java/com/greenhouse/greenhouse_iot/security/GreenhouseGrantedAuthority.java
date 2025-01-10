package com.greenhouse.greenhouse_iot.security;

import com.greenhouse.greenhouse_iot.model.enums.Role;
import org.springframework.security.core.GrantedAuthority;

public class GreenhouseGrantedAuthority implements GrantedAuthority {
    private final String authority;

    public GreenhouseGrantedAuthority(Role role) {
        if(role != null && !role.name().isBlank()) {
            this.authority = String.format("ROLE_%s", role.name());
        }
        else {
            throw new IllegalArgumentException("Greenhouse granted authority can't be empty string or null");
        }
    }

    @Override
    public String getAuthority() {
        return authority;
    }
}
