#
#                       calibration_fitting.py
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
#  Fitting code adapted from ir3_hist_demo1_plot by Michael P. Hayes
#  Date Last Modified: 15th September 2021
#
#  Module Description:
#    A script used to fit various sensor models that are an improvement on
#    the classic "inverse IR model".
#    Note: sonar1 was fitted predominantly by hand
#


from matplotlib.pyplot import subplots, show
import numpy as np
import os

from calibration_models import *


def main():
    # Import data
    os.chdir(os.path.abspath(os.path.dirname(__file__))) # change directory to script dir for access to datasets
    filename = 'calibration.csv'
    _,time,distance,velocity_command,raw_ir1,raw_ir2,raw_ir3,raw_ir4,sonar1,sonar2 = np.loadtxt(filename, delimiter=',', skiprows=1).T
    
    fig, axs = subplots(2, 2)
    axs = axs.flatten()

    # For each raw_ir dataset, create the model, fit the model, and plot the fitted curve
    # Note: sonar1 was fitted predominantly by hand
    for i, ir_data in enumerate([raw_ir1, raw_ir2, raw_ir3, raw_ir4]):
        def plot_fitted(model: Model, ir_data=ir_data, distance=distance):
            model.nonlinear_least_squares_fit(distance, ir_data)
            data_fitted = model.h(distance)

            print(data_fitted)
            axs[i].plot(distance, data_fitted, label=model.__class__.__name__)

        axs[i].plot(distance, ir_data, '.', alpha=0.2, color='grey')
        axs[i].set_title(f'IR {i+1}')
        axs[i].set_xlabel('Distance (m)')
        axs[i].set_ylabel('Measurement')
        axs[i].set_ylim(0, 4)

        better_model = IRBetterModel()
        plot_fitted(better_model)
        print(f'IR {i+1}: {better_model.k}')
        
        # For IR4 create a 3 part piecewise model
        if i == 3:
            outliers = np.where(distance < 0.5, 1, 0) * np.where(ir_data < 1.5, 1, 0)
            ir_data_trimmed = ir_data[np.logical_not(outliers)]
            distance_trimmed = distance[np.logical_not(outliers)]
            three_part_model = IRThreePartModel((0.37, 0.6184386432968607), 0.001)
            plot_fitted(three_part_model, ir_data_trimmed, distance_trimmed)
            print(three_part_model.k)
    
        axs[i].legend()
    show()


if __name__ == '__main__':
    main()
