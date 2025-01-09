idf_component_register(SRCS "wifi_prod.c"
                        INCLUDE_DIRS "."
                        PRIV_REQUIRES driver esp_wifi esp_event nvs_flash)