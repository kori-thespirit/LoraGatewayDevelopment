# ESP32-S3 Pin Mapping Documentation

Tài liệu này mô tả sơ đồ kết nối chân giữa ESP32-S3 và các module sử dụng trong hệ thống.

---

# 1. Strapping Pins (Boot Configuration)

Các chân này được đọc khi ESP32 khởi động, ảnh hưởng đến chế độ boot và cấu hình phần cứng.

| ESP32-S3 | FUNCTION |
|:---------|:---------|
| GPIO0    | Boot mode select |
| GPIO3    | JTAG source select |
| GPIO45   | Flash voltage select |
| GPIO46   | Boot mode / ROM log control |

---

# 2. LoRa Module (SPI)

Ví dụ module: SX1278 / Ra-02

| ESP32-S3 | LORA |
|:---------|:-----|
| GPIO40   | MISO |
| GPIO39   | SCK |
| GPIO41   | MOSI |
| GPIO42   | NSS (CS) |
| GPIO38   | RST |
| GPIO37   | DIO0 |

---

# 3. RS485 Module (UART)

Ví dụ module: MAX3485 / SP3485

| ESP32-S3 | RS485 |
|:---------|:------|
| GPIO17   | TX |
| GPIO18   | RX |
| GPIO16   | DE |
| GPIO15   | RE |

---

# 4. I2C Devices

Ví dụ: Sensor, LCD, EEPROM

| ESP32-S3 | I2C |
|:---------|:----|
| GPIO8    | SDA |
| GPIO9    | SCL |

---

# 5. SPI Devices (General)

| ESP32-S3 | SPI DEVICE |
|:---------|:-----------|
| GPIO12   | MISO |
| GPIO13   | MOSI |
| GPIO14   | SCK |
| GPIO10   | CS |

---

# 6. UART Debug / Programming

| ESP32-S3 | UART |
|:---------|:-----|
| GPIO43   | TXD0 |
| GPIO44   | RXD0 |

---

# 7. Control Signals

| ESP32-S3 | CONTROL |
|:---------|:--------|
| EN       | Chip enable / Reset |
| 3V3      | Power supply |
| GND      | Ground |

---

# 8. Important Notes

## Strapping Pins Warning

Không nên kéo các chân sau xuống mức LOW khi khởi động:

- GPIO0
- GPIO3
- GPIO45
- GPIO46

Sai cấu hình có thể gây:

- Không boot
- Không nạp code
- Treo hệ thống

## Recommended Pull-up / Pull-down

- GPIO0 → Pull-up 10kΩ
- EN → Pull-up 10kΩ

---

# 9. Project Information

## MCU

ESP32-S3

## Communication

- SPI
- UART
- RS485
- I2C

---

# End of Document