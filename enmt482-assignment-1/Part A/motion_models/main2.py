import numpy as np
import matplotlib.pyplot as plt
from models import *


def main():
    filename = 'calibration.csv'
    data = np.loadtxt(filename, delimiter=',', skiprows=1)
    _,time,distance,velocity_command,ir1_data,ir2_data,ir3_data,ir4_data,sonar1_data,sonar2_data = data.T
    
    # models
    sonar1_model = SonarModel([0.99, 0])#sonar1_model = SonarModel([0.995, -0.015]) # TODO make this a quadratic model
    ir1_model = IRBetterModel([-0.25, 0.14, 0.2, 0.005])
    ir2_model = IRBetterModel([-0.25, 0.14, 0.2, 0.005])
    ir3_model = IRBetterModel([-0.00724208, 0.17960182, 0.2964516, -0.00167510])
    ir4_model = IRThreePartModel([-1.51507457, -3.39024013, 20.5436498, 121659.198, 34823.3509, -32.3, 1.6, -1.66510526, 320902572, 9066691.96, 1.07535534, -0.0610574, 0.02014625, 2.14001523, 0.24313071], [0.365, 0.65])

    # don't question it, I'm lazy okay
    locals_ = locals()
    models = [(sonar1_data, sonar1_model)] + [(locals_[f'ir{i}_data'], locals_[f'ir{i}_model']) for i in range(1, 5)]

    for i, (model_data, model) in enumerate(models):
        fig, axs = plt.subplots(3, 1)
        axs = axs.flatten() # type: plt.Axes

        axs[0].plot(distance, model_data, '.', alpha=0.2, color='grey')
        axs[0].plot(distance, model.h(distance), color='orange', label='estimated')
        axs[0].plot(model.h_inv(model.h(distance), distance), model.h(distance), '--', color='green', label='inverted-model')
        axs[0].set_title(f'Fitted curve: Sensor #{i+1}')
        axs[0].set_xlabel('Distance (m)')
        axs[0].set_ylabel('Measurement')
        axs[0].set_ylim(0, 4)
        axs[0].legend()

        axs[1].plot(distance, [0]*len(distance))
        axs[1].plot(distance, model_data - model.h(distance), '.', alpha=0.2, color='red')
        axs[1].set_ylabel('Error')

        error_max_axis = 0.5
        bins = np.linspace(-error_max_axis, error_max_axis, 100)
        axs[2].hist(model_data - model.h(distance), bins=bins, color='orange')
        axs[2].set_xlabel('Measurement')
        axs[2].set_xlim(-error_max_axis, error_max_axis)

    plt.show()


if __name__ == '__main__':
    main()
