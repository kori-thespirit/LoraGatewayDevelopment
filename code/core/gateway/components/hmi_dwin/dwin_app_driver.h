#ifndef _DWIN_APP_DRIVER_H_
#define _DWIN_APP_DRIVER_H_
#include <stdint.h>

#define VP_DATE_DAY      0x1810  // Ngày (2 chữ số)
#define VP_DATE_MONTH    0x1811  // Tháng (2 chữ số)
#define VP_DATE_YEAR     0x1812  // Năm (4 chữ số, ví dụ 2024)
#define VP_TIME_HOUR     0x1800  // Giờ (2 chữ số)
#define VP_TIME_MINUTE   0x1801  // Phút (2 chữ số)
#define VP_TIME_SECOND   0x1802  // Giây (2 chữ số)

/* DWIN VP address */
#define VP_KEYBOARD_INPUT 0x2500
#define VP_BUTTON         0x2600
#define VP_DISPLAY_OUTPUT 0x2000
#define ADDRESS_TEMP      0x1010
#define ADDRESS_HUMID     0x1515

/* Matrix key define */
#define KEY_RUN           0x0002
#define KEY_STOP          0x0003
#define KEY_ENTER         0x0001
#define KEY_CONFIRM       0x00F1
#define DGUS_BAUD         115200

#define CMD_HEAD1           0x5A
#define CMD_HEAD2           0xA5
#define CMD_WRITE           0x82
#define CMD_READ            0x83

#define DGUS_BAUD 115200
#define UART_PORT              (UART_NUM_1)

/* Gateway HMI port */
#define TX_PIN                (5)
#define RX_PIN                (6)

/* Unit test HMI port */
// #define RX_PIN                (16)
// #define TX_PIN                (17)
//
typedef struct rtc {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t date;
    uint16_t year;
} st_rtc_t;

#endif // _DWIN_APP_DRIVER_H_
