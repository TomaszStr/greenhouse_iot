package com.greenhouse.greenhouse_iot.config;

import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;
import org.springframework.util.AntPathMatcher;

import java.util.Collections;
import java.util.Set;

@Slf4j
@Component
public class PublicEndpoints {
    @Value("${server.servlet.context-path}")
    private String serverServletContextPath;
    private final AntPathMatcher pathMatcher = new AntPathMatcher();
    private static final Set<String> PUBLIC_ENDPOINTS = Set.of(
            "/public/**",
            "/auth/login",
            "/users/register",
            "/v3/api-docs/**",
            "/swagger-ui/**",
            "/swagger-ui.html",
            "/users/*/sensors"

    );

    public Set<String> getEndpoints() {
        return Collections.unmodifiableSet(PUBLIC_ENDPOINTS);
    }

    public boolean isPublicEndpoint(String requestURI) {
        log.info("Check if request URI: {} is public", requestURI);

        String normalizedURI;

        if (serverServletContextPath != null && !serverServletContextPath.isBlank() &&
                requestURI.startsWith(serverServletContextPath)) {
            normalizedURI = requestURI.substring(serverServletContextPath.length());
        } else {
            normalizedURI = requestURI;
        }

        return PUBLIC_ENDPOINTS.stream().anyMatch(pattern -> pathMatcher.match(pattern, normalizedURI));
    }
}
