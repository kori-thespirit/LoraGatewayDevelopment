idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES ethernet_w5500 esp_driver_gpio esp_eth esp_netif

                      )
