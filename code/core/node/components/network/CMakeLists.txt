idf_component_register(
    SRCS "mqtts/mqtts_app.c" "wifi_app/wifi_app.c"
    INCLUDE_DIRS "include" "mqtts/include/" "wifi_app/include/"
    # esp_wifi nvs_flash are required for WiFi module
    # mqtt is required for MQTT stack
    # esp_driver_gpio esp_driver_spi esp_eth esp_netif espressif__ethernet_init are required for Ethernet W5500
    REQUIRES esp_wifi nvs_flash 
    mqtt 
)





