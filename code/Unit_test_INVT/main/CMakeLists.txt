set(PROJECT_NAME "modbus_serial_master")

    idf_component_register(SRCS "serial_master.c"
                            #"modbus_params.c"
                        INCLUDE_DIRS "."
                        PRIV_REQUIRES nvs_flash esp_driver_uart esp_driver_gpio 
                        )

# Workaround to avoid static analysis false positives for some components.
# if(CONFIG_FMB_COMPILER_STATIC_ANALYZER_ENABLE AND CMAKE_C_COMPILER_ID STREQUAL "GNU")
#     target_compile_options(${COMPONENT_LIB} PRIVATE "-fanalyzer")
#     message(STATUS "Static analyzer build for ${PROJECT_NAME}.")
# endif()
