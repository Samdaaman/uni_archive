# Michael P. Hayes UCECE, Copyright 2018--2019
import numpy as np
from matplotlib.pyplot import subplots, show
from numpy.linalg import lstsq
from models import *


def ir3_hist_demo1_plot():
    filename = 'calibration.csv'
    data = np.loadtxt(filename, delimiter=',', skiprows=1)
    _,time,distance,velocity_command,raw_ir1,raw_ir2,raw_ir3,raw_ir4,sonar1,sonar2 = data.T
    
    fig, axs = subplots(2, 2)
    axs = axs.flatten()

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

        plot_fitted(IRBasicModel())
        better_model = IRBetterModel()
        plot_fitted(better_model)
        print(f'IR {i+1}: {better_model.k}')
        
        if i == 3:
            outliers = np.where(distance < 0.5, 1, 0) * np.where(ir_data < 1.5, 1, 0)
            ir_data_trimmed = ir_data[np.logical_not(outliers)]
            distance_trimmed = distance[np.logical_not(outliers)]
            three_part_model = IRTThreePartModel((0.37, 0.6184386432968607), 0.001)
            plot_fitted(three_part_model, ir_data_trimmed, distance_trimmed)
            print(three_part_model.k)
    
        axs[i].legend()
    show()


if __name__ == '__main__':
    ir3_hist_demo1_plot()
