package com.greenhouse.greenhouse_iot.model.dto.auth;

import lombok.Data;

@Data
public class LoginRequest {
    private String username;
    private String password;
}
