import time
import serial
import re
import logging

def main():
    logging.basicConfig(level=logging.DEBUG)
    ser = serial.Serial("/dev/ttyACM0", 115200, timeout=1)
    print_output = 0
    # Send reset ESP32, toggle RTS cause ESP32 go to download mode
    ser.dtr = False
    time.sleep(0.1)
    ser.dtr = True    
    while True:
        data = ser.readline()
        if print_output == 0:
            match = re.search(r"LOG START", data.decode("utf-8"))
            if match:
                print("Found the match, start to log")
                print_output = 1
        elif print_output == 1:
            match = re.search(r"LOG END", data.decode("utf-8"))
            if match:
                print_output = 0
                print("Found the match end, stop logging")
                break
        else:
            print("Should not jump to this")

        if print_output == 1 and len(data) != 0:
            logging.info(f"{data.decode('utf-8')}")

if __name__ == "__main__":
    main()

