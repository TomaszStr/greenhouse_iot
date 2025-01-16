package com.greenhouse.greenhouse_iot.exception;

import lombok.extern.slf4j.Slf4j;
import org.springframework.dao.DataIntegrityViolationException;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.authentication.BadCredentialsException;
import org.springframework.security.authorization.AuthorizationDeniedException;
import org.springframework.security.core.userdetails.UsernameNotFoundException;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;

import java.nio.file.AccessDeniedException;

@Slf4j
@RestControllerAdvice
public class GreenhouseExceptionHandler {
    @ExceptionHandler(ResourceNotFoundException.class)
    public ResponseEntity<GreenhouseErrorResponse> handleResourceNotFound(ResourceNotFoundException ex) {
        log.error("Resource not found: {}", ex.getMessage());
        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.NOT_FOUND, ex.getMessage());
        return ResponseEntity.status(HttpStatus.NOT_FOUND).body(errorResponse);
    }

    @ExceptionHandler(UsernameNotFoundException.class)
    public ResponseEntity<GreenhouseErrorResponse> handleUsernameNotFound(UsernameNotFoundException ex) {
        log.error("Username not found: {}", ex.getMessage());
        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.NOT_FOUND, ex.getMessage());
        return ResponseEntity.status(HttpStatus.NOT_FOUND).body(errorResponse);
    }

    @ExceptionHandler(IllegalArgumentException.class)
    public ResponseEntity<GreenhouseErrorResponse> handleInvalidInput(IllegalArgumentException ex) {
        log.error("Invalid input: {}", ex.getMessage());
        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.BAD_REQUEST, ex.getMessage());
        return ResponseEntity.status(HttpStatus.BAD_REQUEST).body(errorResponse);
    }

    @ExceptionHandler(GreenhouseAccessDeniedException.class)
    public ResponseEntity<GreenhouseErrorResponse> handleAuthenticationException(GreenhouseAccessDeniedException ex) {
        log.error("Authentication error: {}", ex.getMessage());
        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.UNAUTHORIZED, ex.getMessage());
        return ResponseEntity.status(HttpStatus.UNAUTHORIZED).body(errorResponse);
    }

    @ExceptionHandler(BadCredentialsException.class)
    public ResponseEntity<GreenhouseErrorResponse> handleBadCredentialsException(GreenhouseAccessDeniedException ex) {
        log.error("Authentication error: {}", ex.getMessage());
        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.UNAUTHORIZED, ex.getMessage());
        return ResponseEntity.status(HttpStatus.UNAUTHORIZED).body(errorResponse);
    }

    @ExceptionHandler(AuthenticationException.class)
    public ResponseEntity<GreenhouseErrorResponse> handleAuthenticationException(AuthenticationException ex) {
        log.error("Authentication error: {}", ex.getMessage());
        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.UNAUTHORIZED, ex.getMessage());
        return ResponseEntity.status(HttpStatus.UNAUTHORIZED).body(errorResponse);
    }

    @ExceptionHandler(AuthorizationDeniedException.class)
    public ResponseEntity<GreenhouseErrorResponse> handleAuthenticationException(AuthorizationDeniedException ex) {
        log.error("Authentication error: {}", ex.getMessage());
        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.UNAUTHORIZED, ex.getMessage());
        return ResponseEntity.status(HttpStatus.UNAUTHORIZED).body(errorResponse);
    }

    @ExceptionHandler(AccessDeniedException.class)
    public ResponseEntity<GreenhouseErrorResponse> handleAccessDenied(GreenhouseAccessDeniedException ex) {
        log.error("Access denied: {}", ex.getMessage());
        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.FORBIDDEN, ex.getMessage());
        return ResponseEntity.status(HttpStatus.FORBIDDEN).body(errorResponse);
    }

    @ExceptionHandler(DataIntegrityViolationException.class)
    public ResponseEntity<GreenhouseErrorResponse> handleDataIntegrityViolation(DataIntegrityViolationException ex) {
        log.error("Data integrity violation: {}", ex.getMessage());
        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.CONFLICT, "Data conflict occurred");
        return ResponseEntity.status(HttpStatus.CONFLICT).body(errorResponse);
    }

    @ExceptionHandler(Exception.class)
    public ResponseEntity<GreenhouseErrorResponse> handleGenericException(Exception ex) {
        log.error("An unexpected error occurred: {}", ex.getMessage(), ex);
        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.INTERNAL_SERVER_ERROR, "An unexpected error occurred");
        return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(errorResponse);
    }

}
