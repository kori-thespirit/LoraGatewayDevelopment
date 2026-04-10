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
## 3.Configuration for Receiver
<img width="702" height="346" alt="image" src="https://github.com/user-attachments/assets/ebe839d3-d548-4229-b9b3-05286996cfe8" />
<img width="720" height="225" alt="image" src="https://github.com/user-attachments/assets/c6c4ec39-3547-4795-ace9-593bbe17e0dc" />
<img width="299" height="167" alt="image" src="https://github.com/user-attachments/assets/f79a5ba2-dc6d-459f-9ac6-fae6bf697a9f" />
<img width="609" height="328" alt="image" src="https://github.com/user-attachments/assets/5afdc329-9f9e-46d8-b2bd-ad6b27b7ffca" />






