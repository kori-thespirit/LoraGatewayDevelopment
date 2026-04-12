# Hướng dẫn bắt đầu với Unit Test Wifi
Nhánh SCRUM-37-unit-test-wi-fi-module được thiết kế để kiểm tra khả năng kết nối Wifi. Hãy làm theo các bước sau để thiết lập.
## 1. Clone Project và Chuyển nhánh
Đầu tiên, bạn cần clone repository và truy cập đúng nhánh unit test dành cho Wifi:

### Clone toàn bộ dự án
```raw
git clone https://github.com/kori-thespirit/LoraGatewayDevelopment.git
cd LoraGatewayDevelopment
```
### Chuyển sang nhánh unit test gửi dữ liệu
```raw
git checkout SCRUM-37-unit-test-wi-fi-module
```
## 2. Cấu hình thông số qua Menuconfig
Sau khi đã ở trong nhánh phù hợp, bạn thực hiện lệnh cấu hình để khớp với phần cứng hiện có.
```raw
idf.py set-target esp32s3
```
## 3. Flash to ESP32
```raw
idf.py -p COM? build flash monitor
```
