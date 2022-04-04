"""Particle filter demonstration program.

M.P. Hayes and M.J. Edwards,
Department of Electrical and Computer Engineering
University of Canterbury
"""

from __future__ import print_function, division
from numpy.random import uniform
import matplotlib
# May need to comment out following line for MacOS
matplotlib.use("TkAgg")
from models import motion_model, sensor_model, sigma_r, sigma_phi
from utils import *
from plot import *
from transform import *
import numpy as np
import math

# Load data

# data is a (many x 13) matrix. Its columns are:
# time_ns, velocity_command, rotation_command, map_x, map_y, map_theta, odom_x, odom_y, odom_theta,
# beacon_ids, beacon_x, beacon_y, beacon_theta
data = np.genfromtxt('data.csv', delimiter=',', skip_header=1)

# Time in ns
t = data[:, 0]

# Velocity command in m/s, rotation command in rad/s
commands = data[:, 1:3]

# Position in map frame, from SLAM (this approximates ground truth)
slam_poses = data[:, 3:6]

# Position in odometry frame, from wheel encoders and gyro
odom_poses = data[:, 6:9]

# Id and measured position of beacon in camera frame
beacon_ids = data[:, 9]
beacon_poses = data[:, 10:13]
# Use beacon id of -1 if no beacon detected
beacon_ids[np.isnan(beacon_ids)] = -1
beacon_ids = beacon_ids.astype(int)
beacon_visible = beacon_ids >= 0

# map_data is a 16x13 matrix.  Its columns are:
# beacon_ids, x, y, theta, (9 columns of covariance)
map_data = np.genfromtxt('beacon_map.csv', delimiter=',', skip_header=1)

Nbeacons = map_data.shape[0]
beacon_locs = np.zeros((Nbeacons, 3))
for m in range(Nbeacons):
    id = int(map_data[m, 0])
    beacon_locs[id] = map_data[m, 1:4]

# Remove jumps in the pose history
slam_poses = clean_poses(slam_poses)

# Transform odometry poses into map frame
odom_to_map = find_transform(odom_poses[0], slam_poses[0])
odom_poses = transform_pose(odom_to_map, odom_poses)

plt.ion()
fig = plt.figure(figsize=(10, 5))
axes = fig.add_subplot(111)

plot_beacons(axes, beacon_locs, label='Beacons')
plot_path(axes, slam_poses, '-', label='SLAM')
# Uncomment to show odometry when debugging
#plot_path(axes, odom_poses, 'b:', label='Odom')

axes.legend(loc='lower right')

axes.set_xlim([-6, None])
axes.axis('equal')

# Tweak axes to make plotting better
axes.invert_yaxis()
axes.set_xlabel('y')
axes.set_ylabel('x')
axes.figure.canvas.draw()
axes.figure.canvas.flush_events()

# TODO: Set this to avoid twirl at start
# When your algorithm works well set to 0
start_step = 0# Originally: 50

# TODO: Number of particles, you may need more or fewer!
Nparticles = 100

# TODO: How many steps between display updates
display_steps = 10

# Define number of steps between particle count updates
Nsteps_particle_count_update = 10

# Define if we are taking the mean squared error
calc_MSE = True

# TODO: Set initial belief
start_pose = slam_poses[start_step]
print(f'start_position is {start_pose}')
Xmin = start_pose[0] - 0.1
Xmax = start_pose[0] + 0.1
Ymin = start_pose[1] - 0.1
Ymax = start_pose[1] + 0.1
Tmin = start_pose[2] - 0.1
Tmax = start_pose[2] + 0.1

weights = np.ones(Nparticles)
poses = np.zeros((Nparticles, 3))

for m in range(Nparticles):
    poses[m] = (uniform(Xmin, Xmax),
                uniform(Ymin, Ymax),
                uniform(Tmin, Tmax))

Nposes = odom_poses.shape[0]
est_poses = np.zeros((Nposes, 3))

display_step_prev = 0
mse_r_error = [] # list for mean squared error
mse_theta_error = [] 
# Iterate over each timestep.
for n in range(start_step + 1, Nposes):

    # Motion model function
    poses = motion_model(poses, commands[n-1], odom_poses[n], odom_poses[n - 1],
                         t[n] - t[n - 1])
    
    # Update number of particles
    if (n % Nsteps_particle_count_update == 0):
        # Reduce number of particles if in good position
        try:
            Nparticles= int(5*((100 - np.sum(weights)-50)))
        except:
            Nparticles = 100
            pass # TODO: have lost recovery kick into action here

        # Ensure Nparticles in suitable range
        Nparticles = np.minimum(300,Nparticles)
        Nparticles = np.maximum(30,Nparticles)

        # Take the Nparticle best particles (list of indicies)
        best_particles = weights.argsort()[:Nparticles]

        # Update poses & weights
        poses = poses[best_particles]
        weights = weights[best_particles]

    # Determine error of robot pos to SLAM
    if (calc_MSE):
        target = np.array(slam_poses[n])
        best_particle = np.argsort(weights)[-1]
        guess = np.array(poses[best_particle])
        
        r_error = ((target[0:2]-guess[0:2])**2).mean()
        r_error = r_error if not math.isnan(r_error) else 0 
        mse_r_error.append(r_error)

        theta_error = ((angle_difference(target[-1],guess[-1]))**2).mean() 
        theta_error = theta_error if not math.isnan(theta_error) else 0
        mse_theta_error.append( theta_error )




    if beacon_visible[n]:

        beacon_id = beacon_ids[n]
        beacon_loc = beacon_locs[beacon_id]
        beacon_pose = beacon_poses[n]

        # Sensor model function
        sensor_weights, beacon_guesses = sensor_model(poses, beacon_pose, beacon_loc)
        plot_beacon_guesses(axes, beacon_guesses[0:1]) if beacon_guesses is not None else None
        weights *= sensor_weights

        if sum(weights) < 1e-50:
            print('All weights are close to zero, you are lost...')
            # Lost recovery

            best_particle = np.argsort(weights)[-1]
            best_poses = poses[best_particle]

            # Drasticaly increase num of particles and see if it can associate any particle
            Nparticles = 500
            xrange=5;yrange=10;theta_range=1.5;
            # Generate random spread based on best weighted particle
            new_poses = np.zeros((Nparticles, 3))
            new_poses[:,0] = best_poses[0] + np.random.uniform(-xrange, xrange, Nparticles) # generate x mesh
            new_poses[:,1] = best_poses[1] + np.random.uniform(-yrange, yrange, Nparticles) # generate x mesh
            new_poses[:,2] = wrapto2pi(best_poses[2] + np.random.uniform(-theta_range, theta_range, Nparticles)) # generate x mesh
            weights = 0.5*np.ones(Nparticles)
            poses = new_poses

            # Drasticaly increase num of particles and see if it can associate any particle
            # with a high weight

        if is_degenerate(weights):
            print('Resampling %d' % n)
            resample(poses, weights)

    else:
        # Plot that no beacon in sight
        plot_beacon_guesses(axes, None, no_beacon=True) 


    est_poses[n] = poses.mean(axis=0)

    if n > display_step_prev + display_steps:
        print(f"n: {n} | particles: {Nparticles}")

        # Show particle cloud
        plot_particles(axes, poses, weights)

        # Leave breadcrumbs showing current odometry
        #plot_path(axes, odom_poses[n], 'k.', label="Odom")

        # Show mean estimate
        plot_path_with_visibility(axes, est_poses[display_step_prev-1 : n+1],
                                  '-', visibility=beacon_visible[display_step_prev-1 : n+1])
        display_step_prev = n


# Calc final MSE error
error_msg = f'MSE error: ({np.array(mse_r_error).mean():0.3f}m, {np.array(mse_theta_error).mean():0.3f}rad)'
print(error_msg)
# Display final plot
print('Done, displaying final plot')
plt.ioff()
plt.show()

# Save final plot to file
plot_filename = 'path.png'
print('Saving final plot to', plot_filename)

plot_path(axes, est_poses, 'r-', label=f'PF ff')
axes.legend(loc='lower right')

fig = plt.figure(figsize=(10, 5))
axes = fig.add_subplot(111)

axes.set_title(f"Sigma_r: {sigma_r} | sigma_phi: {sigma_phi} | {error_msg} | method: {'NEW'}")

plot_beacons(axes, beacon_locs, label='Beacons')
plot_path(axes, slam_poses, 'b-', label='SLAM')
plot_path(axes, odom_poses, 'k:', label='Odom')
plot_path(axes, est_poses, 'r-', label=f'PF')

axes.legend(loc='lower right')

axes.set_xlim([-6, None])
axes.axis('equal')

# Tweak axes to make plotting better
axes.invert_yaxis()
axes.set_xlabel('y')
axes.set_ylabel('x')
fig.savefig(plot_filename, bbox_inches='tight')
