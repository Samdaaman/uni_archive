from filterpy.kalman import KalmanFilter
from filterpy.common import Q_discrete_white_noise

f = KalmanFilter (dim_x=2, dim_z=1)

f.x = np.array([[2.],    # position
                [0.]])   # velocity

#Define the state transition matrix
f.F = np.array([[1.,1.],
                [0.,1.]])

# Define the measurement function
f.H = np.array([[1.,0.]])

#Define the covariance matrix (P already contains np.eye(dim_x), and just multiply by the uncertainty)
#f.P *= 1000.
f.P = np.array([[1000.,    0.],
                [   0., 1000.] ])

#  assign the measurement noise
f.R = 5 # or np.array([[5.]])


# Add process noise 
f.Q = Q_discrete_white_noise(dim=2, dt=0.1, var=0.13)

# Program loop
while (1):
    z = get_sensor_reading() # todo herre
    f.predict()
    f.update(z)
