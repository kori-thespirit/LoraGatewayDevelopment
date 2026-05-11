idf_component_register(SRCS "main.c" "lora_task.c" "modbus_task.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_driver_gpio lora modbus_application_driver lora_protocol
                    )
