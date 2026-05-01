# Unit Test: ESP32 Wi-Fi Station
## 1. Chức năng
Test chức năng kết nối WiFi tới một mạng SSID nào đó
## 2. Hướng dẫn cấu hình WiFi
Thay đổi trực tiếp tại main/unit_test_wifi.c:
```raw
#define WIFI_SSID      "IoTVision_2.4GHz"
#define WIFI_PASS      "iotvision@2022"
#define MAXIMUM_RETRY  5
```

## 3. Thao tác nạp chương trình
```raw
idf.py set-target esp32s3
idf.py -p COM? build flash monitor
```
## 4. Kết quả đạt được
```raw
I (5002) wifi:dp: 1, bi: 102400, li: 3, scale listen interval from 307200 us to 307200 us
I (5022) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5032) wifi:<ba-add>idx:0 (ifx:0, 9a:25:4a:42:bf:2c), tid:7, ssn:2, winSize:64
I (5032) wifi:<ba-add>idx:1 (ifx:0, 9a:25:4a:42:bf:2c), tid:0, ssn:0, winSize:64
I (6022) esp_netif_handlers: sta ip: 192.168.0.51, mask: 255.255.255.0, gw: 192.168.0.1
I (6022) wifi_test: Đã nhận IP:192.168.0.51
I (6022) wifi_test: KẾT NỐI THÀNH CÔNG SSID:IoTVision_2.4GHz
```

