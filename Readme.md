# Hướng dẫn bắt đầu với Unit Test LoRa Node
Nhánh branch_unit_test_lora_node được thiết kế để kiểm tra khả năng gửi dữ liệu từ ESP32 qua module LoRa SX127x. Hãy làm theo các bước sau để thiết lập.
## 1. Clone Project và Chuyển nhánh
Đầu tiên, bạn cần clone repository và truy cập đúng nhánh unit test dành cho Node:

### Clone toàn bộ dự án
```raw
git clone https://github.com/kori-thespirit/LoraGatewayDevelopment.git
cd LoraGatewayDevelopment
```
### Chuyển sang nhánh unit test gửi dữ liệu
```raw
git checkout branch_unit_test_lora_node
```
## 2. Cấu hình thông số qua Menuconfig
Sau khi đã ở trong nhánh phù hợp, bạn thực hiện lệnh cấu hình để khớp với phần cứng hiện có.
```raw
idf.py set-target esp32
idf.py menuconfig
```
## 3.Configuration for Receiver
<img width="702" height="346" alt="image" src="https://github.com/user-attachments/assets/ebe839d3-d548-4229-b9b3-05286996cfe8" />
<img width="720" height="225" alt="image" src="https://github.com/user-attachments/assets/c6c4ec39-3547-4795-ace9-593bbe17e0dc" />
<img width="299" height="167" alt="image" src="https://github.com/user-attachments/assets/f79a5ba2-dc6d-459f-9ac6-fae6bf697a9f" />
<img width="609" height="328" alt="image" src="https://github.com/user-attachments/assets/5afdc329-9f9e-46d8-b2bd-ad6b27b7ffca" />
<img width="609" height="328" alt="image" src="https://github.com/user-attachments/assets/12dace2f-4765-498d-924f-8e01b8aad030" />
<img width="627" height="187" alt="image" src="https://github.com/user-attachments/assets/1dc2a1a7-9a20-4f9e-8a83-40ca4eb89373" />
<img width="700" height="190" alt="image" src="https://github.com/user-attachments/assets/386239e7-0d73-45eb-ad35-3c4c61a8cb6a" />









