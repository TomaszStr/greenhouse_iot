package com.greenhouse.greenhouse_iot.model.mapper;

import com.greenhouse.greenhouse_iot.model.dto.UserDto;
import com.greenhouse.greenhouse_iot.model.entity.User;
import org.mapstruct.Mapper;

@Mapper(componentModel = "spring")
public interface UserMapper {
    UserDto userToUserDto(User user);

//    User userDtoToUser(UserDto userDto);
}
