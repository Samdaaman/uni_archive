import numpy as np
import robodk as rdk

import robot_lib
import tools
import util

robot = robot_lib.robot

# Toolstand frame finding points
P0_B_toolStand = np.array([[-555.6],[-78.5],[19.05]]) # Point 0 (origin) of tool stand
P1_B_toolStand = np.array([[-645.0],[77.2],[19.05]])
P1_S_toolStand = np.array([[-127],[127],[0]])

# Toolstand frame in terms of base frame
T_B_S = util.get_T_to_foreign_frame(P1_B_toolStand, P0_B_toolStand, P1_S_toolStand)

# Tool approach points
P_S_grinderToolGet = np.array([[144.0],[-98.6],[515.0]]) 
P_S_filterToolGet = np.array([[144.0],[-98.6],[515.0]]) 
P_S_cupToolGet = np.array([[144.0],[-98.6],[515.0]]) 

# Group head
P_S_groupHead = util.np_col(9.5, 67.3, 167.0)
R_S_groupHead = util.get_R_given_theta(-90, 'y')

T_S_groupHead = util.get_T_given_P_and_R(P_S_groupHead, R_S_groupHead)
T_S_groupHeadRotated = util.get_T_given_P_and_R(P_S_groupHead, np.matmul(R_S_groupHead, util.get_R_given_theta(45, 'x')))


def insert_portafilter_in_group_head_and_release():
    # Define waypoints
    robot.MoveJ([-138.87, -88.94, -142.67, -115.05, -34.46, -231.06], blocking=True)

    # Move up to insersion position
    T_B_M__groupHead = util.mm(T_B_S, T_S_groupHead, util.inv(tools.T_TCP_filterToolCentre), util.inv(tools.T_M_TCP))
    robot.MoveJ(util.get_T2RDK(T_B_M__groupHead), blocking=True)

    # Rotate 45 degrees
    T_B_M__groupHeadRotated = util.mm(T_B_S, T_S_groupHeadRotated, util.inv(tools.T_TCP_filterToolCentre), util.inv(tools.T_M_TCP))
    robot.MoveL(util.get_T2RDK(T_B_M__groupHeadRotated), blocking=True)

    rdk.pause(2)

    # Drop away from group head
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_B_S, util.np_col(0,0,-50)), blocking=True)

    # Move a short distance away
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), tools.T_M_TCP, deltaR_C=util.get_R_given_theta(-100, 'z')), blocking=True)

    # Wait while TA detach tool
    rdk.pause(3) 
    # filter_tool_detach_silvia.RunProgram(filter_tool_detach_silvia)
    # rdk.pause(TOOL_ATTACH_SUBPROGRAM_TIME)  # to allow subprogram to complete
    # reset_robot_frame() # reset world frame back to robot frame


