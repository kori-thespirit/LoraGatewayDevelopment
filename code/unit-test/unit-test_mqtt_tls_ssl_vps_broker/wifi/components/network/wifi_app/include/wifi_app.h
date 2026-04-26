#ifndef WIFI_APP_H
#define WIFI_APP_H
#define MAXIMUM_RETRY  5
#define WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD

void wifi_station_init();

#endif // WIFI_APP_H
