#ifndef GD20_INVERTER_H
#define GD20_INVERTER_H

// GD20 registers
#define GD20_SLAVE_ID             0x01
#define GD20_REG_CONTROL_CMD      0x2000  // Thanh ghi lệnh chạy/dừng
#define GD20_REG_SET_FREQ         0x2001  // Thanh ghi cài tần số
#define GD20_REG_ID               0x2103  // GD20 ID code

// 1. Mã chạy động cơ chiều thuận: 01 06 20 00 00 01 43 CA
// const uint8_t CMD_RUN_FWD[]  = {0x01, 0x06, 0x20, 0x00, 0x00, 0x01, 0x43, 0xCA};

// // 2. Mã dừng động cơ: 01 06 20 00 00 05 42 09
// const uint8_t CMD_STOP[]     = {0x01, 0x06, 0x20, 0x00, 0x00, 0x05, 0x42, 0x09};

// // 3. Mã ghi tần số 50Hz: 01 06 20 01 13 88 DE 9C
// const uint8_t CMD_SET_50HZ[] = {0x01, 0x06, 0x20, 0x01, 0x13, 0x88, 0xDE, 0x9C};

/* Value of control command (0x2000) */
#define GD20_CONTROL_CMD_FWD     0x0001
#define GD20_CONTROL_CMD_REV     0x0002
#define GD20_CONTROL_CMD_STOP    0x0005

#endif  // !GD20_INVERTER_H
