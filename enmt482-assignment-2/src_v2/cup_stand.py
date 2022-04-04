import numpy as np
import robodk as rdk

import robot_lib
import tools
import util

robot = robot_lib.robot


# Cupstand frame finding points
P0_B_cupStand = np.array([[-1.5],[-600.3],[-20]]) # cup stand frame origin in base frame coords
R_B_cupStandBase = util.get_R_given_theta(90, 'z')

T_B_CS = util.get_T_given_P_and_R(P0_B_cupStand, R_B_cupStandBase)

P_CS_cupPickup = np.array([[0],[0],[217]])
R_CS_cupPickup  = util.get_R_given_theta(-90, 'y')
# Make sure cup tool is rotated 180 deg for pickup
R_CS_cupPickup  = np.matmul(util.get_R_given_theta(np.deg2rad(-90), axis='y'), util.get_R_given_theta(180, 'z'))

T_CS_cupPickupApproach = util.get_T_given_P_and_R(P_CS_cupPickup + tools.dP_E_cupReleaseRetreat + util.np_col(0,0,-30), R_CS_cupPickup)
T_CS_cupPickup = util.get_T_given_P_and_R(P_CS_cupPickup, R_CS_cupPickup)

# Cup drop off position (offset by cup)
R_CS_cupDropoffApproach = util.get_R_given_theta(-90, 'y')
T_CS_cupDropoffApproach = util.get_T_given_P_and_R(P_CS_cupPickup + 1.5*tools.dP_B_cupHeight, R_CS_cupDropoffApproach)


def pickup_coffee_cup_from_stand():
    # Move to cup stand and rotate cup tool by 180deg 
    T_B_M__cupStackPickupApproach = util.mm(T_B_CS, T_CS_cupPickupApproach, util.inv(tools.T_TCP_cupTool), util.inv(tools.T_M_TCP))
    robot.MoveJ(util.get_T2RDK(T_B_M__cupStackPickupApproach), blocking=True)

    # Open cup tool
    robot_lib.run_program_cup_tool_open()

    # Approach cup
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_B_CS, -tools.dP_E_cupReleaseRetreat), blocking=True)
    
    # Close cup tool
    robot_lib.run_program_cup_tool_close()
    
    # Clear cup tool
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_B_CS, tools.dP_B_cupHeight), blocking=True)
    # Retreat from cup
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_B_CS, tools.dP_E_cupReleaseRetreat), blocking=True)
    
    # Rotate cup as having it util.inverted messes up navigation
    robot.MoveJ(util.transform_RDK_pose(robot.Pose(), T_B_CS, deltaR_C=util.get_R_given_theta(-60, 'z')), blocking=True)
    # TODO: Rotate 6th joint 180degrees to flip cup right way
    

def dropoff_coffee_cup_on_stand():
    # Move to cup stand
    T_B_M__cupStackDropoffApproach = util.mm(T_B_CS, T_CS_cupDropoffApproach, util.inv(tools.T_TCP_cupTool), util.inv(tools.T_M_TCP))
    robot.MoveJ(util.get_T2RDK(T_B_M__cupStackDropoffApproach), blocking=True)

    # Drop off cup
    #smooth_cup_release(T_B_CS)

    # Open cup tool
    robot_lib.run_program_cup_tool_open()

    # Move down slightly to remove weight from holder
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_B_CS, util.np_col(0,0,-10)), blocking=True)
    rdk.pause(1)
    # Clear cup area
    robot.MoveJ(util.transform_RDK_pose(robot.Pose(), T_B_CS, util.np_col(50,0,0)), blocking=True)
    #robot.MoveJ(util.transform_RDK_pose(robot.Pose(), T_BC, util.np_col(0,0,80)), blocking=True)
    rdk.pause(1)
    # Close cup again
    robot_lib.run_program_cup_tool_close()




