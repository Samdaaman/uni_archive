import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from collections import deque
import numpy as np
import sys
import serial
import matplotlib

matplotlib.use('Tkagg')

BUFFER = 50


def main(fh):
    num_values = 3
    lines = [None] * num_values
    queues = [deque(np.zeros(BUFFER)) for i in range(num_values)]
    x = np.linspace(-BUFFER + 1, 0, BUFFER)

    while True:
        try:
            try:
                line = fh.readline().decode().strip()
            except UnicodeDecodeError:
                continue
            try:
                values = [int(i) for i in line.split(',')]
                assert len(values) == num_values
            except (ValueError, AssertionError):
                print(line)
                continue

            for i in range(num_values):
                queues[i].popleft()
                queues[i].append(values[i])

                
                if lines[i] is None:
                    lines[i], = plt.plot(x, queues[i])
                else:
                    lines[i].set_ydata(queues[i])

            plt.legend([f'value{i}' for i in range(num_values)])

            yticks = np.linspace(-180, 360, 49).astype(np.int32)
            plt.yticks(yticks, yticks)
            plt.pause(0.01)

            if not plt.get_fignums():
                exit(0)

        except KeyboardInterrupt:
            exit(0)

if __name__ == '__main__':
    # main(sys.stdin)
    # TODO call main with a serial object instead (use pyserial)
    main(serial.Serial('/dev/ttyACM1', baudrate=9600))
