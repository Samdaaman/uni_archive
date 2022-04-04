import tkinter as tk
from typing import List, Optional, Callable
import serial
import time
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import threading

DEBUG = False
VISION_ENUM = 'V_FOUND_NONE, V_FOUND_WEIGHT_LEFT_FAR, V_FOUND_WEIGHT_LEFT_CLOSE, V_FOUND_WEIGHT_RIGHT_FAR, V_FOUND_WEIGHT_RIGHT_CLOSE, V_FOUND_WALL_LEFT, V_FOUND_WALL_RIGHT, V_FOUND_WALL_FRONT'
STATE_ENUM = 'S_BOUNCING_AROUND, S_SCANNING, S_DRIVING_TOWARDS, S_PICKING_UP_LEFT, S_PICKING_UP_RIGHT, S_AVOIDING'
SENSORS = ['Top Left', 'Top right', 'Bottom left', 'Bottom right']
SENSOR_PLOT_X_LIMIT = 100
SENSOR_PLOT_Y_LIMIT = 1500
LEFT_PANEL_WIDTH = 200
PADDING = 5


class EnumItem:
    def __init__(self, value: int, name: str):
        self.value = value
        self.name = name


def generate_enum_items(str_code: str) -> List[EnumItem]:
    str_list = str_code.split(', ')
    return [EnumItem(value=i, name=str_list[i]) for i in range(len(str_list))]


root = tk.Tk()
frame_listbox = tk.Frame(root, width=LEFT_PANEL_WIDTH)
frame_listbox.pack(side=tk.LEFT, fill=tk.Y)
frame_listbox.pack_propagate(0)
frame_plot = tk.Frame(root)
frame_plot.pack(side=tk.RIGHT, fill=tk.Y)

listbox_vision = tk.Listbox(frame_listbox)
listbox_vision.pack(side=tk.TOP, fill=tk.BOTH, expand=1, padx=PADDING, pady=PADDING)
listbox_state = tk.Listbox(frame_listbox)
listbox_state.pack(side=tk.BOTTOM, fill=tk.BOTH, expand=1, padx=PADDING, pady=PADDING)

fig, ax = plt.subplots(figsize=(100, 100), dpi=100)
canvas = FigureCanvasTkAgg(fig, master=frame_plot)
canvas.get_tk_widget().pack(fill=tk.BOTH, expand=1)
sensor_data = []  # type: List[List[int]]
x_list = [i for i in range(-SENSOR_PLOT_X_LIMIT, 0)]
plotting_lock = threading.Lock()
plt.ion()
drawn_legend = False
data_plotted = False
counter = 0


def process_sensor_data(line: str):
    global counter
    global drawn_legend
    global data_plotted

    current_data = ([int(i) for i in line.split('=')[1].split(',')])
    for i in range(len(current_data)):
        if len(sensor_data) <= i:
            sensor_data.append([current_data[i]])
        else:
            sensor_data[i].append(current_data[i])
        sensor_data[i] = sensor_data[i][-SENSOR_PLOT_X_LIMIT:]

    if counter % 10 == 0:
        counter = 0
        plt.cla()
        ax.axis([-SENSOR_PLOT_X_LIMIT, 0, 0, SENSOR_PLOT_Y_LIMIT])
        plt.gca().set_prop_cycle(None)
        for i in range(len(sensor_data)):
            if i < len(SENSORS):
                label = SENSORS[i]
            else:
                label = 'Unknown'
            if len(sensor_data[i]) > 1:
                ax.plot(x_list[:len(sensor_data[i])], sensor_data[i][-SENSOR_PLOT_X_LIMIT::], label=label)
                data_plotted = True
        ax.legend()
        canvas.draw()
    counter += 1


class Command:
    enum_name: str
    current_value: Optional[int]
    mapped_listbox: Optional[tk.Listbox]
    enum_map: Optional[List[EnumItem]]
    process_line: Callable[[str], None]

    def __init__(self, enum_name, mapped_listbox=None, enum_map=None,
                 process_line: Optional[Callable[[str, bool], None]] = None):
        self.enum_name = enum_name
        self.current_value = None
        if process_line is None:
            self.mapped_listbox = mapped_listbox
            self.mapped_listbox.pack()
            self.enum_map = enum_map
            self.process_line = lambda line: self.update_from_line(line)
        else:
            self.process_line = lambda line: process_line(line)

    def update_from_line(self, line: str) -> None:
        new_value = int(line.split('=')[1])
        if self.current_value != new_value:
            self.current_value = new_value
            enum_name = 'Unknown'
            for enum_item in self.enum_map:
                if enum_item.value == new_value:
                    enum_name = enum_item.name
                    break
            # self.mapped_listbox.delete(0)
            self.mapped_listbox.insert(0, enum_name)
            print('listbox update')


vision = Command('vision', mapped_listbox=listbox_vision, enum_map=generate_enum_items(VISION_ENUM))
state = Command('state', mapped_listbox=listbox_state, enum_map=generate_enum_items(STATE_ENUM))
sensors = Command('sensors', process_line=process_sensor_data)


class DataSource:
    def __init__(self):
        if DEBUG:
            self.source = open('ui_v1_test.txt', 'r')
            self.debug_stuff_left = True
        else:
            self.debug_stuff_left = False
            self.line_buffer = b''  # type: bytes
            self.lines_buffer = []  # type: List[str]
            try:
                self.source = serial.Serial('COM8', 9600)
                time.sleep(2)
            except Exception as ex:
                print(f'Failed to establish serial connection, {ex}')
                time.sleep(3)
                print('Retrying...')

    def stuff_left(self):
        if DEBUG:
            return self.debug_stuff_left
        else:
            return len(self.lines_buffer) or self.source.in_waiting

    def read_line(self, wait_if_buffer_empty=False) -> str:
        if DEBUG:
            line = self.source.readline()
            return line.replace('\r', '').replace('\n', '') if isinstance(line, str) else ''

        else:
            if len(self.lines_buffer) > 0:
                return self.lines_buffer.pop(0)
            else:
                if wait_if_buffer_empty:
                    time.sleep(0.1)
                data = self.line_buffer + self.source.read(self.source.in_waiting)
                while data == b'':
                    time.sleep(0.1)
                    data = self.line_buffer + self.source.read(self.source.in_waiting)
                self.line_buffer = b''
                lines_raw = data.split(b'\r\n')

                for i in range(len(lines_raw)):
                    try:
                        line = lines_raw[i].decode('utf-8')
                        if i == len(lines_raw) - 1:
                            self.line_buffer = lines_raw[i]
                        else:
                            self.lines_buffer.append(line)
                    except UnicodeDecodeError:
                        pass
                if len(self.lines_buffer) > 0:
                    print(f'Read {len(self.lines_buffer)} lines')
                return '' if len(self.lines_buffer) == 0 else self.lines_buffer.pop(0)


data_source = DataSource()


def update_data():
    if not plotting_lock.acquire(False):
        # we can't trigger from a plt flush events so just ignore it
        root.after(5, update_data)
        return

    data_chunk = []
    while data_source.stuff_left():
        try:
            data_chunk.append(data_source.read_line())
        except UnicodeDecodeError:
            pass

    if len(data_chunk) > 0:
        print(f'Received lines {len(data_chunk)}')

    for i in range(len(data_chunk)):
        line = data_chunk[i]
        try:
            for command in [vision, state, sensors]:  # type: Command
                if line.startswith(command.enum_name):
                    # print(f'Processing line {line}')
                    if not DEBUG:
                        command.process_line(line)
                    else:
                        counter = 0
                        while counter < 20:
                            command.process_line(line)
                            counter += 1
                        if DEBUG:
                            time.sleep(1)
                    break
            else:
                print(f'Command missing for line: {line}')

        except Exception as ex:
            print(f'Error parsing line: "{line}"\n{ex}')
            final = True
    plotting_lock.release()
    root.after(10, update_data)


# root.geometry('%dx%d+%d+%d' % (root.winfo_screenwidth(), root.winfo_screenheight(), 0, 0))
# root.attributes('-fullscreen', True)
root.state('zoomed')
root.after(0, update_data)
root.mainloop()
