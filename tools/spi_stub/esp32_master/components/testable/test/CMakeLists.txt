set(src "Test_SPI.c")
set(include ".")
idf_component_register(SRCS "${src}"
                       INCLUDE_DIRS "${include}"
                       REQUIRES testable unity esp_driver_gpio)
