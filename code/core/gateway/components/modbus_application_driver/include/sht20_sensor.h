#ifndef SHT20_SENSOR_H_
#define SHT20_SENSOR_H_

#define SHT20_SLAVE_ID              0x08
#define SHT20_RESP_CODE_FAULT       0x84    // Lỗi lệnh 0x04 trả về 0x84, lệnh 0x03 trả về 0x83

/* ========================================================================= */
/* ĐĂNG KÝ ĐẦU VÀO (INPUT REGISTERS - FUNCTION CODE 0x04)                    */
/* Các thanh ghi chỉ đọc (Read-Only) cập nhật liên tục theo thời gian thực   */
/* ========================================================================= */
#define SHT20_REG_TEMP              0x0001  // Giá trị nhiệt độ (Kết quả chia cho 10.0)
#define SHT20_REG_HUMID             0x0002  // Giá trị độ ẩm (Kết quả chia cho 10.0)

/* ========================================================================= */
/* GIỮ SỔ ĐĂNG KÝ (HOLDING REGISTERS - FUNCTION CODE 0x03 / 0x06)             */
/* Các thanh ghi đọc/ghi (Read-Write) cấu hình thông số hệ thống             */
/* ========================================================================= */
#define SHT20_REG_DEVICE_ADDR       0x0101  // Địa chỉ thiết bị Slave ID (Dải cài đặt: 1 ~ 247)
#define SHT20_REG_BAUDRATE          0x0102  // Tốc độ truyền (0: 9600, 1: 14400, 2: 19200)
#define SHT20_REG_TEMP_CALIB        0x0103  // Giá trị hiệu chỉnh nhiệt độ (Dải: -10.0 ~ 10.0)
#define SHT20_REG_HUMID_CALIB       0x0104  // Giá trị điều chỉnh độ ẩm (Dải: -10.0 ~ 10.0)

#endif /* SHT20_SENSOR_H_ */