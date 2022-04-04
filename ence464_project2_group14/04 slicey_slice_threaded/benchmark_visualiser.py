


import os
import sys
import time 
import numpy as np
import datetime
import operator
import matplotlib.pyplot as plt
from matplotlib.ticker import FormatStrFormatter

sys.path.append("../")
BENCHMARK_PATH = "./benchmarks"

class BenchmarkResults():

    def __init__(self, title, notes=""):
        self.title = title
        self.notes = notes
        
        self.n = []
        self.times = []
        self.time_901 = 0

    def plot_performance(self, ax=None, title="", labels=True, label=""):
        # Define new axes if none given
        if ax is None:
            ax = plt.gca()
        
        # Plot line
        line, = ax.plot(self.n,self.times, 
                                            #color='#f32042', 
                                            linewidth=2, 
                                            linestyle=(0, (5, 2, 1, 2)), 
                                            dash_capstyle='round',
                                            alpha=1.0,
                                            label=label)

        # Add styling
        #ax.set_title(title)
        if (labels):
            ax.set_xlabel('Cube size (n)')
            ax.set_ylabel('Time (s)')
            ax.yaxis.set_major_formatter(FormatStrFormatter('%.2f'))
            #ax.yaxis.set_major_locator(plt.MaxNLocator(3))
        ax.legend(loc='best')
  
        return line


def extract_results_from_textfile(filename):

    with open(filename, "r") as benchmark_file:
        benchmark_file_data = benchmark_file.readlines()
        
        currResults = BenchmarkResults(benchmark_file_data[1].strip(), benchmark_file_data[0].strip())

        for line in benchmark_file_data[4:]:
            line = line.strip().split(',')
            currResults.n.append(int(line[0]))
            currResults.times.append(float(line[1]))

        currResults.time_901 = currResults.times[-1]

        return currResults


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

    # Get files:
    filenames = get_filenames(BENCHMARK_PATH)

    benchmarkResults = []
    for f in filenames:
        benchmarkResults.append(extract_results_from_textfile(f))
        

    # Reorder in terms of speed
    benchmarkResults.sort(key=operator.attrgetter('time_901'), reverse=True)

    # Plots setup:
    fig1, axes = plt.subplots(nrows=1, ncols=1)
    # fig1, axes = plt.subplots(nrows=4, ncols=2, figsize=(12, 4))

    # Some plots
    for res in benchmarkResults:
        res.plot_performance(axes, title=res.title, label=res.notes[5:] + f" 901n: {res.times[-1]:.3f}s")
    

    # Performance comparison
    



if __name__ == '__main__':
    # Path for my local system
    main()
    plt.show()
    