idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    # REQUIRES mqtt
                    # PRIV_REQUIRES wifi_prod ble_prod bme280 capacitive_soil_moisture_sensor temt6000 diode
                    )

