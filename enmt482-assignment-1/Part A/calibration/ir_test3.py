# Michael P. Hayes UCECE, Copyright 2018--2019
import numpy as np
from matplotlib.pyplot import subplots, show
from numpy.linalg import lstsq
from models import *


def main():
    filename = 'calibration.csv'
    data = np.loadtxt(filename, delimiter=',', skiprows=1)
    _,time,distance,velocity_command,ir1_data,ir2_data,ir3_data,ir4_data,sonar1_data,sonar2_data = data.T
    
    fig, axs = subplots(2, 2)
    axs = axs.flatten()

    # least squares fitted model
    ir1_model_ls = IRBetterModel([-0.1758607, 0.15450938, 0.20772258, 0.017566930])
    ir2_model_ls = IRBetterModel([-0.17533161, 0.15460977, 0.20662386, 0.01680488])
    ir3_model_ls = IRBetterModel([-0.00724208, 0.17960182, 0.2964516, -0.00167510])
    
    # models tuned by hand (initially least squares fitted)
    ir1_model = IRBetterModel([-0.25, 0.14, 0.2, 0.005])
    ir2_model = IRBetterModel([-0.25, 0.14, 0.2, 0.005])
    ir3_model = IRBetterModel([-0.00724208, 0.17960182, 0.2964516, -0.00167510])
    
    # don't question it, I'm lazy okay
    locals_ = locals()
    models = [(locals_[f'ir{i}_data'], locals_[f'ir{i}_model_ls'], locals_[f'ir{i}_model']) for i in range(1, 4)]

    for i, (ir_data, ir_fitted_ls, ir_fitted_tuned) in enumerate(models):
        axs[i].plot(distance, ir_data, '.', alpha=0.2, color='grey')
        print(ir_data)
        print(ir_fitted_ls)
        axs[i].plot(distance, ir_fitted_ls.h(distance), color='orange', label='least-squares')
        axs[i].plot(distance, ir_fitted_tuned.h(distance), '--', color='green', label='tuned')
        axs[i].set_title(f'IR {i+1}')
        axs[i].set_xlabel('Distance (m)')
        axs[i].set_ylabel('Measurement')
        axs[i].set_ylim(0, 4)
        axs[i].legend()
    show()


if __name__ == '__main__':
    main()
