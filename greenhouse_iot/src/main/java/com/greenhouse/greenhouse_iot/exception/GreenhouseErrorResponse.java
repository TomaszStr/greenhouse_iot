package com.greenhouse.greenhouse_iot.exception;

import lombok.AllArgsConstructor;
import lombok.Data;
import org.springframework.http.HttpStatusCode;

@Data
@AllArgsConstructor
public class GreenhouseErrorResponse { // TODO - IMPLEMENT Spring ErrorResponse
    private HttpStatusCode statusCode;
    private String body;

}
