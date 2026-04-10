# Hướng dẫn bắt đầu với Unit Test LoRa Node
Nhánh branch_unit_test_lora_node được thiết kế để kiểm tra khả năng gửi dữ liệu từ ESP32 qua module LoRa SX127x. Hãy làm theo các bước sau để thiết lập.
## 1. Clone Project và Chuyển nhánh
Đầu tiên, bạn cần clone repository và truy cập đúng nhánh unit test dành cho Node:

### Clone toàn bộ dự án
```
git clone https://github.com/kori-thespirit/LoraGatewayDevelopment.git
cd LoraGatewayDevelopment
```
### Chuyển sang nhánh unit test gửi dữ liệu
```
git checkout branch_unit_test_lora_node
```
## 2. Cấu hình thông số qua Menuconfig
Sau khi đã ở trong nhánh phù hợp, bạn thực hiện lệnh cấu hình để khớp với phần cứng hiện có.
```
idf.py set-target esp32
idf.py menuconfig
```
## 3.Configuration for Transceiver
![Alt text](C:/Users/ADMIN/Pictures/Screenshots/Screenshot 2026-04-10 162103.png)

