set(src "STM32_Slave.c")
set(include "include")
idf_component_register(SRCS "${src}"
                       INCLUDE_DIRS "${include}"
                       REQUIRES esp_driver_spi esp_driver_gpio)
