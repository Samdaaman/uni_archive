#
#                      kalman.py
#
#
#        >(')____,  >(')____,  >(')____,  >(')____,  >(') ___,
#         (` =~~/    (` =~~/    (` =~~/    (` =~~/    (` =~~/
#    ~^~^`---'~^~^~^`---'~^~^~^`---'~^~^~^`---'~^~^~^`---'~^~^~
#
#************************************************************************#


#  Authors:        Zach Preston        zsp10
#                  Sam Hogan           sho116
#
#  Date created: 15th August 2021
#  Date Last Modified: 
#
#  Module Description:
#  


import numpy as np
from numpy.linalg import lstsq
import matplotlib.pyplot as plt

from filterpy.kalman import KalmanFilter
from filterpy.common import Q_discrete_white_noise

# Allow imports from other directories
import sys
sys.path.append("../")
from motion_models.models import *


def main():

    # Import training data
    filename = 'training1.csv'
    _, time, distances, velocity_command, ir1_data, ir2_data, ir3_data, ir4_data, sonar1_data, sonar2_data = np.loadtxt(filename, delimiter=',', skiprows=1).T
    N = len(time)
    dt = time[1] - time[0]

    # Define models
    # sonar1_model = SonarModel([1.007, 0.01]) # TODO make this a quadratic model
    sonar1_model = SonarModel([0.99, 0]) # TODO make this a quadratic model

    # ir1_model = IRBetterModel([-0.25, 0.14, 0.2, 0.005])
    # ir2_model = IRBetterModel([-0.25, 0.14, 0.2, 0.005])
    ir3_model = IRBetterModel([-0.00724208, 0.17960182, 0.2964516, -0.00167510])
    ir4_model = IRThreePartModel([
        -1.51507457,
        -3.39024013,
        20.5436498,
        121659.198,
        34823.3509,
        -32.3,
        1.6,
        -1.66510526,
        320902572,
        9066691.96,
        1.07535534,
        -0.0610574,
        0.02014625,
        2.14001523,
        0.24313071
    ], [0.365, 0.65])

    sonar1_distances = sonar1_model.h_inv(sonar1_data)
    ir3_distances = np.empty(N)
    ir4_distances = np.empty(N)
    
    ir3_distances[0] = ir3_model.h_inv(ir3_data[0:1], sonar1_distances[0:1])[0]
    ir4_distances[0] = ir4_model.h_inv(ir4_data[0:1], sonar1_distances[0:1])[0]

    kf = KalmanFilter(dim_x=2, dim_z=3) # two output states (position and velocity) and one input for each sensor
    kf.x = [sonar1_model.h(sonar1_data[0]), 0] # base initial state estimate off the sonar
    kf.H = np.array([[1, 0], [1, 0], [1, 0]]) # measurement matrix
    kf.P = np.array([[0.01, 0],[0, 0.0001]]) # process noise (filter computes non diagonal entries)
    kf.F = np.array([[1, dt], [0, 1]]) # state transision matrix (process model)
    kf.R = np.diag((1, 0.125, 4)) # measurement noise
    kf.Q = Q_discrete_white_noise(dim=2, dt=dt, var=0.01) # originally  var=0.00001

    estimates = np.empty((N, 2))
    estimates[0] = kf.x
    for i in range(1,N):
        kf.predict()

        ir3_distances[i] = ir3_model.h_inv(ir3_data[i:i+1], (sonar1_distances[i],))[0]
        ir4_distances[i] = ir4_model.h_inv(ir4_data[i:i+1], (sonar1_distances[i],))[0]

        kf.update((
            sonar1_distances[i],
            ir3_distances[i],
            ir4_distances[i],
        ))
        estimates[i] = kf.x

    fig, axs = plt.subplots()
    # Plot actual distance
    axs.plot(time, distances, label='distances')
    # Plot sensor distances
    axs.plot(time, sonar1_distances, '.', alpha=0.2, label='sonar1_distances')
    axs.plot(time, ir3_distances, '.', alpha=0.2, label='ir3_distances')
    axs.plot(time, ir4_distances, '.', alpha=0.2, label='ir4_distances')
    
    axs.plot(time, estimates[:, 0], 'b', label=f'estimates')
    axs.set_xlabel('Time (t)')
    axs.set_ylabel('Range (m)')
    axs.legend()

    plt.show()


# Run program
if __name__ == '__main__':
    main()
