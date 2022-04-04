import numpy as np
import robodk as rdk

import robot_lib
import tools
import util

robot = robot_lib.robot


# Espresso frame finding points
P0_B_espresso = np.array([[-365.5],[-386.7],[349.8]]) # Point 0 (origin) of espresso machine
P1_B_espresso = np.array([[-577.8],[-441.6],[349.4]])
P1_espresso_E = np.array([[0],[218],[0]])

# Position of first button
P_E_btn1 = np.array([[50.67],[35.25],[-27.89]])

# Espresso frame in base frame
T_B_E = util.get_T_to_foreign_frame(P1_B_espresso, P0_B_espresso, P1_espresso_E)

# Button 1 frame in espresso frame
T_E_btn1 = util.get_T_given_P_and_R(P_E_btn1, util.get_R_given_theta(-90, 'y'))
dP_pushTool_depressBtn = util.np_col(-8,0,0)
dP_pushTool_Btn1MovDown = util.np_col(0,0,-10)

# Cup position frames
P_E_cupTray = np.array([[-12.68],[72.0],[-290]]) # Position ON TRAY to place the coffee
P_E_cupTrayDropPos = P_E_cupTray + tools.dP_B_cupHeight # Position of cup tool to pplace the coffee
R_E_cupTrayDropPos = np.matmul(util.get_R_given_theta(-90, 'y'), util.get_R_given_theta(-50, 'x'))

T_E_cupPickupApproach = util.get_T_given_P_and_R(P_E_cupTrayDropPos + tools.dP_E_cupReleaseDrop + tools.dP_E_cupReleaseRetreat, R_E_cupTrayDropPos)
T_E_cupDropoff = util.get_T_given_P_and_R(P_E_cupTrayDropPos, R_E_cupTrayDropPos)
T_B_cupEspressoDropoff = util.mm(T_B_E, util.inv(T_E_cupDropoff))


def press_espresso_button():
    # Define task-specific transforms
    T_B_M__espressoBtn1 = util.mm(T_B_E, T_E_btn1, util.inv(tools.T_TCP_grinderToolPush), util.inv(tools.T_M_TCP))

    # Move to target position
    robot.MoveJ(util.get_T2RDK(T_B_M__espressoBtn1), blocking=True)
    
    # Depress button
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_B_E, dP_pushTool_depressBtn), blocking=True)
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_B_E, -dP_pushTool_depressBtn), blocking=True)
    rdk.pause(3)
    # Turn button off
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_B_E, dP_pushTool_Btn1MovDown), blocking=True)
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_B_E, dP_pushTool_depressBtn), blocking=True)
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_B_E, -dP_pushTool_depressBtn), blocking=True)
    rdk.pause(1)



def dropoff_cup_at_espresso():
    # Add waypoints
    
    # Midway point to prevent flip
    robot.MoveJ([-91.34, 242.27, -154.02, 91.75, 84.16, -40], blocking=True)
    
    robot.MoveJ([-119.659477, 242.272234, -154.024426, 91.752193, 84.158336, -40.000000], blocking=True)

    # Move to target position
    T_B_M__espressoCupDropoff = util.mm(T_B_E, T_E_cupDropoff, util.inv(tools.T_TCP_cupTool), util.inv(tools.T_M_TCP))
    robot.MoveJ(util.get_T2RDK(T_B_M__espressoCupDropoff), blocking=True)

    # Drop off cup
    #smooth_cup_release(T_B_cupEspressoDropoff)

    robot_lib.run_program_cup_tool_open()

    # Move down slightly to remove weight from holder
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_E_cupPickupApproach, util.np_col(-10,0,0)), blocking=True)
    rdk.pause(2)
    # Clear cup area
    robot.MoveJ(util.transform_RDK_pose(robot.Pose(), T_E_cupPickupApproach, util.np_col(0,60,0)), blocking=True)
    #robot.MoveJ(util.transform_RDK_pose(robot.Pose(), T_BC, util.np_col(0,0,80)), blocking=True)
    rdk.pause(2)
    # Close cup again
    robot_lib.run_program_cup_tool_close()
    


def pickup_cup_at_espresso():
    # Add waypoints
    robot.MoveJ([-119.659477, 242.272234, -154.024426, 91.752193, 84.158336, -40.000000], blocking=True)

    # Move to approach position
    T_B_M__espressoCupPickupApproach = util.mm(T_B_E, T_E_cupPickupApproach, util.inv(tools.T_TCP_cupTool), util.inv(tools.T_M_TCP))
    robot.MoveJ(util.get_T2RDK(T_B_M__espressoCupPickupApproach), blocking=True)
    
    # Smooth cup collect
    tools.smooth_cup_collect(T_B_cupEspressoDropoff)


