import numpy as np
import os
from typing import List, Tuple

import matplotlib.pyplot as plt


def extract_results_from_textfile(filename) -> Tuple[str, List[Tuple[int, int]]]:
    with open(filename, "r") as benchmark_file:
        benchmark_file_lines = benchmark_file.readlines()
        notes = benchmark_file_lines[0].split('Notes: ')[1].rstrip()
        results = []

        for line in benchmark_file_lines[4:]:
            line = line.strip().split(',')
            n = int(line[0])
            time = float(line[1])
            results.append((n, time))

        return notes, results
        


# Returns list of all benchmark tests
def get_filenames(directory):
    filenames = []
    for filename in os.listdir(directory):
        if filename.endswith(".txt"):
            filenames.append(os.path.join(directory, filename))
        else:
            continue
    return filenames


def main():
    filenames = get_filenames('benchmarks')
    results_dict = {}

    for filename in filenames:
        notes, results = extract_results_from_textfile(filename)

        if results_dict.get(notes) is None:
            results_dict[notes] = {}
        for n, time in results:
            if results_dict[notes].get(n) is None:
                results_dict[notes][n] = []
            if time != -1:
                results_dict[notes][n].append(time)


    plt.figure()
    for title in sorted(results_dict.keys()):
        x = []
        y = []
        std_devs = []

        for n in results_dict[title].keys():
            times_raw = results_dict[title][n]  # type: list
            times = []

            # Filter -1 times
            for time_raw in times_raw:
                if time_raw > 0:
                    times.append(time_raw)

            if len(times) > 0:
                average_time = np.average(times)
                std_dev = np.std(times, ddof=1)  # ddof=1 for sample std deviation #https://numpy.org/doc/stable/reference/generated/numpy.std.html
                std_devs.append(std_dev)
                x.append(n)
                y.append(average_time)
                print(f'{title},{n},{average_time},{std_dev}')
        
        plt.errorbar(x, y, np.multiply(std_devs, 3), fmt='--', marker='.', ecolor='lightgray', capsize=5, label=title)
        plt.xlabel('n (number of points for each side length of cube)')
        plt.ylabel('Time')

    plt.legend()
    plt.show()

if __name__ == '__main__':
    main()
