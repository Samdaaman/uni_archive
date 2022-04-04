#
#                    CUP COLLECT PROCEDURES
#
#
#        >(')____,  >(')____,  >(')____,  >(')____,  >(') ___,
#         (` =~~/    (` =~~/    (` =~~/    (` =~~/    (` =~~/
#    ~^~^`---'~^~^~^`---'~^~^~^`---'~^~^~^`---'~^~^~^`---'~^~^~
#
#************************************************************************#


#  Authors:        Zach Preston        zsp10
#                  Sam Hogan           sho119
#
#  Date created: 18th September 2021
#  Date Last Modified: 
#
#  Module Description:
# 


from numpy.lib.ufunclike import isposinf
import robolink as rl    # RoboDK API
import robodk as rdk     # Robot toolbox
import numpy as np       # Include numpy


#_____________________________________________________________________________________
#                                                                                     #
#                              INITIALISE ROBODK API CONFIG                           #
#                                                                                     #
#_____________________________________________________________________________________#

# Initialise robolink
RDK = rl.Robolink()

# Initialise robot & starting parameters
robot = RDK.Item('UR5')
target = RDK.Item('Home')   # existing target in station

world_frame = RDK.Item('UR5 Base')  # Define world frame
robot.setPoseFrame(world_frame) # Set robot pose frame to world frame
robot.setPoseTool(robot.PoseTool())


# Define Existing subprograms
grinder_tool_attach = RDK.Item("Grinder Tool Attach (Tool Stand)")
grinder_tool_detach = RDK.Item("Grinder Tool Detach (Tool Stand)")
filter_tool_attach = RDK.Item("Portafilter Tool Attach (Tool Stand)")
filter_tool_detach = RDK.Item("Portafilter Tool Detach (Tool Stand)")
cup_tool_attach = RDK.Item("Cup Tool Attach (Stand)")
cup_tool_detach = RDK.Item("Cup Tool Detach (Stand)")

filter_tool_attach_grinder = RDK.Item("Portafilter Tool Attach (Grinder)")
filter_tool_detach_grinder = RDK.Item("Portafilter Tool Detach (Grinder)")

cup_tool_open = "Cup Tool Open" #RDK.Item("Cup Tool Open")
cup_tool_close = "Cup Tool Close"

CUP_TOOL_SUBPROGRAM_TIME = 1 # seconds
TOOL_ATTACH_SUBPROGRAM_TIME = 2 # seconds

# Define empty vectors
R_NO_ROT = np.identity(3)
P_NO_DISP = np.zeros((3,1))



#_____________________________________________________________________________________
#                                                                                     #
#                          NUMPY WRAPPER & UTILITY FUNCTIONS                          #
#                                                                                     #
#_____________________________________________________________________________________#

# These functions help to keep the code tidy and readable

def get_T2RDK(T_AB):
    """Returns RDK matrix of numpy T_AB matrix"""
    return rdk.Mat(T_AB.tolist())

def get_RDK2T(T_AB_RDK):
    """Returns numpy T matrix given an RDK Mat matrix"""

    temp = T_AB_RDK.rows
    T_np = np.array([[temp[0][0], temp[0][1], temp[0][2], temp[0][3]],
    [temp[1][0], temp[1][1], temp[1][2], temp[1][3]], [temp[2][0], temp[2][1], 
    temp[2][2], temp[2][3]], [temp[3][0], temp[3][1], temp[3][2], temp[3][3]]])

    return T_np

def np_col(*vals):
    """Simple function to return numpy columnvector"""  
    return np.reshape(np.array(vals), (3,1))


def mm(*matrices):
    """Simple wrapper function to perform matrix multiplication (m.m.) multiple times"""
    res = np.identity(matrices[0].shape[0])
    for matrix in matrices:
        assert matrix.shape[0] == matrix.shape[1] and matrix.ndim == 2, 'Only supports square matrices'
        res = np.matmul(res, matrix)
    return res

def inv(m):
    """Simple wrapper function to invert a square np matrix"""
    return np.linalg.inv(m)


#_____________________________________________________________________________________
#                                                                                     #
#                      TRANSFORM MATRIX PROCESSING FUNCTIONS                          #
#                                                                                     #
#_____________________________________________________________________________________#

def get_R_given_theta(theta, axis='z', isDeg=False):
    """Returns the rotation matrix for a single-axis rotation"""
   
    # convert theta to radians
    if isDeg:
        theta = np.deg2rad(theta)
    # generate matrix for z rotation
    if axis=='z':
        R_AB = np.array([[np.cos(theta), -np.sin(theta), 0],
                    [np.sin(theta), np.cos(theta), 0],
                    [0, 0, 1]])
    elif axis=='y':
        R_AB = np.array([[np.cos(theta), 0, np.sin(theta)],
                    [0, 1, 0],
                    [-np.sin(theta), 0, np.cos(theta)]])

    elif axis=='x':
        R_AB = np.array([[1, 0, 0],
                    [0, np.cos(theta), -np.sin(theta)],
                    [0, np.sin(theta), np.cos(theta)]])
    else:
        pass
    
    # Return rotation matrix rounded to 10 decimal places
    # this prevents massive negative numbers from degree conversions
    return np.around(R_AB, 10)


# Constructs T transform given displacement and rotation matrix
def get_T_given_P_and_R(P_BA, R_AB):
    T_AB = np.concatenate((R_AB, P_BA), axis=1)
    T_AB = np.concatenate((T_AB, np.array([[0,0,0,1]])), axis=0)
    return T_AB


# Returns a transform function of point Q in frame B (typically base frame) given three points:
#   P_AQ -> point Q in frame A (ie second point on tool stand)
#   P_AB -> frame B origin in frame A
#   P+QB -> point Q in frame B
def get_T_to_foreign_frame(P_AQ, P_AB, P_QB):
    # It can be seen (in docs) that frame rotations only ever occur about the Z axis
    # Therefore we can determine the row vector 
    # 1. determine angle between x-axes of frame A and B (rotation about Z)
    P_BQ = P_AQ-P_AB
    # theta is angle about x-axis of frame A to frame B (first element)
    # if (force_theta):
    #     theta=force_theta
    # else: ie param -> force_theta=np.cos(np.deg2rad(90)
    theta = ( np.arctan2(P_BQ[1], P_BQ[0]) - np.arctan2(P_QB[1],P_QB[0]) )[0]  

    # 2. Determine rotation matrix between frames (Z-axis rotation)
    R_AB = get_R_given_theta(theta, axis='z')

    # 3. Create frame transform matrix
    T_BQ = np.concatenate((R_AB, P_AB), axis=1)
    T_BQ = np.concatenate((T_BQ, np.array([[0,0,0,1]])), axis=0)

    return T_BQ


#_____________________________________________________________________________________
#                                                                                     #
#                           ROBODK POSE MANIPULATION FUNCTIONS                        #
#                                                                                     #
#_____________________________________________________________________________________#

# Transform a given RDK pose by a specified foreign translation (deltaP) in frame C and rotation (uvw)
#   T_B_C -> assumes transformations are given in base frame (B) (frame the pose is given in)
#           by passing in T_B_C, translations in foreign frame C (ie espresso frame) may be given
def transform_RDK_pose(pose, T_B_C=np.identity(4), deltaP_C=P_NO_DISP, deltaR_C=R_NO_ROT):
    
    # 1. Convert pose to numpy array
    # robot pose tells us T_B_M (master tool frame relative to the base frame)
    T_B_M1 = get_RDK2T(pose)

    # 2. get M1 in terms of foreign frame C
    # Ie Determine master tool frame in terms of Espresso frame (E)
    T_C_M1 = mm(inv(T_B_C), T_B_M1)
    
    # 3. Update the displacement and angle changes made to (M) master tool frame 
    # in frame C perspective
    T_C_M2 = T_C_M1
    T_C_M2[:3,3:] += deltaP_C # Add deltaP displacement to the P column
    T_C_M2[:3,:3] = np.matmul(deltaR_C, T_C_M2[:3,:3])

    # 4. Determine pose of M2 in the base frame (B)
    T_B_M2 = mm(T_B_C, T_C_M2)

    # Return executable code
    return get_T2RDK(T_B_M2)

# Call this after running subprogram
def reset_robot_frame():
    robot.setPoseFrame(world_frame) # Reset frame to base frame (B)
    robot.setPoseTool(robot.PoseTool()) # Reset tool frame

#_____________________________________________________________________________________
#                                                                                     #
#                        POSITION, POSE & WAYPOINT DECLARATIONS                       #
#                                                                                     #
#_____________________________________________________________________________________#

#_______________________|   FRAME KEYS   |_________________________#

#   B -> baseframe
#   M -> master tool point frame
#   TCP -> tool centre point frame

#   E -> espresso machine frame
#   S -> toolstand frame
#   G -> grinder frame

#   cupTool -> cup tool frame
#   grinderTool -> grinder tool


#__________________|   BASE FRAME TRANSFORMS   |___________________#

# Directly use the RDK Matrix object from to hold pose (its an HT)
T_home = rdk.Mat([[     0.000000,     0.000000,     1.000000,   523.370000 ],
    [-1.000000,     0.000000,     0.000000,  -109.000000 ],
    [-0.000000,    -1.000000,     0.000000,   607.850000 ],
    [0.000000,     0.000000,     0.000000,     1.000000 ]])

# There is 50degree offset between TCP and master frames
M_TCP_theta = np.deg2rad(-50)
T_M_TCP = get_T_given_P_and_R(P_NO_DISP, get_R_given_theta(M_TCP_theta))


#__________________|  TOOL TRANSFORM PARAMETERS   |____________________#

# Grinder tool transformations
P_TCP_grinderToolPush = np.array([[0],[0],[102.82]])
R_TCP_grinderToolPush = R_NO_ROT

T_TCP_grinderToolPush = get_T_given_P_and_R(P_TCP_grinderToolPush,  R_TCP_grinderToolPush)

P_TCP_grinderToolLever = np_col(-50, 0, 67.06)
R_TCP_grinderToolLever = mm(get_R_given_theta(90, axis='y', isDeg=True), get_R_given_theta(90, axis='x', isDeg=True))

T_TCP_grinderToolLever = get_T_given_P_and_R(P_TCP_grinderToolLever, R_TCP_grinderToolLever)

# Filter tool transformations
P_TCP_filterToolEdge = np.array([[-32.0],[0],[27.56]])
R_TCP_filterToolEdge = get_R_given_theta(np.deg2rad(-7.5), axis='y')

T_TCP_filterToolEdge = get_T_given_P_and_R(P_TCP_filterToolEdge,  R_TCP_filterToolEdge)

P_TCP_filterToolCentre = np.array([[4.71],[0],[144.76]])
R_TCP_filterToolCentre = get_R_given_theta(np.deg2rad(-7.5), axis='y')

T_TCP_filterToolCentre = get_T_given_P_and_R(P_TCP_filterToolCentre,  R_TCP_filterToolCentre)


# Cup tool transformations
P_TCP_cupToolCentre = np.array([[-47.0],[0],[186.11]])
R_TCP_cupToolCentre = R_NO_ROT

T_TCP_cupTool = get_T_given_P_and_R(P_TCP_cupToolCentre, R_TCP_cupToolCentre)




#__________________|  TOOL STAND PARAMETERS   |____________________#

# Toolstand frame finding points
P0_B_toolStand = np.array([[-555.6],[-78.5],[19.05]]) # Point 0 (origin) of tool stand
P1_B_toolStand = np.array([[-645.0],[77.2],[19.05]])
P1_S_toolStand = np.array([[-127],[127],[0]])

# Toolstand frame in terms of base frame
T_B_S = get_T_to_foreign_frame(P1_B_toolStand, P0_B_toolStand, P1_S_toolStand)

# Tool approach points
P_S_grinderToolGet = np.array([[144.0],[-98.6],[515.0]]) 
P_S_filterToolGet = np.array([[144.0],[-98.6],[515.0]]) 
P_S_cupToolGet = np.array([[144.0],[-98.6],[515.0]]) 

# Group head
P_S_groupHead = np_col(9.5, 67.3, 167.0)
R_S_groupHead = get_R_given_theta(np.deg2rad(-90), axis='y')

T_S_groupHead = get_T_given_P_and_R(P_S_groupHead, R_S_groupHead)
T_S_groupHeadRotated = get_T_given_P_and_R(P_S_groupHead, np.matmul(R_S_groupHead, get_R_given_theta(np.deg2rad(45), axis='x')))


#____________________|  CUP STAND PARAMETERS   |______________________#

# change in posisitions for smoother cup grabs and releases
# Height of cup (add this to points on espresso @ cup stand)
dP_B_cupHeight = np_col(0,0,100)
# Small change in height to move to ensure weight of cup isn't on tool
dP_E_cupReleaseDrop = np_col(0,0,-20) 
# clear cup range for linear move so we don't knock over cup
dP_E_cupReleaseRetreat = np_col(50,0,0)


# Cupstand frame finding points
P0_B_cupStand = np.array([[-1.5],[-600.3],[-20]]) # cup stand frame origin in base frame coords
R_B_cupStandBase = get_R_given_theta(np.deg2rad(90))

T_B_CS = get_T_given_P_and_R(P0_B_cupStand, R_B_cupStandBase)

P_CS_cupPickup = np.array([[0],[0],[217]])
R_CS_cupPickup  = get_R_given_theta(np.deg2rad(-90), axis='y')
# Make sure cup tool is rotated 180 deg for pickup
R_CS_cupPickup  = np.matmul(get_R_given_theta(np.deg2rad(-90), axis='y'), get_R_given_theta(np.deg2rad(180), axis='z'))

T_CS_cupPickupApproach = get_T_given_P_and_R(P_CS_cupPickup + dP_E_cupReleaseRetreat + np_col(0,0,-30), R_CS_cupPickup)
T_CS_cupPickup = get_T_given_P_and_R(P_CS_cupPickup, R_CS_cupPickup)

# Cup drop off position (offset by cup)
R_CS_cupDropoffApproach = get_R_given_theta(np.deg2rad(-90), axis='y')
T_CS_cupDropoffApproach = get_T_given_P_and_R(P_CS_cupPickup + 1.5*dP_B_cupHeight, R_CS_cupDropoffApproach)


#__________________|  ESPRESSO MACHINE PARAMETERS   |____________________#

# Espresso frame finding points
P0_B_espresso = np.array([[-365.5],[-386.7],[349.8]]) # Point 0 (origin) of espresso machine
P1_B_espresso = np.array([[-577.8],[-441.6],[349.4]])
P1_espresso_E = np.array([[0],[218],[0]])

# Position of first button
P_E_btn1 = np.array([[50.67],[35.25],[-27.89]])

# Espresso frame in base frame
T_B_E = get_T_to_foreign_frame(P1_B_espresso, P0_B_espresso, P1_espresso_E)

# Button 1 frame in espresso frame
T_E_btn1 = get_T_given_P_and_R(P_E_btn1, get_R_given_theta(np.deg2rad(-90), axis='y'))
dP_pushTool_depressBtn = np_col(-8,0,0)
dP_pushTool_Btn1MovDown = np_col(0,0,-10)

# Cup position frames
P_E_cupTray = np.array([[-12.68],[72.0],[-290]]) # Position ON TRAY to place the coffee
P_E_cupTrayDropPos = P_E_cupTray #+ dP_B_cupHeight # Position of cup tool to pplace the coffee
R_E_cupTrayDropPos = np.matmul(get_R_given_theta(np.deg2rad(-90), axis='y'), get_R_given_theta(np.deg2rad(-50), axis='x'))

T_E_cupPickupApproach = get_T_given_P_and_R(P_E_cupTrayDropPos + dP_E_cupReleaseDrop + dP_E_cupReleaseRetreat, R_E_cupTrayDropPos)
T_E_cupDropoff = get_T_given_P_and_R(P_E_cupTrayDropPos, R_E_cupTrayDropPos)
T_B_cupEspressoDropoff = mm(T_B_E, inv(T_E_cupDropoff))




#__________________|  GRINDER MACHINE PARAMETERS   |____________________#

# Grinder frame finding points
P0_B_grinder = np_col(482.7, -432.1, 316.1)
P1_B_grinder = np_col(370.5, -322.5, 65.9)
P1_grinder_G = np_col(157.61, 0, -250.45)

T_B_G = get_T_to_foreign_frame(P1_B_grinder, P0_B_grinder, P1_grinder_G)

# Place portafilter positions
P_G_PFdropoff = np_col(157.61, 0, -250.45)
R_G_PFdropoff = get_R_given_theta(np.deg2rad(-90), axis='y')

T_G_PFdropoff = get_T_given_P_and_R(P_G_PFdropoff, R_G_PFdropoff)

# Lever transforms
P_G_leverDepressed = np_col(-35.82, 83.80, -153.00)
R_G_leverDepressed = R_NO_ROT
T_G_leverDepressed = get_T_given_P_and_R(P_G_leverDepressed, R_G_leverDepressed)

LEVER_PULL_OFFSET = 50
P_G_leverPressed = np_col(-35.82 + LEVER_PULL_OFFSET, 83.80, -153.00)
R_G_leverPressed = R_NO_ROT
T_G_leverPressed = get_T_given_P_and_R(P_G_leverPressed, R_G_leverPressed)

# Button positions
dP_pushTool_depressGrinderBtn1 = np_col(-6,0,0)
dP_pushTool_depressGrinderBtn2 = np_col(-4,0,0)
dP_Gbtn_btnSpace = np_col(0,0,-20) # Horizontal spacing of buttons

P_G_btn1 = np_col(-64.42, 89.82, -227.68)
P_G_btn2 = np_col(-80.71, 94.26, -227.68)
R_G_btn1 = np.matmul(get_R_given_theta(np.deg2rad(-90), axis='y'), get_R_given_theta(np.deg2rad(80), axis='x')) #TODO: should be 72deg
T_G_btn1 = get_T_given_P_and_R(P_G_btn1, R_G_btn1)
T_B_Gbtn1 = mm(T_B_G, inv(T_G_btn1))

#_____________________________________________________________________________________
#                                                                                     #
#                                TRANSITION WAYPOINTS                                 #
#                                                                                     #
#_____________________________________________________________________________________#


#_____________________________________________________________________________________
#                                                                                     #
#                                     PROGRAM TASKS                                   #
#                                                                                     #
#_____________________________________________________________________________________#

#__________________|  TOOL STAND TASKS   |____________________#

def get_filter_tool(is_pickup=True):

    # Define specific transforms:
    J_tool_approach = [-157.180000, -79.150000, -79.150000, -109.790000, 90.070000, -164.890000]

    # Move to filter tool pick up position
    robot.MoveJ(J_tool_approach, blocking=True)
    # Pick up / drop off tool

    if (is_pickup):
        filter_tool_attach.RunCode(filter_tool_attach) # call subprogram
    else:
        filter_tool_detach.RunCode(filter_tool_detach) # call subfunction

    rdk.pause(2)  # to allow subprogram to complete
    robot.setPoseFrame(world_frame) # Reset frame to base frame (B)
    robot.setPoseTool(robot.PoseTool()) # Reset tool frame



def get_grinder_tool(is_pickup=True):

    # Define specific transforms:
    J_tool_approach = [-151.880896, -97.616411, -59.103383, -112.890980, 90.242082, -161.879346]

    # Move to filter tool pick up position
    robot.MoveJ(J_tool_approach, blocking=True)
    # Pick up / drop off tool

    if (is_pickup):
        grinder_tool_attach.RunCode(grinder_tool_attach) # call subprogram
    else:
        grinder_tool_detach.RunCode(grinder_tool_detach) # call subfunction

    rdk.pause(2)  # to allow subprogram to complete
    reset_robot_frame() # reset world frame back to robot frame



def get_cup_tool(is_pickup=True):

    # Define specific transforms:
    J_tool_approach = [-151.880896, -97.616411, -59.103383, -112.890980, 90.242082, -161.879346]

    # Move to filter tool pick up position
    robot.MoveJ(J_tool_approach, blocking=True)
    
    # Pick up / drop off tool
    if (is_pickup):
        cup_tool_attach.RunCode(cup_tool_attach) # call subprogram
    else:
        cup_tool_detach.RunCode(cup_tool_detach) # call subfunction

    rdk.pause(TOOL_ATTACH_SUBPROGRAM_TIME)  # to allow subprogram to complete
    reset_robot_frame() # reset world frame back to robot frame


def insert_portafilter_in_group_head_and_release():

    # Define waypoints
    robot.MoveJ([-138.87, -88.94, -142.67, -115.05, -34.46, -231.06], blocking=True)

    # Move up to insersion position
    T_B_M__groupHead = mm(T_B_S, T_S_groupHead, inv(T_TCP_filterToolCentre), inv(T_M_TCP))
    robot.MoveJ(get_T2RDK(T_B_M__groupHead), blocking=True)

    # Rotate 45 degrees
    T_B_M__groupHeadRotated = mm(T_B_S, T_S_groupHeadRotated, inv(T_TCP_filterToolCentre), inv(T_M_TCP))
    robot.MoveL(get_T2RDK(T_B_M__groupHeadRotated), blocking=True)

    rdk.pause(2)

    # Drop away from group head
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_B_S, np_col(0,0,-50)), blocking=True)

    # Move a short distance away
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_M_TCP, deltaR_C=get_R_given_theta(np.deg2rad(-100))), blocking=True)

    # Wait while TA detach tool
    rdk.pause(3) 
    # filter_tool_detach_silvia.RunProgram(filter_tool_detach_silvia)
    # rdk.pause(TOOL_ATTACH_SUBPROGRAM_TIME)  # to allow subprogram to complete
    # reset_robot_frame() # reset world frame back to robot frame


   

#__________________|  ESPRESSO MACHINE TASKS   |____________________#

def press_espresso_button():

    # Define task-specific transforms
    T_B_M__espressoBtn1 = mm(T_B_E, T_E_btn1, inv(T_TCP_grinderToolPush), inv(T_M_TCP))

    # Move to target position
    robot.MoveJ(get_T2RDK(T_B_M__espressoBtn1), blocking=True)
    
    # Depress button
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_B_E, dP_pushTool_depressBtn), blocking=True)
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_B_E, -dP_pushTool_depressBtn), blocking=True)
    rdk.pause(3)
    # Turn button off
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_B_E, dP_pushTool_Btn1MovDown), blocking=True)
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_B_E, dP_pushTool_depressBtn), blocking=True)
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_B_E, -dP_pushTool_depressBtn), blocking=True)
    rdk.pause(1)



def dropoff_cup_at_espresso():

    # Add waypoints
    
    # Midway point to prevent flip
    robot.MoveJ([-91.34, 242.27, -154.02, 91.75, 84.16, -40], blocking=True)
    
    robot.MoveJ([-119.659477, 242.272234, -154.024426, 91.752193, 84.158336, -40.000000], blocking=True)

    # Move to target position
    T_B_M__espressoCupDropoff = mm(T_B_E, T_E_cupDropoff, inv(T_TCP_cupTool), inv(T_M_TCP))
    robot.MoveJ(get_T2RDK(T_B_M__espressoCupDropoff), blocking=True)

    # Drop off cup
    #smooth_cup_release(T_B_cupEspressoDropoff)

    RDK.RunProgram(cup_tool_open, True)
    rdk.pause(CUP_TOOL_SUBPROGRAM_TIME)  # to allow subprogram to complete
    reset_robot_frame() # reset world frame back to robot frame

    # Move down slightly to remove weight from holder
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_E_cupPickupApproach, np_col(-10,0,0)), blocking=True)
    rdk.pause(2)
    # Clear cup area
    robot.MoveJ(transform_RDK_pose(robot.Pose(), T_E_cupPickupApproach, np_col(0,60,0)), blocking=True)
    #robot.MoveJ(transform_RDK_pose(robot.Pose(), T_BC, np_col(0,0,80)), blocking=True)
    rdk.pause(2)
    # Close cup again
    RDK.RunProgram(cup_tool_close, True)
    rdk.pause(CUP_TOOL_SUBPROGRAM_TIME)  # to allow subprogram to complete
    reset_robot_frame() # reset world frame back to robot frame
    


def pickup_cup_at_espresso():
    
    # Add waypoints
    robot.MoveJ([-119.659477, 242.272234, -154.024426, 91.752193, 84.158336, -40.000000], blocking=True)

    # Move to approach position
    T_B_M__espressoCupPickupApproach = mm(T_B_E, T_E_cupPickupApproach, inv(T_TCP_cupTool), inv(T_M_TCP))
    robot.MoveJ(get_T2RDK(T_B_M__espressoCupPickupApproach), blocking=True)
    
    # Smooth cup collect
    smooth_cup_collect(T_B_cupEspressoDropoff)

    


#_____________|  CUP TOOL MANIPULATION TASKS  |_________________#

# Function to release cup in a very smooth way - NOTE: DEPRICATED
# Assumes cup is in dropoff position with cup tool in resting pos
def smooth_cup_release(T_BC=T_B_cupEspressoDropoff):

    # Open cup tool
    RDK.RunProgram(cup_tool_open, True)
    rdk.pause(CUP_TOOL_SUBPROGRAM_TIME)  # to allow subprogram to complete
    reset_robot_frame() # reset world frame back to robot frame

    # Move down slightly
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_M_TCP, dP_E_cupReleaseDrop), blocking=True)
    # Clear cup area
    robot.MoveJ(transform_RDK_pose(robot.Pose(), T_M_TCP, np_col(0,80,0)), blocking=True)
    #robot.MoveJ(transform_RDK_pose(robot.Pose(), T_BC, np_col(0,0,80)), blocking=True)

    # Close cup again
    RDK.RunProgram(cup_tool_close, True)
    rdk.pause(CUP_TOOL_SUBPROGRAM_TIME)  # to allow subprogram to complete
    reset_robot_frame() # reset world frame back to robot frame



# Smooth cup collect for a given foreign frame
def smooth_cup_collect(T_BC=T_B_cupEspressoDropoff):

    # Open cup tool
    RDK.RunProgram(cup_tool_open, True)
    rdk.pause(CUP_TOOL_SUBPROGRAM_TIME)  # to allow subprogram to complete
    reset_robot_frame() # reset world frame back to robot frame

    # Move to coffee cup
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_BC, -dP_E_cupReleaseRetreat), blocking=True)
    # Raise slightly to pickup
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_BC, -dP_E_cupReleaseDrop), blocking=True)

    # Close cup tool
    RDK.RunProgram(cup_tool_close, True)
    rdk.pause(CUP_TOOL_SUBPROGRAM_TIME)  # to allow subprogram to complete
    reset_robot_frame() # reset world frame back to robot frame

    # Clear cup area
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_BC, dP_E_cupReleaseRetreat), blocking=True)



#__________________|  COFFEE STAND TASKS  |____________________#

def pickup_coffee_cup_from_stand():
    
    # Move to cup stand and rotate cup tool by 180deg 
    T_B_M__cupStackPickupApproach = mm(T_B_CS, T_CS_cupPickupApproach, inv(T_TCP_cupTool), inv(T_M_TCP))
    robot.MoveJ(get_T2RDK(T_B_M__cupStackPickupApproach), blocking=True)

    # Open cup tool
    RDK.RunProgram(cup_tool_open, True)
    rdk.pause(CUP_TOOL_SUBPROGRAM_TIME)  # to allow subprogram to complete
    reset_robot_frame() # reset world frame back to robot frame

    # Approach cup
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_B_CS, -dP_E_cupReleaseRetreat), blocking=True)
    
    # Close cup tool
    RDK.RunProgram(cup_tool_close, True)
    rdk.pause(CUP_TOOL_SUBPROGRAM_TIME)  # to allow subprogram to complete
    reset_robot_frame() # reset world frame back to robot frame
    
    # Clear cup tool
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_B_CS, dP_B_cupHeight), blocking=True)
    # Retreat from cup
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_B_CS, dP_E_cupReleaseRetreat), blocking=True)
    
    # Rotate cup as having it inverted messes up navigation
    robot.MoveJ(transform_RDK_pose(robot.Pose(), T_B_CS, deltaR_C=get_R_given_theta(np.deg2rad(-60), axis='z')), blocking=True)
    # TODO: Rotate 6th joint 180degrees to flip cup right way
    

def dropoff_coffee_cup_on_stand():
    
    # Move to cup stand
    T_B_M__cupStackDropoffApproach = mm(T_B_CS, T_CS_cupDropoffApproach, inv(T_TCP_cupTool), inv(T_M_TCP))
    robot.MoveJ(get_T2RDK(T_B_M__cupStackDropoffApproach), blocking=True)

    # Drop off cup
    #smooth_cup_release(T_B_CS)

    # Open cup tool
    RDK.RunProgram(cup_tool_open, True)
    rdk.pause(CUP_TOOL_SUBPROGRAM_TIME)  # to allow subprogram to complete
    reset_robot_frame() # reset world frame back to robot frame

    # Move down slightly to remove weight from holder
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_B_CS, np_col(0,0,-10)), blocking=True)
    rdk.pause(1)
    # Clear cup area
    robot.MoveJ(transform_RDK_pose(robot.Pose(), T_B_CS, np_col(50,0,0)), blocking=True)
    #robot.MoveJ(transform_RDK_pose(robot.Pose(), T_BC, np_col(0,0,80)), blocking=True)
    rdk.pause(1)
    # Close cup again
    RDK.RunProgram(cup_tool_close, True)
    rdk.pause(CUP_TOOL_SUBPROGRAM_TIME)  # to allow subprogram to complete
    reset_robot_frame() # reset world frame back to robot frame



#___________________|  GRINDER TASKS  |______________________#
def grinder_place_portafilter(is_pickup=False):
    # Waypoints
    robot.MoveJ([-16.85, -99.14, -142.51, -110.10, -64.26, 223.60], blocking=True)
    # Move to target position
    T_B_M__grinderPFdropoff = mm(T_B_G, T_G_PFdropoff, inv(T_TCP_filterToolEdge), inv(T_M_TCP))
    robot.MoveJ(get_T2RDK(T_B_M__grinderPFdropoff), blocking=True)

    robot.MoveJ(transform_RDK_pose(robot.Pose(), T_M_TCP, np_col(20,0,100)), blocking=True)

    # Detach filter tool
    if (is_pickup):
        filter_tool_attach_grinder.RunCode(filter_tool_attach_grinder) # call subprogram
    else:
        filter_tool_detach_grinder.RunCode(filter_tool_detach_grinder) # call subfunction
    
    rdk.pause(TOOL_ATTACH_SUBPROGRAM_TIME)
    reset_robot_frame()


def press_grinder_buttons():

    # Waypoint
    robot.MoveJ([-60.16, -147.65, -52.07, -160.28, 174.16, -220.00], blocking=True)

    # Move to target position
    T_B_M__grinderBtn1 = mm(T_B_G, T_G_btn1, inv(T_TCP_grinderToolPush), inv(T_M_TCP))
    robot.MoveJ(get_T2RDK(T_B_M__grinderBtn1), blocking=True)
    
    # Depress button
    rdk.pause(1)
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_B_Gbtn1, -dP_pushTool_depressGrinderBtn1), blocking=True)
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_B_Gbtn1, dP_pushTool_depressGrinderBtn1), blocking=True)
    
    # Wait
    rdk.pause(3)

    # Turn button off
    robot.MoveJ(transform_RDK_pose(robot.Pose(), T_B_Gbtn1, dP_Gbtn_btnSpace), blocking=True)
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_B_Gbtn1, -dP_pushTool_depressGrinderBtn2), blocking=True)
    robot.MoveL(transform_RDK_pose(robot.Pose(), T_B_Gbtn1, dP_pushTool_depressGrinderBtn2), blocking=True)


def pull_grinder_doser():
    # Waypoints
    robot.MoveJ([-47.128214, -116.453451, -96.410577, -147.135972, -92.799820, 230.000000], blocking=True)

    for _ in range(3):
        # Move to behind the lever
        T_B_M__grinderLeverDepressed = mm(T_B_G, T_G_leverDepressed, inv(T_TCP_grinderToolLever), inv(T_M_TCP))
        robot.MoveJ(get_T2RDK(T_B_M__grinderLeverDepressed), blocking=True)

        rdk.pause(1)

        # Pull the lever
        T_B_M__grinderLeverPressed = mm(T_B_G, T_G_leverPressed, inv(T_TCP_grinderToolLever), inv(T_M_TCP))
        robot.MoveJ(get_T2RDK(T_B_M__grinderLeverPressed), blocking=True)

        rdk.pause(1)



#_____________________________________________________________________________________
#                                                                                     #
#                           MAIN EXECUTABLE PROGRAM                                   #
#                                                                                     #
#_____________________________________________________________________________________#

def main():

    # Start at home
    robot.MoveJ(target, blocking=True)

    # A.1 Get filter tool
    get_filter_tool(is_pickup=True)

    # A.2 Move to grinder & place portafilter tool onto the grinder and detach tool
    grinder_place_portafilter(is_pickup=False)

    # B.1 Get grinder tool
    get_grinder_tool(is_pickup=True)

    # B.2 Move to grinder & turn on the grinder, wait 3 seconds, turn the grinder off
    press_grinder_buttons()

    # C.1 Pull the grider dosing lever 3 times


    # C.2 Return the grinder tool
    get_grinder_tool(is_pickup=False)


    # D.1 Pick up the portafilter
    grinder_place_portafilter(is_pickup=True)
    

    # D.2 Move to tamper and scrape coffee from the rim of the filter tool


    # E.1 Tamp the coffee


    # F.1 Get portafilter tool to the group head test point
    # F.2 Insert into coffee machine and rotat 45deg
    # F.3 Move portafilter tool a short distance and pause for 5s
    insert_portafilter_in_group_head_and_release()
    get_filter_tool(is_pickup=False)# REMOVE FOR ACTUAL THING


    # G.1 Get the coffee cup tool
    get_cup_tool(is_pickup=True)

    # G.2 Pick up coffee cup with cup tool
    pickup_coffee_cup_from_stand()

    # H.1 Place coffee cup in drip tray of the coffee machine under the group head
    dropoff_cup_at_espresso()

    # I.1 Put back the cup tool
    get_cup_tool(is_pickup=False)

    # I.2 Get the grinder tool
    get_grinder_tool(is_pickup=True)

    # I.3 Move to the espresso button 1 and turn the espresso on for 3 seconds
    press_espresso_button()
    
    # J.1 Put back the grinder tool
    get_grinder_tool(is_pickup=False)

    # J.2 Get the cup tool
    get_cup_tool(is_pickup=True)

    # J.3 Move to the coffee machine and pick up the cup of coffee
    pickup_cup_at_espresso()

    # J.4 Place steaming cup of coffee upright on the cup stand
    # J.5 Release coffee cup and put back the cup tool
    dropoff_coffee_cup_on_stand() 
    
    # J.6 Return Cup tool
    get_cup_tool(is_pickup=False)
    
    # J.7 Return Home
    robot.MoveJ(target, blocking=True)




def tool_test_routine():
    # Start at home
    robot.MoveJ(T_home, blocking=True)

    # A.1 Get filter tool
    get_filter_tool(is_pickup=True)
    get_filter_tool(is_pickup=False)

    get_grinder_tool(is_pickup=True)
    get_grinder_tool(is_pickup=False)

    get_cup_tool(is_pickup=True)
    get_cup_tool(is_pickup=False)


def test_routine():
    # Start at home
    robot.MoveJ(T_home, blocking=True)
    
    get_cup_tool(is_pickup=True)



if __name__ == '__main__':
    # Execute program
    # test_routine()
    # get_cup_tool(is_pickup=False)
    # get_filter_tool(is_pickup=False)
    # main()
    rdk.pause(1)
    robot.MoveJ(T_home, blocking=True)
    get_grinder_tool()
    pull_grinder_doser()


