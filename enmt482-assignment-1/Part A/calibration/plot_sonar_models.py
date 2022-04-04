#
#                      plot_sonar_models.py
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
from matplotlib.pyplot import subplots, show
from numpy.linalg import lstsq
from models import *
from sonar_models import SonarModel


# Takes in calibration data
def plot_sonar_models(calibration_data_filepath):
    
    # Parse calibration data
    data = np.loadtxt(calibration_data_filepath, delimiter=',', skiprows=1)
    _,time,distance,velocity_command,raw_ir1,raw_ir2,raw_ir3,raw_ir4,sonar1,sonar2 = data.T
    
    # Define sonar models
    m_sonar1 = SonarModel(sonar1,time,distance,velocity_command)
    m_sonar1.set_h_parameters(1.007,0.01)

    m_sonar2 = SonarModel(sonar2,time,distance,velocity_command)
    m_sonar2.set_h_parameters(0.99,0.02)

    # Plot true range and sonar measurements over time

    # Plots setup:
    fig1, axes = plt.subplots(nrows=4, ncols=2, figsize=(12, 4))

    # Sonar 1 plots
    m_sonar1.plot_distance(axes[0,0], title="Sonar 1")
    m_sonar1.plot_model_vs_distance(axes[1,0], title="")
    m_sonar1.plot_model_error(axes[2,0], title="")
    m_sonar1.plot_model_error_histogram(axes[3,0], title="")
    

    # Sonar 2 plots
    m_sonar2.plot_distance(axes[0,1], title="Sonar 2")
    m_sonar2.plot_model_vs_distance(axes[1,1], title="")
    m_sonar2.plot_model_error(axes[2,1], title="")
    m_sonar2.plot_model_error_histogram(axes[3,1], title="")
    

    plt.show()









# Run program
if __name__ == '__main__':
    plot_sonar_models("calibration.csv")