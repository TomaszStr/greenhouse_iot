package com.greenhouse.greenhouse_iot.service;

import com.greenhouse.greenhouse_iot.exception.GreenhouseAccessDeniedException;
import com.greenhouse.greenhouse_iot.exception.AuthenticationException;
import com.greenhouse.greenhouse_iot.model.entity.Sensor;
import com.greenhouse.greenhouse_iot.model.entity.User;
import com.greenhouse.greenhouse_iot.repository.UserRepository;
import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.stereotype.Service;

import java.util.Objects;

@Slf4j
@AllArgsConstructor
@Service
public class SecurityService {

    private final UserRepository userRepository;

    /**
     * Retrieves the currently logged-in user from the security context.
     *
     * @return the logged-in user
     * @throws AuthenticationException if no user is authenticated
     */
    public User getLoggedInUserOrThrow() {
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();

        if (authentication == null || !(authentication.getPrincipal() instanceof User user)) {
            throw new AuthenticationException("No user is logged in.");
        }

        log.info("Logged in user: {}, id: {}", user.getUsername(), user.getId());

        return user;
    }

    /**
     * Checks if the provided user is the owner of the sensor.
     *
     * @param loggedInUser the logged-in user
     * @param sensor the sensor to check ownership for
     * @throws GreenhouseAccessDeniedException if the user is not the owner
     */
    public void checkSensorOwnership(User loggedInUser, Sensor sensor) {
        if (!isOwner(loggedInUser, sensor)) {
            throw new GreenhouseAccessDeniedException("The logged-in user is not the owner of the sensor.");
        }
    }

    /**
     * Checks if the provided user is the owner of the sensor.
     *
     * @param user the user to check
     * @param sensor the sensor to check ownership for
     * @return true if the user is the owner, otherwise false
     */
    public boolean isOwner(User user, Sensor sensor) {
        if(user == null || sensor == null || sensor.getUser() == null)
            return false;
        return Objects.equals(user.getId(), sensor.getUser().getId());
    }

    /**
     * Checks if the logged-in user matches the provided userId.
     *
     * @param loggedInUserId the ID of the logged-in user
     * @param providedUserId the provided user ID to match
     * @throws GreenhouseAccessDeniedException if the user IDs do not match
     */
    public void checkUserIdMatch(Long loggedInUserId, Long providedUserId) {
        if (!Objects.equals(loggedInUserId, providedUserId)) {
            throw new GreenhouseAccessDeniedException("The logged-in user ID does not match the provided user ID.");
        }
    }

    /**
     * Checks if the logged-in user has the "ADMIN" role.
     *
     * @param loggedInUser the logged-in user
     * @return true if the user has the "ADMIN" role, otherwise false
     */
    public boolean isAdmin(User loggedInUser) {
        return loggedInUser.getRoles().contains("ADMIN");
    }
}
