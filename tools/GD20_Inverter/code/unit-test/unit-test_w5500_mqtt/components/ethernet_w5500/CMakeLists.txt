idf_component_register(SRCS "ethernet_w5500.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES esp_eth esp_netif espressif__ethernet_init mqtt
)


