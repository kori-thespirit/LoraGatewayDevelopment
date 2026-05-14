#include <Arduino.h>
#include <DWIN.h>
#include "DWINKEY.h"

// ============================================================================
// ĐỊNH NGHĨA ĐỊA CHỈ VP
// ============================================================================
#define VP_KEYBOARD_INPUT 0x2500
#define VP_BUTTON 0x2600
#define VP_DISPLAY_OUTPUT 0x2000
#define ADDRESS_TEMP 0x1010
#define ADDRESS_HUMID 0x1515


// ============================================================================
// Giá trị key của các nút
// ============================================================================
#define KEY_RUN 0x0002
#define KEY_STOP 0x0003
#define KEY_ENTER 0x0001
#define KEY_CONFIRM 0x00F1
#define DGUS_BAUD 115200

// ============================================================================
// CẤU HÌNH PHẦN CỨNG
// ============================================================================
#if defined(ESP32)
#define DGUS_SERIAL Serial2
DWIN hmi(DGUS_SERIAL, 16, 17, DGUS_BAUD);
#else
DWIN hmi(2, 3, DGUS_BAUD);
#endif

// ============================================================================
// KHỞI TẠO ĐỐI TƯỢNG BÀN PHÍM
// ============================================================================
DWIN_Keyboard keyboard;

// Biến lưu số tạm thời
String pendingNumber = "";
bool hasPendingNumber = false;
float currentTemperature = 30.0;
float currentHumidity = 60.0;
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 2000;
bool systemRunning = true;

// ============================================================================
// BIẾN CHO TIMER (ESP32)
// ============================================================================
hw_timer_t *timer = NULL;
volatile bool timerFlag = false;  // Cờ báo hiệu từ timer

// ============================================================================
// HÀM NGẮT TIMER (ISR) - CHỈ ĐẶT CỜ, KHÔNG LÀM GÌ KHÁC
// ============================================================================
void IRAM_ATTR onTimer() {
  timerFlag = true;  // Chỉ đặt cờ, không xử lý gì thêm
}

// ============================================================================
// HÀM XỬ LÝ CẬP NHẬT NHIỆT ĐỘ/ĐỘ ẨM (CHẠY TRONG LOOP)
// ============================================================================
void processSensorUpdate() {
  Serial.println("\n---  CẬP NHẬT DỮ LIỆU SHT20 ĐỊNH KỲ ---");

  // Cập nhật nhiệt độ và độ ẩm
  updateTemperature();
  updateHumidity();
}

// ============================================================================
// CÁC HÀM KHÁC (sendToDWIN, sendTemperature, sendHumidity,
// updateTemperature, updateHumidity, onHMIEvent)
// ============================================================================

void sendToDWIN(uint16_t address, uint16_t data) {
  uint8_t frame[8];
  frame[0] = 0x5A;
  frame[1] = 0xA5;
  frame[2] = 0x05;
  frame[3] = 0x82;
  frame[4] = (address >> 8) & 0xFF;
  frame[5] = address & 0xFF;
  frame[6] = (data >> 8) & 0xFF;
  frame[7] = data & 0xFF;

  DGUS_SERIAL.write(frame, 8);

  Serial.print("[GUI] Da gui frame len VP 0x");
  Serial.print(address, HEX);
  Serial.print(": gia tri ");
  Serial.println(data);
}

void sendTemperature(float temperature) {
  uint16_t tempValue = (uint16_t)(temperature);
  hmi.setVP(ADDRESS_TEMP, tempValue);

  Serial.print(" Gửi nhiệt độ: ");
  Serial.print(temperature, 1);
  Serial.print("°C (Giá trị gửi: ");
  Serial.print(tempValue);
  Serial.println(")");
}

void sendHumidity(float humidity) {
  uint16_t humValue = (uint16_t)(humidity);
  hmi.setVP(ADDRESS_HUMID, humValue);

  Serial.print(" Gửi độ ẩm: ");
  Serial.print(humidity, 0);
  Serial.print("% (Giá trị gửi: ");
  Serial.print(humValue);
  Serial.println(")");
}

void updateTemperature() {
  if (systemRunning) {
    currentTemperature += 1;
    if (currentTemperature > 90.0) {
      currentTemperature = 30.0;
    }
    sendTemperature(currentTemperature);
  }
}

void updateHumidity() {
  if (systemRunning) {
    currentHumidity += 1;
    if (currentHumidity > 90) {
      currentHumidity = 60;
    }
    sendHumidity(currentHumidity);
  }
}

void onHMIEvent(String address, int lastByte, String message, String response) {

  Serial.println("══════════════════════════════════════════════════════");
  Serial.print("[EVENT] Dia chi VP: 0x");
  Serial.print(address);
  Serial.print(" | LastByte: 0x");
  Serial.println(String(lastByte, HEX));

  if (address == "2500") {

    if (lastByte >= 0x30 && lastByte <= 0x39) {
      char so = '0' + (lastByte - 0x30);

      // ============================================================
      // GIỚI HẠN: Chỉ cho nhập tối đa 2 chữ số (0-99)
      // ============================================================
      if (pendingNumber.length() < 2) {
        pendingNumber += so;
        hasPendingNumber = true;

        int currentValue = pendingNumber.toInt();

        // Đảm bảo giá trị không vượt quá 99
        if (currentValue > 99) {
          currentValue = 99;
          pendingNumber = "99";
        }

        sendToDWIN(VP_DISPLAY_OUTPUT, currentValue);

        Serial.println("─────────────────────────────────────────────");
        Serial.print("[NHAP] Da them so: ");
        Serial.println(so);
        Serial.print("[NHAP] So tam thoi: ");
        Serial.println(pendingNumber);
        Serial.print("[NHAP] Da gui so tam thoi ");
        Serial.print(currentValue);
        Serial.println(" len DWIN (cho xem truoc)!");
        Serial.println("[TRANG THAI] Dang cho nhan Confirm de xac nhan!");
        Serial.println("─────────────────────────────────────────────");
      } else {
        // Đã đạt giới hạn 2 chữ số
        Serial.println("─────────────────────────────────────────────");
        Serial.println("[NHAP] KHONG THE NHAP THEM! Gioi han toi da la 99!");
        Serial.println("─────────────────────────────────────────────");
      }

    } else if (lastByte == 0x00F2) {
      if (pendingNumber.length() > 0) {
        pendingNumber.remove(pendingNumber.length() - 1);
        if (pendingNumber.length() == 0) {
          hasPendingNumber = false;
        }
        Serial.println("─────────────────────────────────────────────");
        Serial.print("[NHAP] Da xoa 1 so! So hien tai: ");
        Serial.println(pendingNumber.length() > 0 ? pendingNumber : "(rong)");
        Serial.println("─────────────────────────────────────────────");
      }
    } else if (lastByte == 0x00F5) {
      pendingNumber = "";
      hasPendingNumber = false;
      Serial.println("─────────────────────────────────────────────");
      Serial.println("[NHAP] Da xoa toan bo so da nhap!");
      Serial.println("─────────────────────────────────────────────");
    }
  }

  else if (address == "2600") {

    if (lastByte == KEY_ENTER) {
      Serial.println("─────────────────────────────────────────────");
      Serial.println("[CONFIRM] NUT CONFIRM DA DUOC NHAN!");

      if (hasPendingNumber && pendingNumber.length() > 0) {
        int finalNumber = pendingNumber.toInt();
        Serial.print("[CONFIRM] SO XAC NHAN: ");
        Serial.println(finalNumber);

        sendToDWIN(VP_DISPLAY_OUTPUT, finalNumber);

        Serial.print("[KET QUA] DA GUI SO ");
        Serial.print(finalNumber);
        Serial.println(" LEN DWIN TAI VP 0x2000!");

        pendingNumber = "";
        hasPendingNumber = false;
      } else {
        Serial.println("[CONFIRM] KHONG CO SO NAO DE XAC NHAN!");
      }
      Serial.println("─────────────────────────────────────────────");
    }

    else if (lastByte == KEY_RUN) {
      Serial.println("─────────────────────────────────────────────");
      Serial.println("[RUN] NUT RUN DUOC NHAN!");
      Serial.println("─────────────────────────────────────────────");
    }

    else if (lastByte == KEY_STOP) {
      Serial.println("─────────────────────────────────────────────");
      Serial.println("[STOP] NUT STOP DUOC NHAN!");
      Serial.println("─────────────────────────────────────────────");
    }

    else {
      Serial.println("─────────────────────────────────────────────");
      Serial.print("[UNKNOWN] NUT KHAC DUOC NHAN, KEY VALUE: ");
      Serial.println(lastByte);
      Serial.println("─────────────────────────────────────────────");
    }
  }

  Serial.println("");
}

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);
  DGUS_SERIAL.begin(DGUS_BAUD, SERIAL_8N1, 16, 17);

#ifdef debug
  Serial.println("");
  Serial.println("╔════════════════════════════════════════════════════════════╗");
  Serial.println("║     DWIN KEYBOARD - NHAP SO VA CHO CONFIRM                 ║");
  Serial.println("╠════════════════════════════════════════════════════════════╣");
  Serial.println("║  Nguyên lý hoạt động:                                      ║");
  Serial.println("║    1. Nhấn các số (0-9) -> ESP32 lưu tạm vào bộ đệm       ║");
  Serial.println("║    2. Nhấn BACKSPACE -> xóa 1 số                           ║");
  Serial.println("║    3. Nhấn CLEAR -> xóa toàn bộ                            ║");
  Serial.println("║    4. Nhấn CONFIRM -> ESP32 gửi số lên VP 0x2000           ║");
  Serial.println("╠════════════════════════════════════════════════════════════╣");
  Serial.println("║  Cấu hình DGUS cần có:                                     ║");
  Serial.println("║    - VP 0x2500: Key Return cho các số (0x30-0x39)          ║");
  Serial.println("║    - VP 0x2500: Key Return cho Backspace (0x00F2)          ║");
  Serial.println("║    - VP 0x2500: Key Return cho Clear (0x00F5)              ║");
  Serial.println("║    - VP 0x2600: Key Return cho Confirm (0x00F1)            ║");
  Serial.println("║    - VP 0x2000: Data Variable Display (hiển thị kết quả)  ║");
  Serial.println("╚════════════════════════════════════════════════════════════╝");
  Serial.println("");
#endif
  // Cấu hình DWIN
  hmi.echoEnabled(true);
  hmi.hmiCallBack(onHMIEvent);
  hmi.setPage(0);
  hmi.setBrightness(80);

  // Gửi giá trị khởi tạo nhiệt độ - độ ẩm
  sendTemperature(currentTemperature);
  sendHumidity(currentHumidity);

  // Khởi tạo ô hiển thị với giá trị 0
  sendToDWIN(VP_DISPLAY_OUTPUT, 0);

  // ==========================================================================
  // CẤU HÌNH TIMER (THAY THẾ HÀM NGẮT TRUYỀN THỐNG)
  // ==========================================================================
  // Timer 0, prescaler = 80, đếm mỗi 1 micro giây (80 MHz / 80 = 1 MHz)
  timer = timerBegin(0, 80, true);

  // Gắn hàm ngắt
  timerAttachInterrupt(timer, &onTimer, true);

  // Đặt chu kỳ 2000000 micro giây = 2 giây
  timerAlarmWrite(timer, 2000000, true);

  // Bật timer
  timerAlarmEnable(timer);
#ifdef debug
  Serial.println("[TIMER] Timer da duoc cau hinh, chu ky 2 giay!");
#endif

  delay(1000);
#ifdef debug
  Serial.println("[STATUS] He thong san sang!");
  Serial.println("[STATUS] Hay nhap so va nhan CONFIRM de gui len DWIN!");
  Serial.println("");
#endif
}

// ============================================================================
// LOOP
// ============================================================================
void loop() {
  // Lắng nghe sự kiện từ DWIN
  hmi.listen();

  // ==========================================================================
  // KIỂM TRA CỜ TỪ TIMER (AN TOÀN HƠN DÙNG NGẮT TRỰC TIẾP)
  // ==========================================================================
  if (timerFlag) {
    timerFlag = false;  // Xóa cờ

    // Xử lý cập nhật nhiệt độ/độ ẩm (KHÔNG nằm trong ISR)
    processSensorUpdate();
  }
  delay(1);
}