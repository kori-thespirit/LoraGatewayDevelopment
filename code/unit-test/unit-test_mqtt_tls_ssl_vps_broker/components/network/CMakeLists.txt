idf_component_register(
    SRCS "mqtts/mqtts_app.c" "wifi_app/wifi_app.c"
    INCLUDE_DIRS "include" "mqtts/include/" "wifi_app/include/"
    REQUIRES esp_wifi mqtt nvs_flash
)


