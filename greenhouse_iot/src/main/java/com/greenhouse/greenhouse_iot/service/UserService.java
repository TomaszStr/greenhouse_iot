package com.greenhouse.greenhouse_iot.service;

import com.greenhouse.greenhouse_iot.model.dto.ChangePasswordDto;
import com.greenhouse.greenhouse_iot.model.dto.UserDto;
import com.greenhouse.greenhouse_iot.model.dto.auth.RegistrationRequest;
import com.greenhouse.greenhouse_iot.model.entity.User;

import java.util.List;

public interface UserService {
    List<UserDto> getUsers();
    UserDto getUserById(Long id);
    UserDto registerUser(RegistrationRequest registrationRequest);
    void changePassword(Long userId, ChangePasswordDto changePasswordDto);
    void deleteUserById(Long id);
    UserDto editUser(UserDto user);
}
