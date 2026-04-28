import serial
import time

PORT = "/dev/ttyACM0"
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=1)


def send_cmd(cmd):
    print(f"Send: {cmd}")
    ser.write((cmd + "\n").encode())

    time.sleep(0.1)

    if ser.in_waiting:
        resp = ser.read(ser.in_waiting).decode(errors="ignore")
        print("Recv:", resp)


try:
    while True:
        cmd = input("Enter cmd (IDLE/START/RUN/STOP): ").strip()
        send_cmd(cmd)

except KeyboardInterrupt:
    print("Exit")
    ser.close()
