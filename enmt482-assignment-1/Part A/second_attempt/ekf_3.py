#
#                      efk_3.py
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
import matplotlib.pyplot as plt

# Allow imports from other directories
import os
import sys
os.chdir(os.path.abspath(os.path.dirname(__file__)))
sys.path.append("../")
from motion_models.models2 import *


# Top equation from https://en.wikipedia.org/wiki/Finite_difference#Multivariate_finite_differences
# df/dx = ( f(x+h) - f(x-h) ) / 2h
def get_partial_derivative_of_sensor_model(x: float, h: float, model: SensorModel) -> float:
    return (model.h_one(x + h) - model.h_one(x - h)) / (2*h)


def main():
    # Import training data
    filename = 'training1.csv'
    _, time, distances, velocity_command, ir1_data, ir2_data, ir3_data, ir4_data, sonar1_data, sonar2_data = np.loadtxt(filename, delimiter=',', skiprows=1).T
    N = len(time)
    dt = time[1] - time[0]

    # Define models (fitted curves go brrr)
    sonar1_model = SonarModel([0.99, 0])
    ir3_model = IRBetterModel([-0.00724208, 0.17960182, 0.2964516, -0.00167510])
    ir4_model = IRThreePartModel([-1.51507457, -3.39024013, 20.5436498, 121659.198, 34823.3509, -32.3, 1.6, -1.66510526, 320902572, 9066691.96, 1.07535534, -0.0610574, 0.02014625, 2.14001523, 0.24313071], [0.365, 0.65])

    # Initialisations
    x = np.empty((N, 2))                          # state vector (x, x_dot) for all time instances
    P = np.empty((N, 2, 2))                       # Covariance pf the state estimate vector
    u = velocity_command                          # control input
    Q = np.array([[0.000001, 0], [0, 0.0001]])    # process noise variance (Var[W]) TODO tune

    # sensor data vector
    z = np.array([
        sonar1_data,
        ir3_data,
        ir4_data,
    ]).T

    # Initial values
    x[0][0] = sonar1_model.h_inv_one(sonar1_data[0])  # base initial position estimate off the sonar model
    x[0][1] = 0                                       # assume initial velocity is zero
    P[0] = np.eye(2) * 0.1                            # covariance of the initial belief
    

    for k in range(1,N):
        # This below code was implemented based on
        # https://en.wikipedia.org/wiki/Extended_Kalman_filter#Discrete-time_predict_and_update_equations
        # https://automaticaddison.com/extended-kalman-filter-ekf-with-python-code-example/
        # When comparing to the notes H_k=C and R_k=SUM_v and A=F from lecture 11

        # -------------------------------------------------------------------------------------------------------------------------------------------
        # Predict
        # -------------------------------------------------------------------------------------------------------------------------------------------
        alpha = 0.0001 if abs(u[k]) > 0.07 and not abs(u[k] - x[k-1][1]) > 0.12 else 0.05  # a piecewise recurrsive filter approximates "lag" in velocity command
        # alpha = 0.03 if abs(u[k]) > 0.02 else 0.3   
        A = np.array([[1, (1-alpha)*dt], [0, 1-alpha]])                                    # state transision matrix based on previous state (∂d/∂x)
        B = np.array([alpha*dt, alpha])                                                    # state transision matrix based on control input (∂u/∂x)
        
        # # Without the filter (just regular d_i = d_0 + v*dt)
        # A = np.array([[1, 0], [0, 0]])
        # B = np.array([dt, 1])

        x_k_neg = A @ x[k-1] + B * u[k]  # form prior belief based on the previous state, commanded velocity and some physics
        P_k_neg = A @ P[k-1] @ A.T + Q   # calculate the covariance of the prior belief

        # -------------------------------------------------------------------------------------------------------------------------------------------
        # Sensor model
        # -------------------------------------------------------------------------------------------------------------------------------------------
        # Determine variances (based of prior belief as thats the best we got at this point)
        var_z_1_k = (10/3 * x_k_neg[0]) ** 2                 # approximatly linear standard deviation with slope 5/3
        var_z_2_k = 0.5 ** 2                                 # approximately constant standard deviation of 0.25
        var_z_3_k = 0.4 ** 2 if x_k_neg[0] > 1 else 3**2     # piecewise standard deviation (depending on whether distance is above or below 1)
        
        # Measurement noise variance matrix (SUM_v from the notes)
        R_k = np.diag((
            var_z_1_k,
            var_z_2_k,
            var_z_3_k,
        ))

        # Sensor model matrix (C matrix from notes)
        H_k = np.array(list([get_partial_derivative_of_sensor_model(x_k_neg[0], 0.001, model), 0] for model in [
            sonar1_model,
            ir3_model,
            ir4_model,
        ]))

        # -------------------------------------------------------------------------------------------------------------------------------------------
        # Update
        # -------------------------------------------------------------------------------------------------------------------------------------------
        # Innovation or measurement residual (how far off our sensor readings are)
        y_k = z[k] - np.array([
            sonar1_model.h_one(x[k-1][0]),
            ir3_model.h_one(x[k-1][0]),
            ir4_model.h_one(x[k-1][0])
        ])

        # # TODO potential impletment filtering of values which cause signify a high change in speed
        # I've tried this and it doesn't seem to positively impact the results
        # for i, model in enumerate([
        #     sonar1_model,
        #     ir3_model,
        #     ir4_model,
        # ]):
        #     speed = abs(x_k_neg[0] - model.h_inv_one(z[k][i], x_k_neg[0])) / dt
        #     if speed > 10:
        #         y_k[i] *= 0.5

        S_k = H_k @ P_k_neg @ H_k.T + R_k                   # Innovation (measurement residual covariance)
        K_k = P_k_neg @ H_k.T @ np.linalg.inv(S_k)          # Near-optimal Kalman Gain
        x[k] = x_k_neg + K_k @ y_k                          # Postier belief (updated state estimate based on sensor readings)
        P[k] = (np.eye(len(K_k)) - K_k @ H_k) @ P_k_neg     # Postier covariance (updated covariance of state estimate)
        

    # -------------------------------------------------------------------------------------------------------------------------------------------
    # Plottting
    # -------------------------------------------------------------------------------------------------------------------------------------------
    # Plotting the filtered distance
    fig, axs = plt.subplots()
    axs.plot(time, distances, label='distances')
    axs.plot(time, x[:, 0], 'b', label=f'estimates')
    axs.set_xlabel('Time (t)')
    axs.set_ylabel('Range (m)')
    axs.set_title('EKF filtered distance')
    axs.legend()

    # Predicting the speed with a recurrsive filter
    fig, axs = plt.subplots()
    actual_speeds = np.zeros(N)
    for i in range(1, N):
        actual_speeds[i] = (distances[i] - distances[i-1]) / dt
    kernel = [0.05] * 20
    actual_speeds_smooth = np.convolve(actual_speeds, kernel, mode='same')
    predicted_speeds = np.zeros(N)
    for i in range(1, N):
        alpha = 0.015 if abs(velocity_command[i]) > 0.07 and not abs(velocity_command[i] - predicted_speeds[i-1]) > 0.12 else 0.1
        predicted_speeds[i] = velocity_command[i] * alpha + (1-alpha) * predicted_speeds[i-1]
    axs.plot(time, velocity_command, label='commanded_speed')
    axs.plot(time, actual_speeds, '.', alpha=0.2, label='actual_speed')
    axs.plot(time, actual_speeds_smooth, label='actual_speed_smooth')
    axs.plot(time, predicted_speeds, label='predicted_speed')
    axs.set_xlabel('Time (t)')
    axs.set_ylabel('Speed (m/s)')
    axs.set_title('Predicting the speed with a recurrsive filter')
    axs.legend()

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
