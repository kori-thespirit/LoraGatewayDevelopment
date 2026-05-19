// ============================================================================
// DWIN_Keyboard.h
// Thư viện xử lý bàn phím cho màn hình DWIN DGUS II
// Dựa trên tài liệu: T5L_DGUSII Application Development Guide V2.9
// Trang tham khảo: 6.6 Return Key Code (trang 97-98), 6.7 Text Input (trang 99-106)
// ============================================================================

#ifndef DWIN_KEYBOARD_H
#define DWIN_KEYBOARD_H

#include <Arduino.h>

// ============================================================================
// ĐỊNH NGHĨA ĐỊA CHỈ VP 
// ============================================================================
#define VP_KEYBOARD_INPUT   0x2500
#define VP_BUTTON           0x2600
#define VP_DISPLAY_OUTPUT   0x2000
#define ADDRESS_TEMP        0x1010
#define ADDRESS_HUMID       0x1515


// ============================================================================
// Giá trị key của các nút
// ============================================================================
#define KEY_RUN             0x0002
#define KEY_STOP            0x0003
#define KEY_ENTER           0x0001
#define KEY_CONFIRM         0x00F1
#define DGUS_BAUD 115200

// ============================================================================
// ĐỊNH NGHĨA KEY CODE THEO BẢNG MÃ ASCII 
// ============================================================================

// ----- Nút số (0-9) -----
#define KEY_0               0x0030
#define KEY_1               0x0031
#define KEY_2               0x0032
#define KEY_3               0x0033
#define KEY_4               0x0034
#define KEY_5               0x0035
#define KEY_6               0x0036
#define KEY_7               0x0037
#define KEY_8               0x0038
#define KEY_9               0x0039

// ----- Chữ cái in hoa (A-Z) -----
#define KEY_A               0x0041
#define KEY_B               0x0042
#define KEY_C               0x0043
#define KEY_D               0x0044
#define KEY_E               0x0045
#define KEY_F               0x0046
#define KEY_G               0x0047
#define KEY_H               0x0048
#define KEY_I               0x0049
#define KEY_J               0x004A
#define KEY_K               0x004B
#define KEY_L               0x004C
#define KEY_M               0x004D
#define KEY_N               0x004E
#define KEY_O               0x004F
#define KEY_P               0x0050
#define KEY_Q               0x0051
#define KEY_R               0x0052
#define KEY_S               0x0053
#define KEY_T               0x0054
#define KEY_U               0x0055
#define KEY_V               0x0056
#define KEY_W               0x0057
#define KEY_X               0x0058
#define KEY_Y               0x0059
#define KEY_Z               0x005A

// ----- Chữ cái in thường (a-z) -----
#define KEY_a               0x0061
#define KEY_b               0x0062
#define KEY_c               0x0063
#define KEY_d               0x0064
#define KEY_e               0x0065
#define KEY_f               0x0066
#define KEY_g               0x0067
#define KEY_h               0x0068
#define KEY_i               0x0069
#define KEY_j               0x006A
#define KEY_k               0x006B
#define KEY_l               0x006C
#define KEY_m               0x006D
#define KEY_n               0x006E
#define KEY_o               0x006F
#define KEY_p               0x0070
#define KEY_q               0x0071
#define KEY_r               0x0072
#define KEY_s               0x0073
#define KEY_t               0x0074
#define KEY_u               0x0075
#define KEY_v               0x0076
#define KEY_w               0x0077
#define KEY_x               0x0078
#define KEY_y               0x0079
#define KEY_z               0x007A

// ----- Ký tự đặc biệt (Bảng 6.7 trang 99-100) -----
#define KEY_SPACE           0x0020      // Dấu cách
#define KEY_EXCLAMATION     0x0021      // !
#define KEY_QUOTE           0x0022      // "
#define KEY_HASH            0x0023      // #
#define KEY_DOLLAR          0x0024      // $
#define KEY_PERCENT         0x0025      // %
#define KEY_AMPERSAND       0x0026      // &
#define KEY_APOSTROPHE      0x0027      // '
#define KEY_LEFT_PAREN      0x0028      // (
#define KEY_RIGHT_PAREN     0x0029      // )
#define KEY_ASTERISK        0x002A      // *
#define KEY_PLUS            0x002B      // +
#define KEY_COMMA           0x002C      // ,
#define KEY_MINUS           0x002D      // -
#define KEY_PERIOD          0x002E      // .
#define KEY_SLASH           0x002F      // /
#define KEY_COLON           0x003A      // :
#define KEY_SEMICOLON       0x003B      // ;
#define KEY_LESS_THAN       0x003C      // <
#define KEY_EQUALS          0x003D      // =
#define KEY_GREATER_THAN    0x003E      // >
#define KEY_QUESTION        0x003F      // ?
#define KEY_AT              0x0040      // @
#define KEY_LEFT_BRACKET    0x005B      // [
#define KEY_BACKSLASH       0x005C      // \

#define KEY_RIGHT_BRACKET   0x005D      // ]
#define KEY_CARET           0x005E      // ^
#define KEY_UNDERSCORE      0x005F      // _
#define KEY_GRAVE           0x0060      // `
#define KEY_LEFT_BRACE      0x007B      // {
#define KEY_VERTICAL_BAR    0x007C      // |
#define KEY_RIGHT_BRACE     0x007D      // }
#define KEY_TILDE           0x007E      // ~

// ----- Nút chức năng đặc biệt (Tài liệu trang 100-101) -----
#define KEY_CANCEL          0x00F0      // Hủy bỏ nhập liệu
#define KEY_ENTER           0x00F1      // Xác nhận / Return
#define KEY_BACKSPACE       0x00F2      // Xóa lùi (Backspace)
#define KEY_DELETE          0x00F3      // Xóa ký tự phía sau (Delete)
#define KEY_CAPS_LOCK       0x00F4      // Khóa chữ hoa (CapsLock)
#define KEY_CLEAR           0x00F5      // Xóa toàn bộ
#define KEY_LEFT            0x00F7      // Di chuyển con trỏ sang trái
#define KEY_RIGHT           0x00F8      // Di chuyển con trỏ sang phải

// ============================================================================
// LỚP DWIN_KEYBOARD
// ============================================================================

class DWIN_Keyboard {
private:
  String _inputBuffer;           // Bộ đệm nhập liệu
  bool _capsLock;                // Trạng thái CapsLock
  bool _shiftPressed;            // Trạng thái Shift (tùy chọn)
  
public:
  // Constructor
  DWIN_Keyboard() {
    _inputBuffer = "";
    _capsLock = false;
    _shiftPressed = false;
  }
  
  // ==========================================================================
  // HÀM CHUYỂN ĐỔI KEY CODE THÀNH KÝ TỰ
  // ==========================================================================
  
  /**
   * Chuyển đổi key code thành ký tự (cho các nút số, chữ cái, ký tự đặc biệt)
   * @param keyCode: Mã key nhận được từ sự kiện onHMIEvent (lastByte)
   * @return: Ký tự tương ứng (char), trả về '\0' nếu không phải ký tự nhập liệu
   */
  char keyCodeToChar(int keyCode) {
    // Nút số 0-9
    if (keyCode >= KEY_0 && keyCode <= KEY_9) {
      return '0' + (keyCode - KEY_0);
    }
    
    // Chữ cái in hoa A-Z
    if (keyCode >= KEY_A && keyCode <= KEY_Z) {
      if (_capsLock) {
        return (char)keyCode;  // CapsLock bật: in hoa
      } else {
        return (char)(keyCode + 32);  // CapsLock tắt: in thường
      }
    }
    
    // Chữ cái in thường a-z (trường hợp bàn phím gửi trực tiếp)
    if (keyCode >= KEY_a && keyCode <= KEY_z) {
      if (_capsLock) {
        return (char)(keyCode - 32);  // CapsLock bật: in hoa
      } else {
        return (char)keyCode;  // CapsLock tắt: in thường
      }
    }
    
    // Ký tự đặc biệt
    switch(keyCode) {
      case KEY_SPACE:         return ' ';
      case KEY_EXCLAMATION:   return '!';
      case KEY_QUOTE:         return '"';
      case KEY_HASH:          return '#';
      case KEY_DOLLAR:        return '$';
      case KEY_PERCENT:       return '%';
      case KEY_AMPERSAND:     return '&';
      case KEY_APOSTROPHE:    return '\'';
      case KEY_LEFT_PAREN:    return '(';
      case KEY_RIGHT_PAREN:   return ')';
      case KEY_ASTERISK:      return '*';
      case KEY_PLUS:          return '+';
      case KEY_COMMA:         return ',';
      case KEY_MINUS:         return '-';
      case KEY_PERIOD:        return '.';
      case KEY_SLASH:         return '/';
      case KEY_COLON:         return ':';
      case KEY_SEMICOLON:     return ';';
      case KEY_LESS_THAN:     return '<';
      case KEY_EQUALS:        return '=';
      case KEY_GREATER_THAN:  return '>';
      case KEY_QUESTION:      return '?';
      case KEY_AT:            return '@';
      case KEY_LEFT_BRACKET:  return '[';
      case KEY_BACKSLASH:     return '\\';
      case KEY_RIGHT_BRACKET: return ']';
      case KEY_CARET:         return '^';
      case KEY_UNDERSCORE:    return '_';
      case KEY_GRAVE:         return '`';
      case KEY_LEFT_BRACE:    return '{';
      case KEY_VERTICAL_BAR:  return '|';
      case KEY_RIGHT_BRACE:   return '}';
      case KEY_TILDE:         return '~';
      default:                return '\0';
    }
  }
  
  // ==========================================================================
  // HÀM XỬ LÝ CÁC NÚT CHỨC NĂNG
  // ==========================================================================
  
  /**
   * Xử lý nút chức năng và cập nhật bộ đệm
   * @param keyCode: Mã key nhận được
   * @return: Loại hành động được thực hiện:
   *   0 - Không xử lý (không phải nút chức năng)
   *   1 - Đã thêm ký tự
   *   2 - Đã xóa ký tự (Backspace)
   *   3 - Đã xóa ký tự (Delete)
   *   4 - Đã xóa toàn bộ (Clear)
   *   5 - Xác nhận (Enter)
   *   6 - Hủy bỏ (Cancel)
   *   7 - CapsLock thay đổi
   */
  int processKey(int keyCode) {
    // Xử lý nút chức năng đặc biệt
    if (keyCode == KEY_BACKSPACE) {
      if (_inputBuffer.length() > 0) {
        _inputBuffer.remove(_inputBuffer.length() - 1);
      }
      Serial.println("[KEY] Backspace - Xóa 1 ký tự");
      return 2;
    }
    
    else if (keyCode == KEY_DELETE) {
      // Xóa ký tự phía sau (cần biết vị trí con trỏ)
      Serial.println("[KEY] Delete");
      return 3;
    }
    
    else if (keyCode == KEY_CLEAR) {
      _inputBuffer = "";
      Serial.println("[KEY] Clear - Xóa toàn bộ");
      return 4;
    }
    
    else if (keyCode == KEY_ENTER) {
      Serial.print("[KEY] Enter - Xác nhận: ");
      Serial.println(_inputBuffer);
      return 5;
    }
    
    else if (keyCode == KEY_CANCEL) {
      _inputBuffer = "";
      Serial.println("[KEY] Cancel - Hủy bỏ nhập liệu");
      return 6;
    }
    
    else if (keyCode == KEY_CAPS_LOCK) {
      _capsLock = !_capsLock;
      Serial.print("[KEY] CapsLock - ");
      Serial.println(_capsLock ? "BẬT (chữ hoa)" : "TẮT (chữ thường)");
      return 7;
    }
    
    else if (keyCode == KEY_LEFT) {
      Serial.println("[KEY] Left - Di chuyển con trỏ sang trái");
      return 0;
    }
    
    else if (keyCode == KEY_RIGHT) {
      Serial.println("[KEY] Right - Di chuyển con trỏ sang phải");
      return 0;
    }
    
    // Xử lý ký tự nhập liệu
    else {
      char c = keyCodeToChar(keyCode);
      if (c != '\0') {
        _inputBuffer += c;
        Serial.print("[KEY] Nhập: ");
        Serial.print(c);
        Serial.print(" | Buffer: ");
        Serial.println(_inputBuffer);
        return 1;
      }
    }
    
    return 0;
  }
  
  /**
   * Xử lý key code và trả về kết quả dưới dạng chuỗi
   * Phù hợp cho việc nhập số (tần số, nhiệt độ...)
   */
  String processNumberInput(int keyCode) {
    char c = keyCodeToChar(keyCode);
    
    // Chỉ chấp nhận số 0-9
    if (c >= '0' && c <= '9') {
      _inputBuffer += c;
      return _inputBuffer;
    }
    
    // Xóa lùi
    if (keyCode == KEY_BACKSPACE && _inputBuffer.length() > 0) {
      _inputBuffer.remove(_inputBuffer.length() - 1);
      return _inputBuffer;
    }
    
    // Xóa toàn bộ
    if (keyCode == KEY_CLEAR) {
      _inputBuffer = "";
      return _inputBuffer;
    }
    
    return _inputBuffer;
  }
  
  // ==========================================================================
  // HÀM KIỂM TRA LOẠI PHÍM
  // ==========================================================================
  
  bool isNumberKey(int keyCode) {
    return (keyCode >= KEY_0 && keyCode <= KEY_9);
  }
  
  bool isLetterKey(int keyCode) {
    return (keyCode >= KEY_A && keyCode <= KEY_Z) || (keyCode >= KEY_a && keyCode <= KEY_z);
  }
  
  bool isSpecialCharKey(int keyCode) {
    return ((keyCode >= KEY_SPACE && keyCode <= KEY_SLASH) ||
            (keyCode >= KEY_COLON && keyCode <= KEY_AT) ||
            (keyCode >= KEY_LEFT_BRACKET && keyCode <= KEY_GRAVE) ||
            (keyCode >= KEY_LEFT_BRACE && keyCode <= KEY_TILDE));
  }
  
  bool isFunctionKey(int keyCode) {
    return (keyCode >= 0x00F0 && keyCode <= 0x00F8);
  }
  
  // ==========================================================================
  // HÀM LẤY THÔNG TIN BỘ ĐỆM
  // ==========================================================================
  
  String getBuffer() {
    return _inputBuffer;
  }
  
  void clearBuffer() {
    _inputBuffer = "";
  }
  
  void setBuffer(String value) {
    _inputBuffer = value;
  }
  
  int getBufferLength() {
    return _inputBuffer.length();
  }
  
  int getBufferAsInt() {
    return _inputBuffer.toInt();
  }
  
  float getBufferAsFloat() {
    return _inputBuffer.toFloat();
  }
  
  bool isCapsLock() {
    return _capsLock;
  }
  
  void setCapsLock(bool state) {
    _capsLock = state;
  }
  
  // ==========================================================================
  // HÀM MÔ TẢ KEY CODE (DEBUG)
  // ==========================================================================
  
  String getKeyDescription(int keyCode) {
    switch(keyCode) {
      // Nút số
      case KEY_0: return "0";
      case KEY_1: return "1";
      case KEY_2: return "2";
      case KEY_3: return "3";
      case KEY_4: return "4";
      case KEY_5: return "5";
      case KEY_6: return "6";
      case KEY_7: return "7";
      case KEY_8: return "8";
      case KEY_9: return "9";
      
      // Chữ cái in hoa
      case KEY_A: return "A";
      case KEY_B: return "B";
      case KEY_C: return "C";
      case KEY_D: return "D";
      case KEY_E: return "E";
      case KEY_F: return "F";
      case KEY_G: return "G";
      case KEY_H: return "H";
      case KEY_I: return "I";
      case KEY_J: return "J";
      case KEY_K: return "K";
      case KEY_L: return "L";
      case KEY_M: return "M";
      case KEY_N: return "N";
      case KEY_O: return "O";
      case KEY_P: return "P";
      case KEY_Q: return "Q";
      case KEY_R: return "R";
      case KEY_S: return "S";
      case KEY_T: return "T";
      case KEY_U: return "U";
      case KEY_V: return "V";
      case KEY_W: return "W";
      case KEY_X: return "X";
      case KEY_Y: return "Y";
      case KEY_Z: return "Z";
      
      // Nút chức năng
      case KEY_CANCEL:    return "CANCEL";
      case KEY_ENTER:     return "ENTER";
      case KEY_BACKSPACE: return "BACKSPACE";
      case KEY_DELETE:    return "DELETE";
      case KEY_CAPS_LOCK: return "CAPS_LOCK";
      case KEY_CLEAR:     return "CLEAR";
      case KEY_LEFT:      return "LEFT_ARROW";
      case KEY_RIGHT:     return "RIGHT_ARROW";
      
      default:
        // Kiểm tra ký tự đặc biệt
        char c = keyCodeToChar(keyCode);
        if (c != '\0') {
          return String(c);
        }
        return "UNKNOWN";
    }
  }
};

#endif // DWIN_KEYBOARD_H