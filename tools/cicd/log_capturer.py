import serial
import re
import logging

def main():
    logging.basicConfig(level=logging.DEBUG)
    ser = serial.Serial("/dev/ttyACM0", 115200, timeout=1)
    print_output = 0
    while True:
        data = ser.readline()
        logging.info(f"{data.decode('utf-8')}")
        if print_output == 0:
            match = re.search(r"LOG START", data.decode("utf-8"))
            if match:
                print("Found the match, start to log")
                print_output = 1
        elif print_output == 1:
            match = re.search(r"LOG STOP", data.decode("utf-8"))
            if match:
                print_output = 0
                print("Found the match end, stop logging")
                print(str_monitor_end)
                break
        else:
            print("Should not jump to this")

        if print_output == 1:
            print(data)

if __name__ == "__main__":
    main()

