"""Plotting functions for particle filter assignment.

M.P. Hayes and M.J. Edwards,
Department of Electrical and Computer Engineering
University of Canterbury
"""

import matplotlib.pyplot as plt
import numpy as np


def plot_poses(axes, poses, colour='green'):
    x = poses[:, 0]
    y = poses[:, 1]
    theta = poses[:, 2]

    dx = 0.015 * np.cos(theta)
    dy = 0.015 * np.sin(theta)

    x, y = y, x
    dx, dy = dy, dx

    # Plot arrows representing poses
    for m in range(len(x)):
        axes.arrow(x[m], y[m], dx[m], dy[m], color=colour)


def plot_particles(axes, poses, weights, colourmap='viridis'):
    cmap = plt.get_cmap(colourmap)

    weights = np.log(weights + 1e-4)

    weight_min = weights.min()
    weight_max = weights.max()

    if weight_min == weight_max:
        idx = weights*0 + 1
    else:
        idx = (weights - weight_min)/(weight_max - weight_min)
    colours = cmap(idx, alpha=0.5)

    x = poses[:, 0]
    y = poses[:, 1]
    theta = np.degrees(poses[:, 2])

    x, y = y, x



    if hasattr(axes, 'particles'):

        # Ammend axes if num particles has changed
        delta_particles = len(x) - len(axes.particles)
        if (delta_particles > 0): # more particles than plots
            for m in range(len(axes.particles), len(x)):
                l, = axes.plot(x[m], y[m], marker=(3, 0, theta[m] + 180),
                       color=colours[m], markersize=5, linestyle='')
                axes.particles.append(l)
        elif (delta_particles < 0): # less particles than plots
            try:
                for axe in range(len(x), len(axes.particles)):
                    axes.particles[axe].remove()
            except:
                return
            #axes.particles = axes.particles[0: len(x)]
            

            
        for m in range(len(x)):

            axes.particles[m].set_data(x[m], y[m])
            axes.particles[m].set_marker((3, 0, theta[m] + 180))
            axes.particles[m].set_color(colours[m])

        axes.figure.canvas.draw()
        axes.figure.canvas.flush_events()
        return

    axes.particles = []
    for m in range(len(x)):
        l, = axes.plot(x[m], y[m], marker=(3, 0, theta[m] + 180),
                       color=colours[m], markersize=5, linestyle='')
        axes.particles.append(l)

    


def plot_beacon_guesses(axes, guesses, colourmap='viridis', no_beacon=False):

    # Clear the beacon guesses
    if (no_beacon):
        if hasattr(axes, 'beacon_guesses'):
            axes.beacon_guesses[0].set_data(2,5)
            axes.beacon_guesses[0].set_marker((3, 0, 180))
        return

    x = guesses[:, 0]
    y = guesses[:, 1]
    theta = np.degrees(guesses[:, 2])

    x, y = y, x

    if hasattr(axes, 'beacon_guesses'):
        for m in range(len(x)):
            axes.beacon_guesses[m].set_data(x[m], y[m])
            axes.beacon_guesses[m].set_marker((3, 0, theta[m] + 180))

        # axes.figure.canvas.draw()
        # axes.figure.canvas.flush_events()
        return

    axes.beacon_guesses = []
    for m in range(len(x)):
        l, = axes.plot(x[m], y[m], marker=(3, 0, theta[m] + 180), markersize=5, linestyle='')
        axes.beacon_guesses.append(l)


def plot_path(axes, poses, fmt='-', label=None):
    x = poses[..., 0]
    y = poses[..., 1]

    x, y = y, x

    axes.plot(x, y, fmt, label=label)
    return


def plot_path_with_visibility(axes, poses, fmt='-', colours=('red', 'green'),
                              label=None, visibility=None):
    """Plot path showing where beacons are visible.

    'visibility' is a boolean array to indicate where beacons are visible
    'colours' sets the path colour for invisible and visible beacons
    """
    
    if visibility is None:
        return plot_path(axes, fmt, label)

    x = poses[..., 0]
    y = poses[..., 1]

    if len(x) == 0:
        return

    x, y = y, x

    m = 0
    k = 0
    for n in range(1, len(x)):
        if visibility[n] == visibility[m]:
            continue
        axes.plot(x[k:n], y[k:n], fmt, label=label, color=colours[visibility[m]])
        m = n
        k = n - 1

    axes.plot(x[k:n], y[k:n], fmt, label=label, color=colours[visibility[m]])


def plot_beacons(axes, beacons, colour='blue', label=None):
    """Plot beacon poses."""

    x = beacons[:, 0]
    y = beacons[:, 1]
    theta = beacons[:, 2]

    xdx = 0.5 * np.cos(theta)
    xdy = 0.5 * np.sin(theta)
    ydx = 0.5 * np.cos(theta + np.pi/2)
    ydy = 0.5 * np.sin(theta + np.pi/2)
    
    x, y = y, x
    xdx, xdy = xdy, xdx
    ydx, ydy = ydy, ydx        

    axes.plot(x, y, 'o', color=colour, label=label)

    axes.plot((x, x + xdx), (y, y + xdy), color='red')
    axes.plot((x, x + ydx), (y, y + ydy), color='green')
    
    for m in range(len(x)):
        axes.text(x[m], y[m], '%d' % m)        


def clean_poses(poses):
    """Null out jumps in SLAM poses by replacing with NaN."""
    clean_poses = np.copy(poses)
    last_good = 0
    for i in range(1, len(clean_poses)):
        dist = np.sqrt(np.sum((clean_poses[i, :2] - clean_poses[last_good, :2])**2))
        if dist > 2:
            clean_poses[i] = np.nan
        else:
            last_good = i
    return clean_poses
