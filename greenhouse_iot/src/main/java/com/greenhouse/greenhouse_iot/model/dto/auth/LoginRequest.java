package com.greenhouse.greenhouse_iot.model.dto.auth;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;
import lombok.Data;

@Data
public class LoginRequest {
    @NotBlank
    @Size(min = 3, max = 50, message = "Username must be between 3 and 50 characters")
    private String username;
    @NotBlank
    @Size(min = 8, max = 50, message = "Username must be between 3 and 50 characters")
    private String password;
}
