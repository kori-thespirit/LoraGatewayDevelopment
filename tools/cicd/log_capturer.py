import serial
import re
import logging

def main():
    logging.basicConfig(level=logging.DEBUG)
    ser = serial.Serial("/dev/ttyACM0", 115200, timeout=1)
    while True:
        data = ser.readline()
        logging.info(f"{data.decode('utf-8')}")


if __name__ == "__main__":
    main()

