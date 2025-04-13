package com.greenhouse.greenhouse_iot.model.dto;

import com.greenhouse.greenhouse_iot.model.enums.Role;
import lombok.Data;

import java.util.Set;

@Data
public class UserDto {
    private Long id;
    private String username;
    private String firstName;
    private String lastName;
    private Set<Role> roles;
}
