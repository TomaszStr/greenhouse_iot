package com.greenhouse.greenhouse_iot.config;

import io.swagger.v3.oas.models.Components;
import io.swagger.v3.oas.models.OpenAPI;
import io.swagger.v3.oas.models.info.Contact;
import io.swagger.v3.oas.models.info.Info;
import io.swagger.v3.oas.models.security.SecurityRequirement;
import io.swagger.v3.oas.models.security.SecurityScheme;
import io.swagger.v3.oas.models.servers.Server;
import lombok.extern.slf4j.Slf4j;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import java.util.List;

@Slf4j
@Configuration
public class SwaggerConfig {

    @Bean
    public OpenAPI defineOpenApi() {
        log.info("Swagger available at: http://localhost:8080/api/swagger-ui/index.html");

        Server server = new Server();
        server.setUrl("http://localhost:8080/api");
        server.setDescription("Development");

        Contact myContact = new Contact();
        myContact.setName("student");
        myContact.setEmail("@student");

        Info information = new Info()
                .title("Greenhouse IoT System API")
                .version("0.1")
                .description("This API exposes endpoints to greenhouse IoT API.")
                .contact(myContact);

        SecurityScheme securityScheme = new SecurityScheme()
                .name("JWT")
                .type(SecurityScheme.Type.HTTP)
                .scheme("bearer")
                .bearerFormat("JWT");

        return new OpenAPI()
                .info(information)
                .servers(List.of(server))
                .addSecurityItem(new SecurityRequirement().addList("JWT"))
                .components(
                        new Components()
                                .addSecuritySchemes("JWT", securityScheme)
                );

    }
}