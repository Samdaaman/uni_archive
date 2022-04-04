"""Pose transformation functions for particle filter assignment.

M.P. Hayes and M.J. Edwards,
Department of Electrical and Computer Engineering
University of Canterbury
"""

import numpy as np
from utils import *


# Functions for working with 2D poses represented as (x, y, theta)

def transform_pose(tf, pose):
    """Apply transformation 'tf' (dx, dy, dtheta) to 'pose' (x, y, theta).

    Both 'pose' and 'tf' can be lists or arrays.

    The arguments can be array-likes of broadcastable shapes, and the result
    will be the shape of whichever is larger.  Here are some examples of valid
    shape combinations.

    Applying a transformation to a pose:
    >>> transform_pose((1, 1, np.pi/2), (1, 0, 0))
    array([ 1.        ,  2.        ,  1.57079633])

    Applying a list of transformations to a single pose (e.g., rotating a list of poses to plot them
    better):
    >>> transform_pose([(0, 0, 0), (1, 1, np.pi/4), (1, 1, np.pi/2)], (1, 0, 0))
    array([[ 1.        ,  0.        ,  0.        ],
           [ 1.70710678,  1.70710678,  0.78539816],
           [ 1.        ,  2.        ,  1.57079633]])

    Applying a list of transformations to a list of poses (e.g., adding noise to particles):
    >>> transform_pose([(0, 0, 0), (1, 1, np.pi/4), (1, 1, np.pi/2)], [(1, 0, 0), (2, 0, 0), (3, 0, 0)])
    array([[ 1.        ,  0.        ,  0.        ],
           [ 2.41421356,  2.41421356,  0.78539816],
           [ 1.        ,  4.        ,  1.57079633]])

    Applying a transformation to a list of poses (e.g., adding predicted movement to particles):
    >>> np.set_printoptions(suppress=True)
    >>> transform_pose([(0, 0, np.pi/2)], [(1, 0, 0), (2, 0, 0), (3, 0, 0)])
    array([[ 0.        ,  1.        ,  1.57079633],
           [ 0.        ,  2.        ,  1.57079633],
           [ 0.        ,  3.        ,  1.57079633]])
    """
    # Make variables the right shapes and types
    a = np.array(tf, dtype=np.double)
    b = np.array(pose, dtype=np.double)
    a_ = a.reshape(-1, 3)
    b_ = b.reshape(-1, 3)
    result = np.zeros(np.broadcast(tf, pose).shape, dtype=np.double).reshape((-1, 3))

    # Do the actual calculation
    result[:, 0] = a_[:, 0] + b_[:, 0] * np.cos(a_[:, 2]) - b_[:, 1] * np.sin(a_[:, 2])
    result[:, 1] = a_[:, 1] + b_[:, 0] * np.sin(a_[:, 2]) + b_[:, 1] * np.cos(a_[:, 2])
    result[:, 2] = a_[:, 2] + b_[:, 2]

    # Make sure angle is in [-pi, pi)
    result[:, 2] = wraptopi(result[:, 2])
    return result.reshape(np.broadcast(tf, pose).shape)


def inverse_transform(tf):
    """Return the inverse transformation of 'tf'.

    'tf' can be a single transformation, or a list of transformations.
    """
    # Make variables the rights shapes and types
    tf = np.array(tf, dtype=np.double)
    tf_ = tf.reshape(-1, 3)
    result = np.zeros_like(tf_)

    # Do the actual calculation
    result[:, 0] =  np.cos(tf_[:, 2]) * (-tf_[:, 0]) + np.sin(tf_[:, 2]) * (-tf_[:, 1])
    result[:, 1] = -np.sin(tf_[:, 2]) * (-tf_[:, 0]) + np.cos(tf_[:, 2]) * (-tf_[:, 1])
    result[:, 2] = -tf_[:, 2]

    # Make sure angle is in [-pi, pi)
    result[:, 2] = wraptopi(result[:, 2])
    return result.reshape(tf.shape)


def find_transform(posea, poseb):
    """Return the transformation from `posea` to `poseb`.

    The arguments can be array-likes of broadcastable shapes as in transform_pose.
    """
    return transform_pose(inverse_transform(posea), poseb)
