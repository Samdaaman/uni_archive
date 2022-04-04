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

    ir1_model = IRBetterModel([-0.25, 0.14, 0.2, 0.005])
    ir2_model = IRBetterModel([-0.25, 0.14, 0.2, 0.005])
    ir3_model = IRBetterModel([-0.00724208, 0.17960182, 0.2964516, -0.00167510])
    # ir4_model = TODO

    sonar1_distances = sonar1_model.h_inv(sonar1_data)
    ir1_distances = ir1_model.h_inv(ir1_data, sonar1_distances)
    ir2_distances = ir2_model.h_inv(ir2_data, sonar1_distances)
    ir3_distances = ir3_model.h_inv(ir3_data, sonar1_distances)
    
    kf = KalmanFilter(dim_x=2, dim_z=4) # two output states (position and velocity) and one input for each sensor
    kf.x = [sonar1_model.h(sonar1_data[0]), 0] # base initial state estimate off the sonar
    kf.H = np.array([[1, 0], [1, 0], [1, 0], [1, 0]]) # measurement matrix
    kf.P = np.array([[0, 0],[0, 0]]) # process noise (filter computes non diagonal entries)
    kf.F = np.array([[1, dt], [0, 1]]) # state transision matrix (process model)
    kf.R = np.diag((2, 16, 16, 16)) # measurement noise
    kf.Q = Q_discrete_white_noise(dim=2, dt=dt, var=0.001)

    estimates = np.empty((N, 2))
    estimates[0] = kf.x
    for i in range(1,N):
        kf.predict()
        kf.update((
            sonar1_distances[i],
            ir1_distances[i],
            ir2_distances[i],
            ir3_distances[i],
        ))
        estimates[i] = kf.x

    fig, axs = plt.subplots()
    axs.plot(time, estimates[:, 0], label=f'estimates')
    axs.plot(time, distances, label='distances')
    axs.plot(time, sonar1_distances, '.', alpha=0.2, label='sonar1_distances')
    axs.plot(time, ir1_distances, '.', alpha=0.2, label='ir1_distances')
    axs.plot(time, ir2_distances, '.', alpha=0.2, label='ir2_distances')
    axs.plot(time, ir3_distances, '.', alpha=0.2, label='ir3_distances')
    axs.set_xlabel('Time (t)')
    axs.set_ylabel('Range (m)')
    axs.legend()

    plt.show()

    # sonar1_model = SonarModel([1.007, 0.01]) # TODO make this a quadratic model
    # ir1_model = IRBetterModel([-0.17586071, 0.15450938, 0.20772258, 0.01756693])
    # ir2_model = IRBetterModel([-0.17533161, 0.15460977, 0.20662386, 0.01680488])
    # ir3_model = IRBetterModel([-0.00724208, 0.17960182, 0.2964516, -0.00167510])
    
    # # Extract distances by inverting models

#     # Setup filter
#     f = KalmanFilter (dim_x=2, dim_z=1)

#     f.x = np.array([[2.],    # position
#                 [0.]])   # velocity

#     #Define the state transition matrix
#     f.F = np.array([[1.,1.],
#                     [0.,1.]])

#     # Define the measurement function
#     f.H = np.array([[1.,0.]])

#     #Define the covariance matrix (P already contains np.eye(dim_x), and just multiply by the uncertainty)
#     #f.P *= 1000.
#     f.P = np.array([[1000.,    0.],
#                     [   0., 1000.] ])

#     #  assign the measurement noise
#     f.R = 5 # or np.array([[5.]])

#     # print n

#     k_estimates = []
#     for n in range(N):
#         # z = np.array([[sonar1_distances[n]],
#         #             [ir1_distances[n]]
#         #             ])
#         # print(z.shape)
#         z = ir1_distances[n]

#         f.predict()
#         f.update(z)
#         k_estimates.append(f.x[0])
#         #print(f"Estimate: {f.x[0]} | Actual: {distances[n]}")

#     # Plot results
#     plot_kalman(time, distances, k_estimates, ir1_distances)

# def plot_kalman(time, distance, k_estimate, s1_estimate):
    
#     fig, axs = plt.subplots(1, 1)
    
#     axs.plot(time,s1_estimate, ".",label="Sensor Inv",
#                                             alpha=0.2, 
#                                             color='#f32042', 
#                                             linewidth=1, 
#                                             #linestyle=(0, (5, 2, 1, 2)), 
#                                             #dash_capstyle='round',                                       
#                                             )
#     axs.plot(time,k_estimate, label="Kalman",
#                                             alpha=0.2, 
#                                             color='#009922', 
#                                             linewidth=1, 
#                                             #linestyle=(0, (5, 2, 1, 2)), 
#                                             #dash_capstyle='round',                                       
#                                             )

#     axs.plot(time,distance, label="Actual")
#     axs.set_xlabel('Time (t)')
#     axs.set_ylabel('Range (m)')
#     axs.legend()
#     plt.show()




if __name__ == '__main__':
    main()
