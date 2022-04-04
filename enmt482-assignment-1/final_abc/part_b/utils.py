"""Utility functions for particle filter assignment.

M.P. Hayes and M.J. Edwards,
Department of Electrical and Computer Engineering
University of Canterbury
"""

import numpy as np
import bisect
from numpy.random import uniform



distributions = ['uniform', 'gaussian']

def pdf(x, muX, sigmaX, distribution):

    if distribution == 'gaussian':
        return gauss(x, muX, sigmaX)
    elif distribution == 'uniform':
        xmin = muX - np.sqrt(12) * sigmaX / 2
        xmax = muX + np.sqrt(12) * sigmaX / 2
        return 1.0 * ((x >= xmin) & (x <= xmax)) / (xmax - xmin)
    raise ValueError('Unknown distribution %s' % distribution)


def gauss(v, mu=0, sigma=1):
    """Evaluate a Gaussian PDF."""
    
    return np.exp(-0.5 * ((v - mu) / sigma)**2) / (np.sqrt(2 * np.pi) * sigma)


###############################################################################
# Particle filter functions

def resample(particles, weights):
    """Resample particles in proportion to their weights.

    Particles and weights should be arrays, and will be updated in place.
    """
    cum_weights = np.cumsum(weights)

    if cum_weights[-1] == 0.0:
        print('All weights are zero, giving up...')
        return False

    cum_weights /= cum_weights[-1]

    new_particles = []
    for _ in particles:
        # Copy a particle into the list of new particles, choosing based
        # on weight
        m = bisect.bisect_left(cum_weights, uniform(0, 1))
        p = particles[m]
        new_particles.append(p)

    # Replace old particles with new particles
    for m, p in enumerate(new_particles):
        particles[m] = p

    # Reset weights
    weights[:] = 1
    return True


def is_degenerate(weights):
    """Return true if the particles are degenerate and need resampling."""
    
    weights_sum = np.sum(weights)
    w = weights / weights_sum
    return 1.0 / np.sum(w**2) < 0.5 * len(w)

def score_array(a: np.ndarray):
    """
        np.argsort returns an array of indices which can be used create a new sorted array from the original array.
            the indices it returns are the lowest to highest elements of the array in sorted order

        score_array is used to score indices in the array. The lowest item gets 0, second lowest 1, etc....

        example
            >>> score_array(np.array((2, 5, 3, 1)))
                (1, 3, 2, 0)

            whereas
            >>> np.argsort(np.array((2, 5, 3, 1)))
                (3, 0, 2, 1)
    """
    argsort_indices = np.argsort(a)
    scores = np.empty(len(a), dtype=int)
    for i, argsort_index in enumerate(argsort_indices):
        scores[argsort_index] = i # set the ith lowest element (scores[argsort_index]) the score i-1
    return scores


###############################################################################
# Functions for working with angles (in radians)

def wrapto2pi(angle):
    """Convert angle into range [0, 2 * pi)."""
    return angle % (2 * np.pi)


def wraptopi(angle):
    """Convert angle into range [-pi, pi)."""
    return wrapto2pi(angle + np.pi) - np.pi


def angle_difference(angle1, angle2):
    """Return principal difference angle, angle2 - angle1  with return value in range [-pi, pi)."""
    return ((((angle2 - angle1) % (2 * np.pi)) + (3 * np.pi)) % (2 * np.pi)) - np.pi
