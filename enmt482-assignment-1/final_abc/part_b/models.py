"""Particle filter sensor and motion model implementations.

M.P. Hayes and M.J. Edwards,
Department of Electrical and Computer Engineering
University of Canterbury
"""

import numpy as np
from numpy import cos, sin, tan, arccos, arcsin, arctan2, sqrt, exp
from numpy.random import randn, normal
from utils import gauss, wraptopi, angle_difference, gauss, pdf, score_array, wrapto2pi
from scipy.stats import norm

# Global sensor model tuning parameters
mu_r = 0; mu_phi = 0
# sigma_r = 0.1 # Excpects to be within 40cm
# sigma_phi = 0.1 # Note: 0.1~6deg one sigma expects 68% of all vals to be in range
sigma_r = 0.05 # Excpects to be within 40cm
sigma_phi = 0.1 # Note: 0.1~6deg one sigma expects 68% of all vals to be in range

def motion_model(particle_poses, speed_command, odom_pose, odom_pose_prev, dt):
    """Apply motion model and return updated array of particle_poses.

    Parameters
    ----------

    particle_poses: an M x 3 array of particle_poses where M is the
    number of particles.  Each pose is (x, y, theta) where x and y are
    in metres and theta is in radians.

    speed_command: a two element array of the current commanded speed
    vector, (v, omega), where v is the forward speed in m/s and omega
    is the angular speed in rad/s.

    odom_pose: the current local odometry pose (x, y, theta).

    odom_pose_prev: the previous local odometry pose (x, y, theta).

    dt is the time step (s).

    Returns
    -------
    An M x 3 array of updated particle_poses.

    """

    M = particle_poses.shape[0]

    # Extract local pose (x is a vector for each paricle 0-M)
    # In robot coords (ie relative to starting pos of the robot)
    x,y,theta = odom_pose
    x_prev, y_prev,theta_prev = odom_pose_prev


    # Generate process notes
    # Generate random noise on d, phi1, phi2
    #sigma_phi1=0.06; sigma_phi2=0.01; sigma_d=0.02; # MEthod1 best
    sigma_phi1=0.06; sigma_phi2=0.01; sigma_d=0.02; # MEthod2 best
    # sigma_phi1=0.01; sigma_phi2=0.01; sigma_d=0.02;
    #sigma_phi1=0.; sigma_phi2=0.; sigma_d=0.00; #NOTE: this line yields regular motion model
    phi1_noise = np.random.normal(0,sigma_phi1,M)
    phi2_noise = np.random.normal(0,sigma_phi2,M)
    d_noise = np.random.normal(0,sigma_d,M)


    # Calc local frame range + angle changes
    # Decouple first bearing in odom model
    phi1 = arctan2((y-y_prev), (x-x_prev)) - theta_prev
    # pos changes with noise
    phi1_n = phi1+phi1_noise
    
    # Decouple translational range in step
    d=np.sqrt((x-x_prev)**2 + (y-y_prev)**2)
    d_n = d+d_noise

    # Decouple second bearing in odom model
    phi2 = theta - theta_prev - phi1_n
    phi2_n = phi2+phi2_noise


    for m in range(M):
        # Equivilant to x = x_prev + dcos(...)
        particle_theta_prev = particle_poses[m,2]
        particle_poses[m, 0] += d_n[m]*cos(particle_theta_prev + phi1_n[m]) 
        particle_poses[m, 1] += d_n[m]*sin(particle_theta_prev + phi1_n[m])
        particle_poses[m, 2] += phi1_n[m] + phi2_n[m]
    
    return particle_poses


def sensor_model_relative_method(particle_poses, beacon_pose, beacon_loc):
    """Apply sensor model and return particle weights.

    Parameters
    ----------
    
    particle_poses: an M x 3 array of particle_poses (in the map
    coordinate system) where M is the number of particles.  Each pose
    is (x, y, theta) where x and y are in metres and theta is in
    radians.

    beacon_pose: the measured pose of the beacon (x, y, theta) in the
    robot's camera coordinate system.

    beacon_loc: the pose of the currently visible beacon (x, y, theta)
    in the map coordinate system.

    Returns
    -------
    An M element array of particle weights.  The weights do not need to be
    normalised.

    """

    M = particle_poses.shape[0]
    particle_weights = np.zeros(M)
    
    #Extract beacon poses (camera d coord frame)
    c_x_b, c_y_b, c_theta_b = beacon_pose

    # Extract beacon positions (map coord frame)
    x_b_loc, y_b_loc, theta_b_loc = beacon_loc

    # Extract particle poses (map coord frame)
    x_m, y_m, theta_m = particle_poses.T #Gotta transpose to extract


    # 1. Convert camera coord frame to robot coord frame
    r_x_b = c_x_b + 0.120#- 0.054 # camera offset from centre of robot by ~54mm
    r_y_b = c_y_b # no y offset
    r_theta_b = c_theta_b # No angle offset

    # 2. Decouple the measured range and bearing from robot to beacon measurement
    r_rb = np.sqrt(r_x_b**2 + r_y_b**2) # range from robot to beacon
    phi_rb = arctan2(r_y_b, r_x_b) # bearing from robot to beacon

    
    for m in range(M):
        
        # 3. Decouple the range and bearing of each particle to the beacon's actual position
        dx = x_b_loc - x_m[m]
        dy = y_b_loc - y_m[m]
        
        r_mb = np.sqrt((dx)**2 + (dy)**2)
        phi_mb = angle_difference(theta_m[m], arctan2(dy, dx))

        # 4. Calculate particle & beacon discrepencies between range and bearing 
 
        range_weight = gauss(r_rb - r_mb, mu_r, sigma_r)
        phi_weight = gauss(angle_difference(phi_rb, phi_mb),mu_phi, sigma_phi)
        
        # 5. Weight current particle Define liklihood functions :
        particle_weights[m] = range_weight*phi_weight


    particle_weights = particle_weights/np.max(particle_weights)
    
    
    return particle_weights, None




def sensor_model_absolute_method(particle_poses, beacon_pose, beacon_loc):
    """Apply sensor model and return particle weights.

    Parameters
    ----------
    
    particle_poses: an M x 3 array of particle_poses (in the map
    coordinate system) where M is the number of particles.  Each pose
    is (x, y, theta) where x and y are in metres and theta is in
    radians.

    beacon_pose: the measured pose of the beacon (x, y, theta) in the
    robot's camera coordinate system.

    beacon_loc: the pose of the currently visible beacon (x, y, theta)
    in the map coordinate system.

    Returns
    -------
    An M element array of particle weights.  The weights do not need to be
    normalised.

    """

    M = particle_poses.shape[0]
    particle_weights = np.zeros(M)
    
    #Extract beacon poses (robot coord frame)
    c_x_b, c_y_b, c_theta_b = beacon_pose

    # Extract beacon positions (map coord frame)
    x_b_loc, y_b_loc, theta_b_loc = beacon_loc

    # Extract particle poses (map coord frame)
    m_x,m_y,m_theta = particle_poses.T #Gotta transpose to extract


    # 1. Convert camera coord frame to robot coord frame
    x_b = c_x_b #+ 0.054#- 0.054 # camera offset from centre of robot by ~54mm
    y_b = c_y_b # no y offset
    theta_b = c_theta_b # No angle offset



    # PERFORM UPDATE STEP
    # (Give low weights to particles that think beacon is
    # far away from where it actually is in map coords)

    # Decouple relative range of beacon (robot frame)
    r_b_robot = np.sqrt((x_b)**2 + (y_b)**2)

    # Decouple relative bearing of beacon (robot frame)
    phi_b_robot = arctan2(y_b, x_b)

    # Iterate (remove later to use broadcasting)
    # Guesses of beacon position (map coord frame)
    beacon_guesses_x = np.empty(M)
    beacon_guesses_y = np.empty(M)
    beacon_guesses_theta = np.empty(M)

    # WORKS
    for m in range(M):
        beacon_guesses_x[m] = m_x[m] + r_b_robot * cos(m_theta[m] + phi_b_robot)
        beacon_guesses_y[m] = m_y[m] + r_b_robot * sin(m_theta[m] + phi_b_robot)
        beacon_guesses_theta[m] = wrapto2pi(m_theta[m] + theta_b) # as theta_b_loc is always [0, 2pi)

    beacon_discrepancies_position = (x_b_loc - beacon_guesses_x)**2 + (y_b_loc - beacon_guesses_y)**2
    beacon_discrepancies_theta = abs(theta_b_loc - beacon_guesses_theta)
    

    # Define liklihood function TODO: tune this
    for m in range(M):

        mu_r = 0; mu_phi = 0; sigma_r = 0.5; sigma_phi = 2
        
        f_r_dist = gauss(beacon_discrepancies_position[m], mu_r, sigma_r)
        f_phi_dist = gauss(beacon_discrepancies_theta[m],mu_phi, sigma_phi)

        particle_weights[m] = f_r_dist*f_phi_dist
    
    # Normalise
    if (np.max(particle_weights) > 0):
        particle_weights = particle_weights/np.max(particle_weights)

    # Methods for beacon scoring for visualisations:
    # Score the beacon discrepancies in position from 0 to M-1. Where a lower score is better
    scores_position = score_array(beacon_discrepancies_position)
    
    # Score the beacon discrepancies in theta from 0 to M-1. Where a lower score is better
    scores_theta = score_array(beacon_discrepancies_theta)

    # Total and normalise the score array
    scores = scores_position + scores_theta # square to punish individual high distance scores more
    scores = score_array(scores)

    score_order = np.argsort(scores)
    beacon_guesses_sorted = np.array((beacon_guesses_x[score_order], beacon_guesses_y[score_order], beacon_guesses_theta[score_order])).T


    return particle_weights, beacon_guesses_sorted




# Select which sensor model to use in demo.py
sensor_model = sensor_model_relative_method


