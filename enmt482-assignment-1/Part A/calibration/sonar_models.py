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
from numpy.linalg import lstsq
import matplotlib.pyplot as plt

class SonarModel():

    def __init__(self, raw_data, time_data=[], distance_data=[], velocity_command_data=[]):
        
        # Input raw entried
        self.raw_data = raw_data
        self.distance_data = distance_data
        self.velocity_command_data = velocity_command_data
        self.time_data = time_data

        # Process data on initialisation
        self.preprocess_data(raw_data, distance_data)

        # Define default model parameters
        self.set_h_parameters(1,0)
        
        
    # Preprocess data to clean out outliers
    def preprocess_data(self, raw_data, distance_data):
        
        inliers = np.where(np.absolute(raw_data - distance_data) < 0.08 , 1, 0) #* np.where(raw_data < 4, 1, 0)
        
        self.cleaned_data = np.compress(inliers, raw_data)
        self.cleaned_distance_data = np.compress(inliers, self.distance_data)
        self.cleaned_time_data = np.compress(inliers, self.time_data)
        
        return

    

    # linear model tuning in form of y=mx+c
    def set_h_parameters(self, m=0, c=0):
        self.h_m = m
        self.h_c = c

    # Model function - returns Z = h(X) + V(x) 
    def Z(self, x):
        # Broadcast over array
        h = self.h_m * x + self.h_c
        return h
        
    # Returns measurement error of model
    def get_measurement_error(self):
        return self.cleaned_distance_data - self.Z(self.cleaned_data)


    

    #_____________________Internal plotting methods_____________________#


    # Plot method that takes an ax subobject and returns 
    # the ax object populated with the relevant rendered figure
    # Good styling resource: 
    # https://www.pythoninformer.com/python-libraries/matplotlib/line-plots/

    def plot_distance(self, ax=None, title="", labels=True):
        # Define new axes if none given
        if ax is None:
            ax = plt.gca()
        
        # Plot line
        line, = ax.plot(self.time_data,self.raw_data,
                                            '.', 
                                            alpha=0.2, 
                                            label="reading")
        
        line, = ax.plot(self.time_data,self.distance_data, 
                                            color='#f32042', 
                                            linewidth=2, 
                                            linestyle=(0, (5, 2, 1, 2)), 
                                            dash_capstyle='round',
                                            alpha=1.0,
                                            label="distance")

        # Add styling
        ax.set_title(title)
        if (labels):
            ax.set_xlabel('Time (s)')
            ax.set_ylabel('Raw Val')
        ax.legend(loc='best')
  
        return line


    # Plot method that takes an ax subobject and returns 
    # the ax object populated with the relevant rendered figure
    def plot_model_vs_distance(self, ax=None, title="", labels=True):
        # Define new axes if none given
        if ax is None:
            ax = plt.gca()
        
        # Plot line
        line, = ax.plot(self.cleaned_distance_data,self.cleaned_data,
                                            '.', 
                                            alpha=0.2, 
                                            label="cleaned reading")
        
        line, = ax.plot(self.cleaned_distance_data, self.Z(self.cleaned_data), 
                                            color='#f32042', 
                                            linewidth=2, 
                                            linestyle=(0, (5, 2, 1, 2)), 
                                            dash_capstyle='round',
                                            alpha=1.0,
                                            label=f"h={self.h_m}x+{self.h_c}")

        # Add styling
        ax.set_title(title)

        # Plot labels if requested
        if (labels):
            ax.set_xlabel('Distance (m)')
            ax.set_ylabel('Measurement (x)')
        ax.legend(loc='best')

        # Return the axes content
        return line


    # Plot method that takes an ax subobject and returns 
    # the ax object populated with the relevant rendered figure
    def plot_model_error(self, ax=None, title="", labels=True):
        # Define new axes if none given
        if ax is None:
            ax = plt.gca()
        
        

        line, = ax.plot([0, self.cleaned_distance_data[-1]], [0, 0], '--', color='#000000')
        
        line, = ax.plot(self.cleaned_distance_data, self.get_measurement_error(),
                                            '.', 
                                            alpha=0.2, 
                                            color='#f32042', 
                                            linewidth=1, 
                                            #linestyle=(0, (5, 2, 1, 2)), 
                                            #dash_capstyle='round',                                       
                                            label=f"h={self.h_m}x+{self.h_c} error")

        # Add styling
        ax.set_title(title)

        # Plot labels if requested
        if (labels):
            ax.set_xlabel('Distance (m)')
            ax.set_ylabel('Measurement Error')
        ax.legend(loc='best')

        # Return the axes content
        return line


    # Plot method that takes an ax subobject and returns 
    # the ax object populated with the error histogram rendered figure
    def plot_model_error_histogram(self, ax=None, title="", labels=True):
        # Define new axes if none given
        if ax is None:
            ax = plt.gca()

        # Get histogram
        counts, bins = np.histogram(self.get_measurement_error(), bins=50, density=True, range=(-0.05, 0.05))
        print(bins)
        hist = ax.hist(bins[:-1], bins, weights=counts)

        # Add styling
        ax.set_title(title)

        # Plot labels if requested
        if (labels):
            ax.set_xlabel('Measurement Error (v)')
            ax.set_ylabel('Bins')
        ax.legend(loc='best')

        # Return the axes content
        return hist



    #______________Motion model plots___________#
       # Plot method that takes an ax subobject and returns 
    # the ax object populated with the relevant rendered figure
    # Good styling resource: 
    # https://www.pythoninformer.com/python-libraries/matplotlib/line-plots/

    def plot_motion_model(self, ax=None, title="", labels=True):
        # Define new axes if none given
        if ax is None:
            ax = plt.gca()
        
        # Plot line
        line, = ax.plot(self.time_data,self.raw_data,
                                            '.', 
                                            alpha=0.2, 
                                            label="reading")
        
        line, = ax.plot(self.time_data,self.distance_data, 
                                            color='#f32042', 
                                            linewidth=2, 
                                            linestyle=(0, (5, 2, 1, 2)), 
                                            dash_capstyle='round',
                                            alpha=1.0,
                                            label="distance")

        # Add styling
        ax.set_title(title)
        if (labels):
            ax.set_xlabel('Time (s)')
            ax.set_ylabel('Raw Val')
        ax.legend(loc='best')
  
        return line


        

