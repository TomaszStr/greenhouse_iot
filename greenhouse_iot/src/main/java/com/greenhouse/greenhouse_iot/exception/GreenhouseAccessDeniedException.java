package com.greenhouse.greenhouse_iot.exception;

import org.springframework.security.access.AccessDeniedException;

public class GreenhouseAccessDeniedException extends AccessDeniedException {
    public GreenhouseAccessDeniedException(String message) {
        super(message);
    }
}
