package com.greenhouse.greenhouse_iot.model.entity;

import com.greenhouse.greenhouse_iot.security.GreenhouseGrantedAuthority;
import jakarta.persistence.*;
import lombok.*;
import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.userdetails.UserDetails;

import java.util.Collection;
import java.util.Set;

@Getter
@Setter
@Builder
@NoArgsConstructor
@AllArgsConstructor
@Entity(name = "greenhouse_users")
public class User implements UserDetails {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(name = "username", unique = true, nullable = false)
    private String username;

    @Column(name = "password", nullable = false)
    private String password;

    @Column(name = "first_name")
    private String firstName;

    @Column(name = "last_name")
    private String lastName;

    @OneToMany(mappedBy = "user")
    private Set<Sensor> sensors;

    @Enumerated(EnumType.STRING)
    private Set<Role> roles;

    @Override
    public Collection<? extends GrantedAuthority> getAuthorities() {
        System.out.println(roles.stream().map(Enum::name).toList());
        System.out.println(roles.stream().map(GreenhouseGrantedAuthority::new).toList());
        return roles.stream().map(GreenhouseGrantedAuthority::new).toList();
    }

    @Override
    public String getPassword() {
        return password;
    }
}
