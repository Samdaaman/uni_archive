import serial
import time
import matplotlib.pyplot as plt
from typing import Optional


def handshake() -> Optional[serial.Serial]:
    try:
        ser = serial.Serial('COM8', 19200, timeout=3)
        time.sleep(0.5)
        return ser
    except Exception as e:
        print(str(e))


def main():
    ser = handshake()
    if ser is None:
        print('Failed to complete initial handshake')
        exit(1)
    else:
        try:
            while True:
                line = ser.readline()
                line_arr = line.decode('utf-8').split(',')
                x = int(line_arr[0])
                y = int(line_arr[1])
                plt.clf()
                plt.axis([0, 1023, 0, 1023])
                plt.scatter(x, y)
                plt.pause(0.05)
                print(f'{x}, {y}')
        finally:
            exit(0)



if __name__ == "__main__":
    main()
