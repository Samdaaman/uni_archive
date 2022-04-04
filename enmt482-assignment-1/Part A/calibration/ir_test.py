# Michael P. Hayes UCECE, Copyright 2018--2019
import numpy as np
from matplotlib.pyplot import subplots, show
from numpy.linalg import lstsq


def model(r, k):
    # This model is not that good!  
    return k[0] + k[1] / (r + k[2])

def model_nonlinear_least_squares_fit(r, v, iterations=5):
    N = len(r)
    A = np.ones((N, 3))
    k = np.zeros(3)

    for i in range(iterations):
        # Calculate Jacobians for current estimate of parameters.
        for n in range(N):
            A[n, 0] = 1 # this is implied by the creation of A but was left for readability
            A[n, 1] = 1 / (r[n] + k[2])
            A[n, 2] = -k[1] / (r[n] + k[2])**2
            
        # Use least squares to estimate the parameters.
        deltak, res, rank, s = lstsq(A, v - model(r, k), rcond=None)
        k += deltak
    return k


def ir3_hist_demo1_plot(distance=1.0, width=0.5):

    # Load data
    filename = 'calibration.csv'
    data = np.loadtxt(filename, delimiter=',', skiprows=1)

    # Split into columns
    _,time,distance,velocity_command,raw_ir1,raw_ir2,raw_ir3,raw_ir4,sonar1,sonar2 = data.T

    data = raw_ir3

    k = model_nonlinear_least_squares_fit(distance, data, 100)
    data_fit = model(distance, k)

    error = data - data_fit

    error_max = 0.2
    zmin = -error_max
    zmax = error_max

    dmin = distance - 0.5 * width
    dmax = distance + 0.5 * width    
    
    m = (distance < dmax) & (distance > dmin)

    bins = np.linspace(zmin, zmax, 100)

    smean = error[m].mean()
    sstd = error[m].std()

    fig, axs = subplots(3)
    axs[0].plot(distance, data, '.', alpha=0.2)
    axs[0].plot(distance, data_fit)    
    axs[0].set_xlabel('Distance (m)')
    axs[0].set_ylabel('Measurement')
    axs[0].set_ylim(0, 4)
    # axs[0].axvspan(dmin, dmax, color='C1', alpha=0.4)
    axs[0].set_title('mean = %.2f  std = %.2f' % (smean, sstd))

    axs[1].plot(distance, data - data_fit, '.', alpha=0.2)
    axs[1].set_xlabel('Distance (m)')
    axs[1].set_ylabel('Error')
    axs[1].set_ylim(-0.5, 0.5)
    # axs[1].axvspan(dmin, dmax, color='C1', alpha=0.4)
    
    axs[2].hist(error[m], bins=bins, color='C2')
    axs[2].set_xlabel('Error')
    axs[2].set_ylabel('Count')    
    axs[2].set_xlim(zmin, zmax)

    show()

if __name__ == '__main__':
    ir3_hist_demo1_plot()
