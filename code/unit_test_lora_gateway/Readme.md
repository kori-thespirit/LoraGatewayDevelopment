# Hướng dẫn bắt đầu với Unit Test LoRa Gateway
Folder unit_test_lora_gateway được thiết kế để kiểm tra khả năng gửi dữ liệu từ ESP32 qua module LoRa SX127x.
## 1. Clone Project và Chuyển nhánh

### Clone toàn bộ dự án
```raw
git clone https://github.com/kori-thespirit/LoraGatewayDevelopment.git
cd LoraGatewayDevelopment
```
### Truy cập folder unit_test_lora_gateway
```raw
cd code
cd unit_test_lora_gateway
```
## 2. Nạp code 

```raw
idf.py set-target esp32s3
idf.py -p COM? build flash monitor
```