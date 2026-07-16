// DWIN_Keyboard.h
// Thư viện xử lý bàn phím cho màn hình DWIN DGUS II
// Đã sửa lỗi comment và macro

#ifndef DWIN_KEYBOARD_H
#define DWIN_KEYBOARD_H

#include <string>
#include <cstdio>

// ============================================================================
// ĐỊNH NGHĨA ĐỊA CHỈ VP 
// ============================================================================
#define VP_KEYBOARD_INPUT   0x2500
#define VP_BUTTON           0x2600
#define VP_DISPLAY_OUTPUT   0x2000

// ============================================================================
// ĐỊNH NGHĨA KEY CODE THEO BẢNG MÃ ASCII 
// ============================================================================

// Nút số (0-9)
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

// Chữ cái in hoa (A-Z)
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

// Chữ cái in thường (a-z)
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

// Ký tự đặc biệt
#define KEY_SPACE           0x0020
#define KEY_EXCLAMATION     0x0021
#define KEY_QUOTE           0x0022
#define KEY_HASH            0x0023
#define KEY_DOLLAR          0x0024
#define KEY_PERCENT         0x0025
#define KEY_AMPERSAND       0x0026
#define KEY_APOSTROPHE      0x0027
#define KEY_LEFT_PAREN      0x0028
#define KEY_RIGHT_PAREN     0x0029
#define KEY_ASTERISK        0x002A
#define KEY_PLUS            0x002B
#define KEY_COMMA           0x002C
#define KEY_MINUS           0x002D
#define KEY_PERIOD          0x002E
#define KEY_SLASH           0x002F
#define KEY_COLON           0x003A
#define KEY_SEMICOLON       0x003B
#define KEY_LESS_THAN       0x003C
#define KEY_EQUALS          0x003D
#define KEY_GREATER_THAN    0x003E
#define KEY_QUESTION        0x003F
#define KEY_AT              0x0040
#define KEY_LEFT_BRACKET    0x005B

// SỬA LỖI: Dòng này bị lỗi comment, đã tách riêng
#define KEY_BACKSLASH       0x005C

#define KEY_RIGHT_BRACKET   0x005D
#define KEY_CARET           0x005E
#define KEY_UNDERSCORE      0x005F
#define KEY_GRAVE           0x0060
#define KEY_LEFT_BRACE      0x007B
#define KEY_VERTICAL_BAR    0x007C
#define KEY_RIGHT_BRACE     0x007D
#define KEY_TILDE           0x007E

// Nút chức năng đặc biệt (KHÔNG trùng với DWIN.h)
#define KB_CANCEL           0x00F0
#define KB_ENTER            0x00F1
#define KB_BACKSPACE        0x00F2
#define KB_DELETE           0x00F3
#define KB_CAPS_LOCK        0x00F4
#define KB_CLEAR            0x00F5
#define KB_LEFT             0x00F7
#define KB_RIGHT            0x00F8

// ============================================================================
// LỚP DWIN_KEYBOARD
// ============================================================================

class DWIN_Keyboard {
private:
    std::string _inputBuffer;
    bool _capsLock;
    
public:
    DWIN_Keyboard() {
        _inputBuffer = "";
        _capsLock = false;
    }
    
    char keyCodeToChar(int keyCode) {
        // Nút số 0-9
        if (keyCode >= KEY_0 && keyCode <= KEY_9) {
            return '0' + (keyCode - KEY_0);
        }
        
        // Chữ cái in hoa A-Z
        if (keyCode >= KEY_A && keyCode <= KEY_Z) {
            if (_capsLock) {
                return (char)keyCode;
            } else {
                return (char)(keyCode + 32);
            }
        }
        
        // Chữ cái in thường a-z
        if (keyCode >= KEY_a && keyCode <= KEY_z) {
            if (_capsLock) {
                return (char)(keyCode - 32);
            } else {
                return (char)keyCode;
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
    
    int processKey(int keyCode) {
        // Sử dụng KB_ constants để tránh trùng với DWIN.h
        if (keyCode == KB_BACKSPACE) {
            if (_inputBuffer.length() > 0) {
                _inputBuffer.pop_back();
            }
            printf("[KEY] Backspace - Xoa 1 ky tu\n");
            return 2;
        }
        else if (keyCode == KB_DELETE) {
            printf("[KEY] Delete\n");
            return 3;
        }
        else if (keyCode == KB_CLEAR) {
            _inputBuffer = "";
            printf("[KEY] Clear - Xoa toan bo\n");
            return 4;
        }
        else if (keyCode == KB_ENTER) {
            printf("[KEY] Enter - Xac nhan: %s\n", _inputBuffer.c_str());
            return 5;
        }
        else if (keyCode == KB_CANCEL) {
            _inputBuffer = "";
            printf("[KEY] Cancel - Huy bo nhap lieu\n");
            return 6;
        }
        else if (keyCode == KB_CAPS_LOCK) {
            _capsLock = !_capsLock;
            printf("[KEY] CapsLock - %s\n", _capsLock ? "BAT (chu hoa)" : "TAT (chu thuong)");
            return 7;
        }
        else if (keyCode == KB_LEFT) {
            printf("[KEY] Left - Di chuyen con tro sang trai\n");
            return 0;
        }
        else if (keyCode == KB_RIGHT) {
            printf("[KEY] Right - Di chuyen con tro sang phai\n");
            return 0;
        }
        else {
            char c = keyCodeToChar(keyCode);
            if (c != '\0') {
                _inputBuffer += c;
                printf("[KEY] Nhap: %c | Buffer: %s\n", c, _inputBuffer.c_str());
                return 1;
            }
        }
        return 0;
    }
    
    std::string processNumberInput(int keyCode) {
        char c = keyCodeToChar(keyCode);
        
        if (c >= '0' && c <= '9') {
            _inputBuffer += c;
            return _inputBuffer;
        }
        
        if (keyCode == KB_BACKSPACE && _inputBuffer.length() > 0) {
            _inputBuffer.pop_back();
            return _inputBuffer;
        }
        
        if (keyCode == KB_CLEAR) {
            _inputBuffer = "";
            return _inputBuffer;
        }
        
        return _inputBuffer;
    }
    
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
    
    std::string getBuffer() {
        return _inputBuffer;
    }
    
    void clearBuffer() {
        _inputBuffer = "";
    }
    
    void setBuffer(std::string value) {
        _inputBuffer = value;
    }
    
    int getBufferLength() {
        return _inputBuffer.length();
    }
    
    int getBufferAsInt() {
        if (_inputBuffer.empty()) return 0;
        return std::stoi(_inputBuffer);
    }
    
    float getBufferAsFloat() {
        if (_inputBuffer.empty()) return 0.0f;
        return std::stof(_inputBuffer);
    }
    
    bool isCapsLock() {
        return _capsLock;
    }
    
    void setCapsLock(bool state) {
        _capsLock = state;
    }
    
    std::string getKeyDescription(int keyCode) {
        if (keyCode >= KEY_0 && keyCode <= KEY_9) {
            char str[2] = { (char)('0' + (keyCode - KEY_0)), '\0' };
            return std::string(str);
        }
        if (keyCode >= KEY_A && keyCode <= KEY_Z) {
            char str[2] = { (char)keyCode, '\0' };
            return std::string(str);
        }
        if (keyCode >= KEY_a && keyCode <= KEY_z) {
            char str[2] = { (char)keyCode, '\0' };
            return std::string(str);
        }
        switch(keyCode) {
            case KB_CANCEL:    return "CANCEL";
            case KB_ENTER:     return "ENTER";
            case KB_BACKSPACE: return "BACKSPACE";
            case KB_DELETE:    return "DELETE";
            case KB_CAPS_LOCK: return "CAPS_LOCK";
            case KB_CLEAR:     return "CLEAR";
            case KB_LEFT:      return "LEFT_ARROW";
            case KB_RIGHT:     return "RIGHT_ARROW";
            default: {
                char c = keyCodeToChar(keyCode);
                if (c != '\0') return std::string(1, c);
                return "UNKNOWN";
            }
        }
    }
};

#endif // DWIN_KEYBOARD_H