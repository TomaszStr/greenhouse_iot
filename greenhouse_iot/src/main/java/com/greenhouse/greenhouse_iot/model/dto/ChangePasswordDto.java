package com.greenhouse.greenhouse_iot.model.dto;

import lombok.Data;

@Data
public class ChangePasswordDto {
    private String oldPassword;
    private String newPassword;
    private String repeatedNewPassword;
}
