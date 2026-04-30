#ifndef GD20_INVERTER_H
#define GD20_INVERTER_H

// GD20 registers
#define GD20_SLAVE_ID                       0x01
#define GD20_RESP_CODE_FAULT                0x80
#define GD20_REG_CONTROL_CMD                0x2000  // Thanh ghi lệnh chạy/dừng
#define GD20_REG_SET_FREQ                   0x2001  // Thanh ghi đặt tần số
#define GD20_REG_SET_TORQUE                 0x2004  // Thanh ghi đặt moment
#define GD20_REG_SET_LIMIT_FREQ_FWD         0x2005  // Thanh ghi đặt tần số giới hạn chạy thuận
#define GD20_REG_SET_LIMIT_FREQ_REV         0x2006  // Thanh ghi đặt tần số giới hạn chạy nghịch
#define GD20_REG_SET_LIMIT_TORQUE           0x2007  // Thanh ghi đặt moment giới hạn
#define GD20_REG_SET_LIMIT_TORQUE_BRAKE     0x2008  // Thanh ghi đặt moment giới hạn khi phanh

#define GD20_REG_STATUS                     0x2100  // SW1 (Stage Word) - Trạng thái hoạt động
#define GD20_REG_ERROR                      0x2102  // Hiển thị lỗi (Error Code)
#define GD20_REG_ID                         0x2103  // GD20 ID code

#define GD20_OPERATION_FREQ                 0x3000  // Tần số thực tế đang chạy (Actual Frequency)
#define GD20_SET_FREQ                       0x3001  // Tần số đặt (Set Frequency)
#define GD20_BUS_VOLTAGE                    0x3002  // Điện áp Bus (Bus Voltage)
#define GD20_OUTPUT_VOLTAGE                 0x3003  // Điện áp đầu ra (Output Voltage)
#define GD20_OUTPUT_CURRENT                 0x3004  // Dòng điện đầu ra (Output Current)
#define GD20_OUTPUT_SPEED                   0x3005  // Tốc độ động cơ (Output Speed)
#define GD20_OUTPUT_POWER                   0x3006  // Công suất động cơ (Output Power)
#define GD20_OUTPUT_TORQUE                  0x3007  // Moment động cơ (Output Torque)

// 1. Mã chạy động cơ chiều thuận: 01 06 20 00 00 01 43 CA
// const uint8_t CMD_RUN_FWD[]  = {0x01, 0x06, 0x20, 0x00, 0x00, 0x01, 0x43, 0xCA};

// // 2. Mã dừng động cơ: 01 06 20 00 00 05 42 09
// const uint8_t CMD_STOP[]     = {0x01, 0x06, 0x20, 0x00, 0x00, 0x05, 0x42, 0x09};

// // 3. Mã ghi tần số 50Hz: 01 06 20 01 13 88 DE 9C
// const uint8_t CMD_SET_50HZ[] = {0x01, 0x06, 0x20, 0x01, 0x13, 0x88, 0xDE, 0x9C};

/* Value of control command (0x2000) */
#define GD20_CONTROL_CMD_FWD                0x0001  //Chạy động cơ chiều thuận
#define GD20_CONTROL_CMD_REV                0x0002  //Chạy động cơ chiều nghịch
#define GD20_CONTROL_CMD_STOP               0x0005  //Dừng động cơ
#define GD20_CONTROL_CMD_COAST              0x0006  //Dừng động cơ nhanh (coast)
#define GD20_CONTROL_CMD_RESET              0x0007  //Reset biến tần

/*Value of set frequency (0x2001) */
#define GD20_SET_FREQ_0HZ                   0     //0Hz
#define GD20_SET_FREQ_10HZ                  1000  //10Hz
#define GD20_SET_FREQ_25HZ                  2500  //25Hz
#define GD20_SET_FREQ_30HZ                  3000  //30Hz
#define GD20_SET_FREQ_50HZ                  5000  //50Hz
#define GD20_SET_FREQ_60HZ                  6000  //60Hz
#define GD20_SET_FREQ_75HZ                  7500  //75Hz
#define GD20_SET_FREQ_100HZ                 10000 //100Hz Max tần số GD20 hỗ trợ

/* Value of status register (0x2100) */
#define GD20_STATUS_RUN                     0x0001  //Chạy thuận
#define GD20_STATUS_REV                     0x0002  //Chạy nghịch
#define GD20_STATUS_STOP                    0x0003  //Dừng
#define GD20_STATUS_FAULT                   0x0004  //Lỗi
#define GD20_STATUS_COAST                   0x0005  //Trạng thái POFF
#define GD20_STATUS_RESET                   0x0006  //Trạng thái tiền kích thích

/* GD20 Modbus Fault messege respone */
#define GD20_EXC_ILLEGAL_CMD                0x01  // Lệnh không hợp lệ (Illegal command)
#define GD20_EXC_ILLEGAL_DATA_ADDR          0x02  // Địa chỉ dữ liệu không hợp lệ (Illegal data address)
#define GD20_EXC_ILLEGAL_VALUE              0x03  // Giá trị dữ liệu không hợp lệ (Illegal value)
#define GD20_EXC_OPERATION_FAILED           0x04  // Thao tác thất bại (Operation failed)
#define GD20_EXC_PASSWORD_ERROR             0x05  // Lỗi mật khẩu (Password error)
#define GD20_EXC_DATA_FRAME_ERROR           0x06  // Lỗi khung dữ liệu (Data frame error)
#define GD20_EXC_WRITTEN_NOT_ALLOWED        0x07  // Không được phép ghi (Written not allowed)
#define GD20_EXC_PARAM_CANNOT_MODIFY        0x08  // Thông số không thể sửa đổi khi đang chạy
#define GD20_EXC_PASSWORD_PROTECTION        0x09  // Bảo vệ mật khẩu (System locked)
#endif  // !GD20_INVERTER_H
