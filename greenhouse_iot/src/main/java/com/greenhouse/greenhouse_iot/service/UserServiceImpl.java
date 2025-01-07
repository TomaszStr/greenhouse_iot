package com.greenhouse.greenhouse_iot.service;

import com.greenhouse.greenhouse_iot.model.dto.UserDto;
import com.greenhouse.greenhouse_iot.model.dto.auth.RegistrationRequest;
import com.greenhouse.greenhouse_iot.model.entity.User;
import com.greenhouse.greenhouse_iot.exception.ResourceNotFoundException;
import com.greenhouse.greenhouse_iot.model.entity.Role;
import com.greenhouse.greenhouse_iot.model.mapper.UserMapper;
import com.greenhouse.greenhouse_iot.repository.UserRepository;
import lombok.AllArgsConstructor;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;

import java.util.HashSet;
import java.util.List;

@Service
@AllArgsConstructor
public class UserServiceImpl implements UserService{
    private final UserRepository userRepository;
    private final UserMapper userMapper;
    private final PasswordEncoder passwordEncoder;
    @Override
    public List<UserDto> getUsers() {
        return userRepository.findAll().stream().map(userMapper::userToUserDto).toList();
    }

    @Override
    public UserDto getUserById(Long userId) {
        return userMapper.userToUserDto(userRepository.findById(userId).orElseThrow(() -> new ResourceNotFoundException("No user exists with id: "+userId)));
    }

    @Override
    public UserDto registerUser(RegistrationRequest registrationRequest) {
        if (userRepository.findByUsername(registrationRequest.getUsername()).isPresent()) {
            throw new IllegalArgumentException("Username is already taken");
        }

        User user = new User();
        user.setUsername(registrationRequest.getUsername());
        user.setPassword(passwordEncoder.encode(registrationRequest.getPassword()));
        user.setFirstName(registrationRequest.getFirstName());
        user.setLastName(registrationRequest.getLastName());
        user.setRoles(new HashSet<>(List.of(new Role[]{Role.USER})));

        user = userRepository.save(user);
        return userMapper.userToUserDto(user);
    }

    @Override
    public void deleteUserById(Long id) {
        userRepository.deleteById(id);
    }

    @Override
    public UserDto editUser(UserDto user) {
        User userToEdit = userRepository.findById(user.getId()).orElse(null);
        if(userToEdit == null)
            return null;
//        userToEdit = userMapper.userDtoToUser(user);
//        TODO modify user values
        userRepository.save(userToEdit);
        return  userMapper.userToUserDto(userToEdit);
    }
}
