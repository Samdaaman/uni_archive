
import os
import sys
import time 
import numpy as np
import datetime
import matplotlib.pyplot as plt

sys.path.append("../")

def main():
    filename = 'main'
    iterations=300
    n_tests = [21, 31, 51, 101, 151, 201, 301, 401, 501, 601, 701, 801, 901]
    #n_tests = n_tests[:4]
    times = []

    # Build program
    os.system(f'gcc -o {filename}.o -O3 -Ofast {filename}.c -lpthread')

    for i, n in enumerate(n_tests):
        os.system('sleep 1')
        
        start_time = datetime.datetime.now()

        os.system(f'./{filename}.o {n} {iterations} > /dev/null 2> /dev/null')

        end_time = datetime.datetime.now()

        delta_t = (end_time - start_time).total_seconds()
        times.append(delta_t)
        
        os.system(f'echo "Test n={n} iter={iterations} | t={delta_t}s"')

    plot_n_test_results(n_tests, times, iterations)

def plot_n_test_results(test_n_vals, times, iterations):

    # determine metadata
    num_rows, num_cols = 1, 1

    fig, ax = plt.subplots(figsize=(12.8, 7.5))
    fig.tight_layout()
    
    # Plot original image
    plt.subplot(num_rows, num_cols,1)
    plt.plot(test_n_vals, times,
                            color='#f32042', 
                            linewidth=2, 
                            linestyle="-", 
                            marker='o',
                            dash_capstyle='round',
                            alpha=1.0,)
    plt.xlabel('Cube length n')
    plt.ylabel('Time (s)')
    plt.title(f'Poisson time complexity for {iterations} iterations')

    plt.show()




if __name__ == '__main__':
    # Path for my local system
    main()