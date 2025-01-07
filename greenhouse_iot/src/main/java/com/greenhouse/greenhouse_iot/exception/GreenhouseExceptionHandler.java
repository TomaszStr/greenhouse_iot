package com.greenhouse.greenhouse_iot.exception;

import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;

@Slf4j
@RestControllerAdvice
public class GreenhouseExceptionHandler {
    @ExceptionHandler(ResourceNotFoundException.class)
    public ResponseEntity<GreenhouseErrorResponse> handleResourceNotFound(ResourceNotFoundException ex) {
        log.error("Resource not found: {}", ex.getMessage());
        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.NOT_FOUND, ex.getMessage());
        return ResponseEntity.status(HttpStatus.NOT_FOUND).body(errorResponse);
    }

//    @ExceptionHandler(InvalidInputException.class)
//    public ResponseEntity<GreenhouseErrorResponse> handleInvalidInput(InvalidInputException ex) {
//        log.error("Invalid input: {}", ex.getMessage());
//        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.BAD_REQUEST, ex.getMessage());
//        return ResponseEntity.status(HttpStatus.BAD_REQUEST).body(errorResponse);
//    }

    @ExceptionHandler(AuthenticationException.class)
    public ResponseEntity<GreenhouseErrorResponse> handleAuthenticationException(AuthenticationException ex) {
        log.error("Authentication error: {}", ex.getMessage());
        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.UNAUTHORIZED, ex.getMessage());
        return ResponseEntity.status(HttpStatus.UNAUTHORIZED).body(errorResponse);
    }

    @ExceptionHandler(AccessDeniedException.class)
    public ResponseEntity<GreenhouseErrorResponse> handleAccessDenied(AccessDeniedException ex) {
        log.error("Access denied: {}", ex.getMessage());
        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.FORBIDDEN, ex.getMessage());
        return ResponseEntity.status(HttpStatus.FORBIDDEN).body(errorResponse);
    }

//    @ExceptionHandler(DataIntegrityViolationException.class)
//    public ResponseEntity<GreenhouseErrorResponse> handleDataIntegrityViolation(DataIntegrityViolationException ex) {
//        log.error("Data integrity violation: {}", ex.getMessage());
//        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.CONFLICT, "Data conflict occurred");
//        return ResponseEntity.status(HttpStatus.CONFLICT).body(errorResponse);
//    }

    @ExceptionHandler(Exception.class)
    public ResponseEntity<GreenhouseErrorResponse> handleGenericException(Exception ex) {
        log.error("An unexpected error occurred: {}", ex.getMessage(), ex);
        GreenhouseErrorResponse errorResponse = new GreenhouseErrorResponse(HttpStatus.INTERNAL_SERVER_ERROR, "An unexpected error occurred");
        return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(errorResponse);
    }

}
