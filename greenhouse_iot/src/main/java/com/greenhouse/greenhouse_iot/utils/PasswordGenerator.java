package com.greenhouse.greenhouse_iot.utils;

import org.apache.commons.lang3.RandomStringUtils;
import org.springframework.stereotype.Service;

import java.security.SecureRandom;

@Service
public class PasswordGenerator {
    private final char[] possibleCharacters = ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789~`!@#$%^&*()-_=+[{]}\\|;:,<.>/?").toCharArray();

    public String getRandomPassword(int length){
        if (length <= 0) {
            throw new IllegalArgumentException("Password length must be greater than zero");
        }
        return RandomStringUtils.random( length, 0, possibleCharacters.length-1,
                false, false, possibleCharacters, new SecureRandom());
    }
}
