idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES modbus_application_driver esp_driver_gpio

                      )
