#include <Arduino.h>
#include <DWIN.h>
#include "DWINKEY.h"

// ============================================================================
// ĐỊNH NGHĨA ĐỊA CHỈ VP
// ============================================================================
#define VP_KEYBOARD_INPUT 0x2500
#define VP_BUTTON         0x2600
#define VP_DISPLAY_OUTPUT 0x2000
#define ADDRESS_TEMP      0x1010
#define ADDRESS_HUMID     0x1515


// ============================================================================
// Giá trị key của các nút
// ============================================================================
#define KEY_RUN           0x0002
#define KEY_STOP          0x0003
#define KEY_ENTER         0x0001
#define KEY_CONFIRM       0x00F1
#define DGUS_BAUD         115200

// ============================================================================
// ĐỊA CHỈ VP CHO THỜI GIAN VÀ NGÀY THÁNG
// ============================================================================
// ---- Thời gian (3 ô Data Variable) ----
#define VP_TIME_HOUR     0x1800  // Giờ (2 chữ số)
#define VP_TIME_MINUTE   0x1801  // Phút (2 chữ số)
#define VP_TIME_SECOND   0x1802  // Giây (2 chữ số)

// ---- Dấu ":" (2 ô Text Display cố định, không cần gửi dữ liệu) ----
// Text Display 1 tại vị trí giữa giờ và phút, Text Display 2 giữa phút và giây
// Nội dung text cố định là ":" được set sẵn trên DGUS

// ---- Ngày tháng (3 ô Data Variable) ----
#define VP_DATE_DAY      0x1810  // Ngày (2 chữ số)
#define VP_DATE_MONTH    0x1811  // Tháng (2 chữ số)
#define VP_DATE_YEAR     0x1812  // Năm (4 chữ số, ví dụ 2024)

// ---- Dấu "/" (2 ô Text Display cố định, không cần gửi dữ liệu) ----
// Text Display 1 tại vị trí giữa ngày và tháng, Text Display 2 giữa tháng và năm
// Nội dung text cố định là "/" được set sẵn trên DGUS
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
// BIẾN CHO THỜI GIAN THỰC
// ============================================================================
int gio = 16;
int phut = 45;
int giay = 0;

int ngay = 18;
int thang = 7;
int nam = 2026;

unsigned long previousTimeMillis = 0;
const unsigned long timeInterval = 1000;  // 1 giây
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
   // ==========================================================================
  // CẬP NHẬT ĐỒNG HỒ THỜI GIAN THỰC MỖI 1 GIÂY
  // ==========================================================================
  unsigned long currentMillis = millis();
  if (currentMillis - previousTimeMillis >= timeInterval) {
    previousTimeMillis = currentMillis;
    
    // Cập nhật thời gian (tăng giây)
    updateRealTime();


    
  }
}
// Hàm mới: Ghi 1 Word (2 bytes) dữ liệu vào địa chỉ VP
// 'address' là địa chỉ VP (ví dụ: 0x1515)
// 'data' là dữ liệu cần gửi (ví dụ: 3025 cho nhiệt độ 30.25)
// Hàm gửi 2 bytes (1 word) lên DWIN
// address: địa chỉ VP (ví dụ 0x1515)
// data: giá trị cần gửi (0 - 65535)
void setVPWord(uint16_t address, uint16_t data) {
    // Tạo frame gửi theo protocol của DWIN
    uint8_t frame[8];
    
    frame[0] = 0x5A;                          // Header byte 1
    frame[1] = 0xA5;                          // Header byte 2
    frame[2] = 0x05;                          // Data length (5 bytes sau frame header)
    frame[3] = 0x82;                          // Write command
    frame[4] = (address >> 8) & 0xFF;         // Address high byte
    frame[5] = address & 0xFF;                // Address low byte
    frame[6] = (data >> 8) & 0xFF;            // Data high byte
    frame[7] = data & 0xFF;                   // Data low byte
    
    // Gửi frame qua Serial
    DGUS_SERIAL.write(frame, 8);
    
    // Debug: in ra frame đã gửi
    Serial.print("[GUI] Frame: ");
    for(int i = 0; i < 8; i++) {
        if(frame[i] < 0x10) Serial.print("0");
        Serial.print(frame[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}
// ============================================================================
// CẬP NHẬT THỜI GIAN (TĂNG MỖI GIÂY)
// ============================================================================
void updateRealTime() {
  giay++;
  
  if (giay >= 60) {
    giay = 0;
    phut++;
    
    if (phut >= 60) {
      phut = 0;
      gio++;
      
      if (gio >= 24) {
        gio = 0;
        ngay++;
        
        // Kiểm tra số ngày trong tháng
        int ngayTrongThang = 31;
        if (thang == 4 || thang == 6 || thang == 9 || thang == 11) {
          ngayTrongThang = 30;
        } else if (thang == 2) {
          bool namNhuan = (nam % 400 == 0) || (nam % 4 == 0 && nam % 100 != 0);
          ngayTrongThang = namNhuan ? 29 : 28;
        }
        
        if (ngay > ngayTrongThang) {
          ngay = 1;
          thang++;
          
          if (thang > 12) {
            thang = 1;
            nam++;
          }
        }
      }
    }
  }
  sendTimeToDWIN(gio, phut);
  sendDateToDWIN(ngay, thang, nam);
}
// ============================================================================
// GỬI THỜI GIAN LÊN DWIN (3 Data Variable riêng biệt)
// ============================================================================
void sendTimeToDWIN(int hour, int minute) {
  uint16_t gio = (uint16_t)(hour);
  uint16_t phut = (uint16_t)(minute);

  // Gửi giờ (0-23)
  hmi.setVP(VP_TIME_HOUR, gio);
  
  // Gửi phút (0-59)
  hmi.setVP(VP_TIME_MINUTE, phut);
  

  
  // Debug log
  Serial.print("[TIME] Gui len DWIN: ");
  if (gio < 10) Serial.print("0");
  Serial.print(gio);
  Serial.print(":");
  if (phut < 10) Serial.print("0");
  Serial.print(phut);
  Serial.print(":");
  if (giay < 10) Serial.print("0");
  Serial.println(giay);
}
// ============================================================================
// GỬI NGÀY THÁNG LÊN DWIN (3 Data Variable riêng biệt)
// ============================================================================
void sendDateToDWIN(int day, int month,int year) {
  uint16_t ngay = (uint16_t)(day);
  uint16_t thang = (uint16_t)(month);
  uint16_t nam = (uint16_t)(year);
  // Gửi ngày (1-31)
  hmi.setVP(VP_DATE_DAY, ngay);
  
  // Gửi tháng (1-12)
  hmi.setVP(VP_DATE_MONTH, thang);
  
  // Gửi năm (ví dụ: 2024)
  setVPWord(VP_DATE_YEAR, nam);
  
  // Debug log
  Serial.print("[DATE] Gui len DWIN: ");
  if (ngay < 10) Serial.print("0");
  Serial.print(ngay);
  Serial.print("/");
  if (thang < 10) Serial.print("0");
  Serial.print(thang);
  Serial.print("/");
  Serial.println(nam);
}
// ============================================================================
// GỬI TOÀN BỘ THỜI GIAN VÀ NGÀY THÁNG LÊN DWIN
// ============================================================================

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
  uint16_t tempValue = (uint16_t)(temperature*100);
  setVPWord(ADDRESS_TEMP, tempValue);

  Serial.print(" Gửi nhiệt độ: ");
  Serial.print(temperature, 1);
  Serial.print("°C (Giá trị gửi: ");
  Serial.print(tempValue);
  Serial.println(")");
}

void sendHumidity(float humidity) {
  int humValue = (int)(humidity*100);
  setVPWord(ADDRESS_HUMID, (uint16_t)humValue);

  Serial.print(" Gửi độ ẩm: ");
  Serial.print(humidity, 0);
  Serial.print("% (Giá trị gửi: ");
  Serial.print(humValue);
  Serial.println(")");
}

void updateTemperature() {
  if (systemRunning) {
    currentTemperature += 0.25;
    if (currentTemperature > 90.0) {
      currentTemperature = 30.0;
    }
    sendTemperature(currentTemperature);
  }
}

void updateHumidity() {
  if (systemRunning) {
    currentHumidity += 0.5;
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
    
    // Đảm bảo giá trị không vượt quá 70
    if (currentValue > 70) {
      currentValue = 70;
      pendingNumber = "70";
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
    Serial.println("[NHAP] KHONG THE NHAP THEM! Gioi han toi da la 70!");
    Serial.println("─────────────────────────────────────────────");
  }

    } else if (lastByte == 0x00F2) {
  if (pendingNumber.length() > 0) {
    pendingNumber.remove(pendingNumber.length() - 1);
    if (pendingNumber.length() == 0) {
      hasPendingNumber = false;
    }
    
    // ============================================================
    // CẬP NHẬT LẠI MÀN HÌNH DWIN SAU KHI XÓA
    // ============================================================
    int currentValue = pendingNumber.length() > 0 ? pendingNumber.toInt() : 0;
    sendToDWIN(VP_DISPLAY_OUTPUT, currentValue);
    
    Serial.println("─────────────────────────────────────────────");
    Serial.print("[NHAP] Da xoa 1 so! So hien tai: ");
    Serial.println(pendingNumber.length() > 0 ? pendingNumber : "(rong)");
    Serial.print("[NHAP] Da cap nhat len DWIN: ");
    Serial.println(currentValue);
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