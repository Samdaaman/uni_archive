import matplotlib.pyplot as plt
from typing import Union
import math

ARENA_WIDTH = 2.4
ARENA_LENGTH = 4.9
DIVISION_WIDTH = 0.2


def plot_grid():
    return


def plot_ball_path(start_angle: Union[float, int], max_distance: Union[float, int]):
    distance = 0.0
    x = 0
    y = ARENA_LENGTH / 2.0
    x_all = [x]
    y_all = [y]
    angle = start_angle

    print((x, y, angle))
    while distance < max_distance:
        dx = 0.0
        dy = 0.0

        if angle == math.pi / 2 or angle == -math.pi / 2:
            print('its 90 deg time')
            dy = (angle / math.pi * 2) * ARENA_LENGTH
            angle = -angle
        else:
            test_x_dist = ((ARENA_WIDTH - x) if math.pi / 2 > angle > -math.pi / 2 else -x)
            test_y_dist = math.tan(angle) * test_x_dist
            if test_y_dist + y == ARENA_LENGTH:
                print('we hit a corner')
                exit(1)
            elif 0 < test_y_dist + y < ARENA_LENGTH:
                # hit side
                dx = test_x_dist
                dy = test_y_dist
                angle = math.pi - angle
            else:
                # hit top/bottom
                dy = ((ARENA_LENGTH - y) if angle > 0 else -y)
                dx = dy / math.tan(angle)
                angle = - angle

        distance += math.sqrt(dx*dx + dy*dy)

        while angle > math.pi:
            angle -= 2 * math.pi
        while angle <= -math.pi:
            angle += 2 * math.pi
        # input()

        x += dx
        y += dy
        x_all.append(x)
        y_all.append(y)
        print((x, y, angle))

    coordinates = [i for i in zip(x_all, y_all)]
    lines = []
    for i in range(len(coordinates) - 1):
        lines.append(([coordinates[i][0], coordinates[i+1][0]], [coordinates[i][1], coordinates[i+1][1]]))

    for line in lines:
        plt.plot(line[0], line[1])
    plt.show()


def main():
    plot_ball_path(math.pi / 3, 30)


if __name__ == '__main__':
    main()
