# Hướng dẫn bắt đầu với Unit Test LoRa Gateway
Nhánh branch_unit_test_lora_gateway được thiết kế để kiểm tra khả năng gửi dữ liệu từ ESP32 qua module LoRa SX127x. Hãy làm theo các bước sau để thiết lập.
## 1. Clone Project và Chuyển nhánh
Đầu tiên, bạn cần clone repository và truy cập đúng nhánh unit test dành cho Gateway:

### Clone toàn bộ dự án
```raw
git clone https://github.com/kori-thespirit/LoraGatewayDevelopment.git
cd LoraGatewayDevelopment
```
### Chuyển sang nhánh unit test gửi dữ liệu
```
git checkout branch_unit_test_lora_gateway
```
## 2. Cấu hình thông số qua Menuconfig
Sau khi đã ở trong nhánh phù hợp, bạn thực hiện lệnh cấu hình để khớp với phần cứng hiện có.
```raw
idf.py set-target esp32s3
idf.py menuconfig
```
## 3. Configuration for Receiver
### 1. Application Configuration -> (✅)As the receiver 
### 2. Lora Configuration 
| ESP32 | LORA |
| :--- | :--- | :--- | :--- | 
| GPIO 40 | MISO GPIO |
| GPIO 39 | SCK GPIO |
| GPIO 41 | MOSI GPIO |
| GPIO 42 | NSS GPIO |
| GPIO 38 | RST GPIO |
## 4. Flash to ESP32
```
idf.py -p COM? build flash monitor
```







