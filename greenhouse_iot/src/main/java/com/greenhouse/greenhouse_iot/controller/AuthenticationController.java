package com.greenhouse.greenhouse_iot.controller;

import com.greenhouse.greenhouse_iot.model.dto.auth.AuthenticationResponse;
import com.greenhouse.greenhouse_iot.model.dto.auth.LoginRequest;
import com.greenhouse.greenhouse_iot.model.entity.User;
import com.greenhouse.greenhouse_iot.service.GreenhouseIotUserDetailsService;
import com.greenhouse.greenhouse_iot.utils.JwtUtil;
import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.ResponseEntity;
import org.springframework.security.authentication.AuthenticationManager;
import org.springframework.security.authentication.BadCredentialsException;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.Objects;

@AllArgsConstructor
@Slf4j
@RestController
@RequestMapping("/auth")
public class AuthenticationController {

    private final AuthenticationManager authenticationManager;
    private final JwtUtil jwtUtil;
    private final GreenhouseIotUserDetailsService userDetailsService;

    @PostMapping("/login")
    public ResponseEntity<?> createAuthenticationToken(@RequestBody LoginRequest loginRequest) {
        try {
            log.info("Authenticating login request");
            authenticationManager.authenticate(
                    new UsernamePasswordAuthenticationToken(loginRequest.getUsername(), loginRequest.getPassword()));
        } catch (BadCredentialsException e) {
            log.error("Exception - bad credentials: " + e.getMessage());
            throw new BadCredentialsException("Incorrect username or password", e);
        }

        final UserDetails userDetails = userDetailsService.loadUserByUsername(loginRequest.getUsername());
        final String jwt = jwtUtil.generateToken(userDetails);

        User user = (User) userDetails;
        log.info("Authentication successful, logged user: {}, id: {} roles: {}",
                user.getUsername(), user.getId(), user.getRoles());

        return ResponseEntity.ok(new AuthenticationResponse(jwt, user.getId(), user.getUsername(), user.getRoles()));
    }
}
