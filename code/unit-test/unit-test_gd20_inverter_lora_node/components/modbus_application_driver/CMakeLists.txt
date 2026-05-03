idf_component_register(SRCS "modbus_application_driver.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES esp_driver_uart esp_driver_gpio esp_timer
)


