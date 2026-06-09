idf_component_register(SRCS "main.cpp" "lora_task.c" "daemon_task.c" "network_task.c" "common_task.c"  "hmi_task.cpp"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES network lora rtc_ds3231 sdcard lora_protocol 
modbus_application_driver

)
