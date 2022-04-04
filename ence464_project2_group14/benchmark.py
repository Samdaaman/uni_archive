import csv
from datetime import datetime
import os
import signal
import subprocess
import sys
import time
from typing import List, Optional, Tuple

ITERATIONS = 500
TIMEOUT = 60 * 15  # timeout after 15min
#N_ARRAY = [9, 11, 15, 21, 31, 51, 101, 151, 201, 301, 401, 501, 601, 701, 801, 901]
N_ARRAY = [301]

# Alarm used for Timeout as in https://stackoverflow.com/a/1191537
class Alarm(Exception):
    pass

def alarm_handler(signum, frame):
    raise Alarm

signal.signal(signal.SIGALRM, alarm_handler)


def time_it(n: int):
    time.sleep(3) # just chill out for a bit

    signal.alarm(TIMEOUT)  # setup timeout
    
    proc = subprocess.Popen(['./main.o', f'{n}', f'{ITERATIONS}'], stdout=subprocess.PIPE)

    try:
        stdout_data, stderr_data = proc.communicate()
    except Alarm:
        proc.kill()
        time_total = -1

    else:
        time_str = stdout_data.decode()
        try:
            nano_seconds = int(time_str.split(' ')[0])
            seconds = int(time_str.split(' ')[1])
            time_total = seconds + nano_seconds / 10**9
        except Exception as ex:
            print(f'while decoding "{time_str}" CAUGHT EXCEPTION: {ex}')
            time_total = -2

    signal.alarm(0)  # reset the alarm

    return time_total
    

def benchmark_choice(folder_name: str, gcc_command: str, notes: str, no_output: bool):
    print('-' * 50)
    print(f'Benchmarking {folder_name}, Notes: {notes}')
    os.chdir(folder_name)

    if os.system(gcc_command) != 0:
        print('gcc failed :(')
        return

    try:
        output = []
        timed_out = False
        for n in N_ARRAY:
            if timed_out:
                time_total = -1
            else:
                time_total = time_it(n)
                if time_total == -1:
                    print(f'{n}x{n}x{n} Timed out')
                    timed_out = True
            print(f'[{n}x{n}x{n}]:'.ljust(15) + f'{time_total}')
            output.append((n, time_total))
    except KeyboardInterrupt:
        pass

    if not no_output:
        with open(f'../benchmarks/{notes}_{datetime.now().strftime("%Y_%m_%d_%H:%M:%S")}.txt', 'w') as fh:
            fh.write(f'Notes: {notes}\n')
            fh.write(f'Benchmarking "{folder_name}" with {ITERATIONS} iterations\n\n')
            fh.write('n,time\n')
            writer = csv.writer(fh)
            writer.writerows(output)

    print('Done')

def main():
    if len(sys.argv) >= 4:
        folder_name = sys.argv[1].rstrip('/')
        gcc_command = sys.argv[2]
        notes = sys.argv[3]
        if len(sys.argv) == 5 and sys.argv[4] == 'no-output':
            no_output = True
        else:
            no_output = False
        benchmark_choice(folder_name, gcc_command, notes, no_output)
        exit(0)

    else:
        print('Usage: python3 benchmark.py "<folder_name>" "<gcc command>" "<notes>" [optional]"no-output"')
        exit(1)

if __name__ == '__main__':
    main()
