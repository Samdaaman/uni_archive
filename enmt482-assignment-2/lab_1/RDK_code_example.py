# A quick example to get you moving with the RoboDK python API
# To run this, you will need a target in your RoboDK station called 'home'
# you can just create this anywhere

from robolink.robolink import Robolink

RDK = Robolink()


robot = RDK.Item('UR5')
world_frame = RDK.Item('UR5 Base')
target = RDK.Item('home') # You might have a target called 'home' in your station - if you dont for this example, you'll get an error

servo_positions = [[0.000000, -90.000000, -90.000000, 0.000000, 90.000000, -0.000000],
                   [2.000000, -91.000000, -90.000000, 0.000000, 90.000000, -0.000000],
                   [4.000000, -92.000000, -90.000000, 0.000000, 90.000000, -0.000000],
                   [6.000000, -93.000000, -90.000000, 0.000000, 90.000000, -0.000000],
                   [8.000000, -94.000000, -90.000000, 0.000000, 90.000000, -0.000000],
                   [10.000000, -95.000000, -90.000000, 0.000000, 90.000000, -0.000000],
                    [8.000000, -96.000000, -90.000000, 0.000000, 90.000000, -0.000000],
                    [6.000000, -97.000000, -90.000000, 0.000000, 90.000000, -0.000000],
                    [4.000000, -98.000000, -90.000000, 0.000000, 90.000000, -0.000000],
                    [2.000000, -99.000000, -90.000000, 0.000000, 90.000000, -0.000000],
                    [0.000000, -100.000000, -90.000000, 0.000000, 90.000000, -0.000000]]    # define some joint angles


# first, move home:
robot.MoveJ(target, blocking=True)


# servo through joint positions using MoveJ
for ii in servo_positions:
    robot.MoveJ(ii)
#end for

robot.MoveL(target, blocking=True)
