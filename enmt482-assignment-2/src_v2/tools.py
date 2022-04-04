import numpy as np

import robodk as rdk
import robot_lib
import util

robot = robot_lib.robot


# There is 50degree offset between TCP and master frames
M_TCP_theta =- 50
T_M_TCP = util.get_T_given_P_and_R(None, util.get_R_given_theta(M_TCP_theta, 'z'))

# Grinder tool transformations
P_TCP_grinderToolPush = np.array([[0],[0],[102.82]])
R_TCP_grinderToolPush = util.R_NO_ROT
T_TCP_grinderToolPush = util.get_T_given_P_and_R(P_TCP_grinderToolPush, R_TCP_grinderToolPush)

P_TCP_grinderToolLever = util.np_col(-50, 0, 67.06)
R_TCP_grinderToolLever = util.mm(util.get_R_given_theta(90, 'y'), util.get_R_given_theta(90, 'x'))
T_TCP_grinderToolLever = util.get_T_given_P_and_R(P_TCP_grinderToolLever, R_TCP_grinderToolLever)

# Filter tool transformations
P_TCP_filterToolEdge = np.array([[-32.0],[0],[27.56]])
R_TCP_filterToolEdge = util.get_R_given_theta(-7.5, 'y')
T_TCP_filterToolEdge = util.get_T_given_P_and_R(P_TCP_filterToolEdge,  R_TCP_filterToolEdge)

P_TCP_filterToolCentre = np.array([[4.71],[0],[144.76]])
R_TCP_filterToolCentre = util.get_R_given_theta(-7.5, 'y')
T_TCP_filterToolCentre = util.get_T_given_P_and_R(P_TCP_filterToolCentre,  R_TCP_filterToolCentre)

# Cup tool transformations
P_TCP_cupToolCentre = np.array([[-47.0],[0],[186.11]])
R_TCP_cupToolCentre = util.R_NO_ROT
T_TCP_cupTool = util.get_T_given_P_and_R(P_TCP_cupToolCentre, R_TCP_cupToolCentre)


J_tool_approach = [-156.784696, -81.045663, -75.493125, -113.658042, 89.509670, -183.065198]

def get_filter_tool(is_pickup=True):
    # Define specific transforms:
    # J_tool_approach = [-157.180000, -79.150000, -79.150000, -109.790000, 90.070000, -164.890000]

    # Move to filter tool pick up position
    robot.MoveJ(J_tool_approach, blocking=True)
    # Pick up / drop off tool

    if (is_pickup):
        robot_lib.filter_tool_attach.RunCode() # call subprogram
    else:
        robot_lib.filter_tool_detach.RunCode() # call subprogram

    robot_lib.better_wait_finished()  # to allow subprogram to complete


def get_grinder_tool(is_pickup=True):
    # Define specific transforms:
    # J_tool_approach = [-151.880896, -97.616411, -59.103383, -112.890980, 90.242082, -161.879346]

    # Move to filter tool pick up position
    robot.MoveJ(J_tool_approach, blocking=True)
    # Pick up / drop off tool

    if (is_pickup):
        robot_lib.grinder_tool_attach.RunCode() # call subprogram
    else:
        robot_lib.grinder_tool_detach.RunCode() # call subprogram

    robot_lib.better_wait_finished()  # to allow subprogram to complete


def get_cup_tool(is_pickup=True):
    # Define specific transforms:
    # J_tool_approach = [-151.880896, -97.616411, -59.103383, -112.890980, 90.242082, -161.879346]

    # Move to filter tool pick up position
    robot.MoveJ(J_tool_approach, blocking=True)
    
    # Pick up / drop off tool
    if (is_pickup):
        robot_lib.cup_tool_attach.RunCode() # call subprogram
    else:
        robot_lib.cup_tool_detach.RunCode() # call subprogram

    robot_lib.better_wait_finished()  # to allow subprogram to complete


#_____________|  CUP TOOL MANIPULATION TASKS  |_________________#
# change in posisitions for smoother cup grabs and releases
# Height of cup (add this to points on espresso @ cup stand)
dP_B_cupHeight = util.np_col(0,0,100)
# Small change in height to move to ensure weight of cup isn't on tool
dP_E_cupReleaseDrop = util.np_col(0,0,-20) 
# clear cup range for linear move so we don't knock over cup
dP_E_cupReleaseRetreat = util.np_col(50,0,0)

# Function to release cup in a very smooth way - NOTE: DEPRICATED
# Assumes cup is in dropoff position with cup tool in resting pos
def smooth_cup_release():
    # Open cup tool
    robot_lib.run_program_cup_tool_open()

    # Move down slightly
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_M_TCP, dP_E_cupReleaseDrop), blocking=True)
    # Clear cup area
    robot.MoveJ(util.transform_RDK_pose(robot.Pose(), T_M_TCP, util.np_col(0,80,0)), blocking=True)
    #robot.MoveJ(util.transform_RDK_pose(robot.Pose(), T_BC, util.np_col(0,0,80)), blocking=True)

    # Close cup again
    robot_lib.run_program_cup_tool_close()


# Smooth cup collect for a given foreign frame
def smooth_cup_collect(T_BC):
    # Open cup tool
    robot_lib.run_program_cup_tool_open()

    # Move to coffee cup
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_BC, -dP_E_cupReleaseRetreat), blocking=True)
    # Raise slightly to pickup
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_BC, -dP_E_cupReleaseDrop), blocking=True)

    # Close cup tool
    robot_lib.run_program_cup_tool_close()

    # Clear cup area
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_BC, dP_E_cupReleaseRetreat), blocking=True)

