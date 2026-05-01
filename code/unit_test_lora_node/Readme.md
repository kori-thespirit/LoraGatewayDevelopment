# Hướng dẫn bắt đầu với Unit Test LoRa Node
Folder unit_test_lora_node được thiết kế để kiểm tra khả năng gửi dữ liệu từ ESP32 qua module LoRa SX127x.
## 1. Clone Project và Chuyển nhánh

### Clone toàn bộ dự án
```raw
git clone https://github.com/kori-thespirit/LoraGatewayDevelopment.git
cd LoraGatewayDevelopment
```
### Truy cập folder unit_test_lora_node
```raw
cd code
cd unit_test_lora_node
```
## 2. Nạp code 
```raw
idf.py set-target esp32
idf.py -p COM? build flash monitor
```