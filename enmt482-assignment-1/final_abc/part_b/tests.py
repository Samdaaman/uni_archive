"""Particle filter tests

Note, run with motion_model not adding noise.
This assumes that heading angles are wrapped to be in the range [-pi, pi)

M.P. Hayes
Department of Electrical and Computer Engineering
University of Canterbury
"""

from __future__ import print_function, division
from models_1 import motion_model, sensor_model
from numpy import array, any, set_printoptions
from utils import wraptopi
from numpy import pi, newaxis


class Robot(object):

    def __init__(self, x=0, y=0, heading=pi / 2):

        self.x = x
        self.y = y
        self.heading = wraptopi(heading)

    @property
    def pose(self):
        return array((self.x, self.y, self.heading), dtype=float)
        
    def transition(self, v, omega, dt=0.1):

        from numpy import sin, cos
        
        hp = self.heading

        if omega == 0.0:
            self.x += v * cos(hp) * dt
            self.y += v * sin(hp) * dt
        else:
            self.x += -v / omega * sin(hp) + v / omega * sin(hp + omega * dt)
            self.y += v / omega * cos(hp) - v / omega * cos(hp + omega * dt)
            self.heading = wraptopi(hp + omega * dt)


def test_move(v, omega=0, heading=0, lheading=0, dt=1):

    odom = Robot(1, 2, lheading)
    robot = Robot(3, 4, heading)    

    odom_prev_pose = odom.pose
    odom.transition(v, omega, dt)
    odom_pose = odom.pose

    prev_pose = robot.pose
    robot.transition(v, omega, dt)    
    expected_pose = robot.pose

    poses = prev_pose[newaxis, :]
    commands = array((v, omega))
    poses = motion_model(poses, commands, odom_pose, odom_prev_pose, dt)
    if any(abs(poses - expected_pose) > 1e-12):
        raise ValueError('Expected %s got %s.  Did you set motion error to 0?' % (poses[0], expected_pose))
    print('Passed v=%.3f, omega=%.3f, heading=%.3f, lheading=%.3f' % (v, omega, heading, lheading))

    
set_printoptions(precision=3)   
test_move(1, 0)
test_move(-1, 0)
test_move(1, 0, lheading=pi / 2)
test_move(1, 0, heading=-pi, lheading=pi / 2)
test_move(1, 1, lheading=pi / 2)
test_move(1, 1, heading=-pi, lheading=pi / 2)
test_move(1, 1, heading=pi / 2, lheading=pi / 2)
test_move(1, 1, heading=pi / 2, lheading=pi)
test_move(1, 1, heading=3 * pi, lheading=pi)
test_move(1, 20, heading=3 * pi, lheading=pi)
test_move(20, 0, heading=3 * pi, lheading=pi)
