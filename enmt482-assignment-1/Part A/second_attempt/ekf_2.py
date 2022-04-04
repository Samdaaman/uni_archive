#
#                      efk_1.py
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
import os
import sys
os.chdir(os.path.abspath(os.path.dirname(__file__)))
sys.path.append("../")
from motion_models.models2 import *


# Top equation from https://en.wikipedia.org/wiki/Finite_difference#Multivariate_finite_differences
# df/dx = ( f(x+h) - f(x-h) ) / 2h
# df/dx^2 = ( f(x+h) - 2*f(x) + f(x-h) ) / h^2
def get_partial_of_sensor_model(x: float, h: float, model: SensorModel, degree: int) -> float:
    if degree == 1:
        return (model.h_one(x + h) - model.h_one(x - h)) / (2*h)
    elif degree == 2:
        return (model.h_one(x + h) - 2*model.h_one(x) + model.h_one(x - h)) / (h**2)
    else:
        raise NotImplementedError()


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



    # kf = KalmanFilter(dim_x=2, dim_z=3) # two output states (position and velocity) and one input for each sensor
    # kf.x = [sonar1_model.h(sonar1_data[0]), 0] # base initial state estimate off the sonar
    # kf.H = np.array([[1, 0], [1, 0], [1, 0]]) # measurement matrix
    # kf.P = np.array([[0.01, 0],[0, 0.0001]]) # process noise (filter computes non diagonal entries)
    # kf.F = np.array([[1, dt], [0, 1]]) # state transision matrix (process model)
    # kf.R = np.diag((1, 0.125, 4)) # measurement noise
    # kf.Q = Q_discrete_white_noise(dim=2, dt=dt, var=0.01) # originally  var=0.00001

    # estimates[0] = kf.x
    x = np.empty((N, 2))
    P = np.empty((N, 2, 2))

    z = np.array([
        sonar1_data,
        ir3_data,
        ir4_data,
    ]).T

    x[0][0] = sonar1_model.h_inv_one(sonar1_data[0])  # base initial position estimate off the sonar model
    x[0][0] = 0  # assume initial velocity is zero
    P[0] = np.eye(2) * 0.1 # covariance of the initial belief
    A = np.array([[1, 0], [0, 0]])  # state transision matrix (df/dx)
    B = np.array([dt, 1])
    u = velocity_command
    Q = np.array([[0.0001, 0], [0, 0.0001]]) # process noise variance (Var[W]) TODO tune

    for k in range(1,N):
        # This below code was implemented based on
        # https://en.wikipedia.org/wiki/Extended_Kalman_filter#Discrete-time_predict_and_update_equations
        # When comparing to the notes H_k=C and R_k=SUM_v and A=F from lecture 11

        # -------------------------------------------------------------
        # Predict
        # -------------------------------------------------------------
        x_k_neg = A @ x[k-1] + B * u[k]
        P_k_neg = A @ P[k-1] @ A.T + Q

        # -------------------------------------------------------------
        # sensor model
        # -------------------------------------------------------------
        var_z_1_k = (5/3 * x_k_neg[0]) ** 2
        var_z_2_k = 0.25 ** 2
        var_z_3_k = 0.4 ** 2 if x_k_neg[0] > 1 else 1.5**2
        
        # measurement noise variance matrix (SUM_v from the notes)
        R_k = np.diag((
            var_z_1_k,
            var_z_2_k,
            var_z_3_k,
        ))

        # sensor model matrix (C matrix from notes)
        H_k = np.array(list([
                get_partial_of_sensor_model(x_k_neg[0], 0.001, model, 1),
                # get_partial_of_sensor_model(x_k_neg[0], 0.001, model, 2),
                0,
            ] for model in [
                sonar1_model,
                ir3_model,
                ir4_model,
            ]))

        

        # -------------------------------------------------------------
        # Update
        # -------------------------------------------------------------
        y_k = z[k] - np.array([
            sonar1_model.h_one(x[k-1][0]),
            ir3_model.h_one(x[k-1][0]),
            ir4_model.h_one(x[k-1][0])
        ])

        for i, model in enumerate([
            sonar1_model,
            ir3_model,
            ir4_model,
        ]):
            speed = abs(x_k_neg[0] - model.h_inv_one(z[k][i], x_k_neg[0])) / dt
            if speed > 5:
                y_k[i] = 0

        S_k = H_k @ P_k_neg @ H_k.T + R_k
        K_k = P_k_neg @ H_k.T @ np.linalg.inv(S_k)
        x[k] = x_k_neg + K_k @ y_k
        P[k] = (np.eye(len(K_k)) - K_k @ H_k) @ P_k_neg
        

    fig, axs = plt.subplots()
    # Plot actual distance
    axs.plot(time, distances, label='distances')
    axs.plot(time, x[:, 0], 'b', label=f'estimates')
    axs.set_xlabel('Time (t)')
    axs.set_ylabel('Range (m)')
    axs.legend()

    # fig, axs = plt.subplots()
    # actual_speeds = np.zeros(N)
    # for i in range(1, N):
    #     actual_speeds[i] = (distances[i] - distances[i-1]) / dt
    # axs.plot(time, actual_speeds, '.', alpha=0.2, label='actual_speed')
    # axs.plot(time, speeds[:, 0], '.', alpha=0.2, label='sonar1_speed')
    # # axs.plot(time, speeds[:, 1], '.', alpha=0.2, label='ir3_speed')
    # # axs.plot(time, speeds[:, 2], '.', alpha=0.2, label='ir4_speed')
    # axs.set_xlabel('Time (t)')
    # axs.set_ylabel('Speed (m/s)')
    # axs.legend()

    # fig, axs = plt.subplots()
    # axs.plot(time, ir3_data, '.', alpha=0.2, label='ir3_data')
    # axs.plot(time, ir3_model.h(distances), label='ir3_ideal')
    # axs.legend()
    # axs.set_xlabel('Time (t)')
    # axs.set_ylabel('Measured (raw sensor units)')
    
    plt.show()


# Run program
if __name__ == '__main__':
    main()
