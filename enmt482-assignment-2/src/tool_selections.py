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


# A more advanced example to get you moving with the RoboDK python API
# Note, as there are many solutions for a given pose, sometimes when
# running this, the robot may choose a weird pose that then doesn't allow
# the subsequent motion (due to being near a singularity etc). If this occurs, 
# just manually reset the robot startingposition to somewhere else and try again
# C Pretty, 18 Sept 2019
# version 2

import robolink as rl    # RoboDK API
import robodk as rdk     # Robot toolbox
import numpy as np


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

cup_tool_open = "Cup Tool Open" #RDK.Item("Cup Tool Open")
cup_tool_close = "Cup Tool Close"

# Define empty vectors
R_NO_ROT = np.identity(3)
P_NO_DISP = np.zeros((3,1))
#_____________________________________________________________________________________#
#
#                              DEFINE PROCESSING FUNCTIONS                            #
#
#_____________________________________________________________________________________#

# Returns rotation matrix for a single-axis rotation
def get_R_given_theta(theta, axis='z', isDeg=False):
   
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


# BUG: note, if we do T_from_points(P0_B_cupStand, PCollect_B_cupStand, PCollect_S_cupStand) 
# it will take us to the cup collection position not frame S

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

# Returns point P in frame B (P_BQ) when given:
#   P_AQ -> point Q in frame A
#   T_AB -> transformation matrix of frame B in frame A
def P_switch_frame(P_AQ, T_AB):
    # Returns P_BQ -> point Q in frame B
    return T_AB.dot(np.concatenate((P_AQ, np.array([[1.0]]) ), axis=0))[:3]

# Returns pose in frame B (Pose_BQ) when given:
#   Pose_A -> Pose in frame A
#   T_AB -> transformation matrix of frame B in frame A
def pose_switch_frame(Pose_A, T_AB):
    # Returns Pose_B -> pose in frame B
    return T_AB.dot(Pose_A)


# Returns a frame with the point P_AQ translated by xyz with an updated orientation uvw
def transform_point_to_pose(P_AQ, xyz=P_NO_DISP, uvw=P_NO_DISP):

    #1. get current robot pose in KUKA angles
    # pose_B is a 6x1 array
    pose_B = rdk.Pose_2_KUKA(robot.Pose())

    #2. update our pose & KUKA angles (if specified in params)
    if ((xyz == np.zeros((3,1)) ).all()):
        # If no xyz translation given 
        pose_B[:3] = (P_AQ + xyz).flatten()
    else:
        # If xyz translation given
        pose_B[:3] = xyz.flatten()

    #3. update orientation (if specified in params)
    if ((uvw == np.zeros((3,1)) ).all()):
        # If rotation given
        pose_B[3:] = (np.array([pose_B[3:]]).T + uvw ).flatten() # bae always said I made good one-liners 
    else:
        # if no rotation given
        pose_B[3:] = uvw.flatten()

    return rdk.KUKA_2_Pose(pose_B)




# Transforms point Q in a foreign frame (ie espresso frame) (P_QB) into a pose in local frame
# Used to find transform to move cup tool to ie. P_cup_collect_S at a specified orientation
def transform_foreign_point_to_local_pose(P_AQ, T_AB, xyz=np.zeros((3,1)), uvw=np.zeros((3,1))):

    # 1. convert foreign point to base frame
    P_BQ = P_switch_frame(P_AQ, T_AB)

    # 2. use tranformation 
    return transform_point_to_pose(P_BQ, xyz, uvw)

#___________________NUMPY UTILITY FUNCTIONS_______________#

# Returns RDK matrix of numpy T_AB matrix
def get_T2RDK(T_AB):
    return rdk.Mat(T_AB.tolist())

# Returns numpy T matrix given an RDK Mat matrix
def get_RDK2T(T_AB_RDK):

    temp = T_AB_RDK.rows
    T_np = np.array([[temp[0][0], temp[0][1], temp[0][2], temp[0][3]],
    [temp[1][0], temp[1][1], temp[1][2], temp[1][3]], [temp[2][0], temp[2][1], temp[2][2], temp[2][3]], [temp[3][0], temp[3][1], temp[3][2], temp[3][3]]])

    return T_np

# Simple function to return numpy columnvector
def np_col(*vals):
    return np.reshape(np.array(vals), (3,1))


# Simple wrapper function to perform matrix multiplication (m.m.) multiple times
def mm(*m):
    res = np.identity(4)
    for i in range(len(m)):
        res = np.matmul(res, m[i])
    return res

# simple wrapper function to invert a square np matrix
def inv(m):
    return np.linalg.inv(m)


# RDK Matrix facing function
# Transform a given RDK pose by a specified foreign translation (deltaP) in frame C and rotation (uvw)
#   T_B_C -> assumes transformations are given in base frame (B) (frame the pose is given in)
#           by passing in T_B_C, translations in foreign frame C (ie espresso frame) may be given
def transform_RDK_pose(pose, T_B_C=np.identity(4), deltaP_C=P_NO_DISP, uvw=P_NO_DISP):
    
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

    # 4. Determine pose of M2 in the base frame (B)
    T_B_M2 = mm(T_B_C, T_C_M2)

    # Return executable code
    return get_T2RDK(T_B_M2)

    


#__________________| ENTITY FRAME LOCATIONS FROM BASE FRAME (B)  |____________________#

# Directly use the RDK Matrix object from to hold pose (its an HT)
T_home = rdk.Mat([[     0.000000,     0.000000,     1.000000,   523.370000 ],
    [-1.000000,     0.000000,     0.000000,  -109.000000 ],
    [-0.000000,    -1.000000,     0.000000,   607.850000 ],
    [0.000000,     0.000000,     0.000000,     1.000000 ]])

# There is 50degree offset between TCP and master frames
M_TCP_theta = np.deg2rad(-50)
T_M_TCP = get_T_given_P_and_R(P_NO_DISP, get_R_given_theta(M_TCP_theta))


# Cup stand points
P_cup_stand_B = np.array([[-1.5],[-600.3],[-20]]) # cup stand frame origin in base frame coords
#T_cup_stand_B = rdk.Mat()



#__________________| TOOL STAND POINTS  |____________________#

P0_toolstand_B = np.array([[-555.6],[-78.5],[19.05]]) # Point 0 (origin) of tool stand
P1_toolstand_B = np.array([[-645.0],[77.2],[19.05]])
P1_toolstand_S = np.array([[-127],[127],[0]])

#np.array([[],[],[]])

#__________________| CUP STAND POINTS  |_____________________#

P0_B_cupStand = np.array([[-1.5],[-600.3],[-20]]) # cup stand frame origin in base frame coords
R_B_cupStandBase = get_R_given_theta(np.deg2rad(0))
PCollect_B_cupStand = np.array([[-1.5],[-600.3],[197]])
PCollect_S_cupStand = np.array([[0],[0],[217]])


J_cup_stand_approach = [-58.810794, -84.831211, -155.789759, -118.884848, 299.269576, - 39.400602]



#__________________| TOOL FRAME POINTS  |____________________#

P_TCP_cupToolCentre = np.array([[-47.0],[0],[186.11]])
R_TCP_cupToolCentre = R_NO_ROT

P_TCP_grinderToolPush = np.array([[0],[0],[102.82]])
R_TCP_grinderToolPush = R_NO_ROT


def home_to_cup_stand():
    
    T_B_cupStandBase = get_T_given_P_and_R(P0_B_cupStand, R_NO_ROT) # Yes this works!

    T_cupStandBase_cupCollect = get_T_given_P_and_R(PCollect_S_cupStand, R_NO_ROT) # works!

    R_cupCollect_cupTool = np.array([[0,1,0],[0,0,-1],[1,0,0]])
    T_cupCollect_cupTool = get_T_given_P_and_R(P_NO_DISP, R_cupCollect_cupTool)

    T_TCP_cupTool = get_T_given_P_and_R(P_TCP_cupToolCentre, R_TCP_cupToolCentre)
    
    T_B_M__cupCollect = mm(T_B_cupStandBase, T_cupCollect_cupTool, np.linalg.inv(T_TCP_cupTool), np.linalg.inv(T_M_TCP))
    
    T_cup_collect__RDK = rdk.Mat(T_B_M__cupCollect.tolist())

    robot.MoveJ(T_home, blocking=True)

    #robot.MoveL(T_cup_collect__RDK, blocking=True)    
    #robot.MoveJ(J_cup_stand_approach, blocking=True) 
    robot.MoveJ(T_cup_collect__RDK, blocking=True)

    RDK.RunProgram(cup_tool_open, True)
    rdk.pause(3)  # to allow subprogram to complete
    RDK.RunProgram(cup_tool_close, True)
    rdk.pause(3)  # to allow subprogram to complete

    # robot.MoveR(T_B_M__cupCollectLift, blocking=True)

    # Return home
    robot.MoveJ(target)




#__________________| ESPRESSO  POINTS  |_____________________#
P0_B_espresso = np.array([[-365.5],[-386.7],[349.8]]) # Point 0 (origin) of espresso machine
P1_B_espresso = np.array([[-577.8],[-441.6],[349.4]])
P1_espresso_E = np.array([[0],[218],[0]])

P_E_btn1 = np.array([[50.67],[35.25],[-27.89]])
P_E_btn2 = np.array([[50.67],[35.25],[-61.39]])
P_E_btn2 = np.array([[50.67],[35.25],[-94.89]])

P_E_btn1Pressed = np.array([[42.67],[35.25],[-27.89]])

def home_to_espresso():


    # Espresso frame in base frame
    T_B_E = get_T_to_foreign_frame(P1_B_espresso, P0_B_espresso, P1_espresso_E)
    
    # Button 1 frame in espresso frame
    T_E_btn1 = get_T_given_P_and_R(P_E_btn1, get_R_given_theta(np.deg2rad(-90), axis='y'))

    # (inverted side:) 
    # Note axis of push button is coming out of Z axis here
    T_TCP_grinderTool = get_T_given_P_and_R(P_TCP_grinderToolPush,  R_NO_ROT)

    # before
    #T_B_M__espressoBtn1 = np.matmul(np.matmul(np.matmul( T_E_btn1, np.linalg.inv(T_TCP_grinderTool)), np.linalg.inv(T_M_TCP)))
    # after
    T_B_M__espressoBtn1 = mm(T_B_E, T_E_btn1, inv(T_TCP_grinderTool), inv(T_M_TCP))
    
    T_espressoBtn1_RDK = get_T2RDK(T_B_M__espressoBtn1)

    #T_espressoBtn1Release_RDK = transform_RDK_pose(robot.Pose(), T_B_E, deltaP=np_col(8,0,0))

    #T_E_btn1Pressed = get_T_given_P_and_R(P_E_btn1Pressed, get_R_given_theta(np.deg2rad(-90), axis='y'))
    #T_B_M__espressoBtn1Pressed = T_B_E @ T_E_btn1Pressed @ np.linalg.inv(T_TCP_grinderTool) @ np.linalg.inv(T_M_TCP)
    #T_espressoBtn1Pressed_RDK = get_T2RDK(T_B_M__espressoBtn1Pressed)
    
    # ______JOINT ANGLE WAY POINTS & TRANSFORM DELTAS_______#
    dP_pushTool_depressBtn = np_col(0,0,8)
    
    deltP_pushTool_depressBtn = np_col(-8,0,0)


    # ______TASKS_______#
    #0. Move home
    robot.MoveJ(T_home, blocking=True)

    # 1. Get grinder tool
    #home_to_grinder_tool_pickup()

    # 2. Move to button 1
    robot.MoveJ(T_espressoBtn1_RDK, blocking=True)
    rdk.pause(2)
    #dT to depress button
    robot.MoveJ(transform_RDK_pose(robot.Pose(), T_B_E, deltP_pushTool_depressBtn), blocking=True)
    rdk.pause(1)
    robot.MoveJ(transform_RDK_pose(robot.Pose(), T_B_E, -deltP_pushTool_depressBtn), blocking=True)
    rdk.pause(1)
    robot.MoveJ(transform_RDK_pose(robot.Pose(), T_B_E, np_col(0,0,-40)), blocking=True)
    rdk.pause(1)
    robot.MoveJ(transform_RDK_pose(robot.Pose(), T_B_E, deltP_pushTool_depressBtn), blocking=True)


    return 
    



def home_to_grinder_tool_pickup(is_pickup=True):
    # Joint angles
    J_intermediatepoint = [-151.880896, -97.616411, -59.103383, -112.890980, 90.242082, -161.879346]

    # Convert a numpy array into a Mat (e.g.after calculation)
    T_grinderapproach_np = np.array([[     0.173648,    -0.984800,    -0.004000,  -502.103741],
        [ -0.984789,    -0.173618,    -0.006928,  -145.353888 ],
        [  0.006128,     0.005142,    -0.999968,   535.250260 ],
        [  0.000000,     0.000000,     0.000000,     1.000000 ]])

    T_grinderapproach = rdk.Mat(T_grinderapproach_np.tolist())


    robot.MoveJ(T_home, blocking=True)
    robot.MoveJ(J_intermediatepoint, blocking=True)
    robot.MoveL(T_grinderapproach, blocking=True)
    
    if (is_pickup):
        grinder_tool_attach.RunCode(grinder_tool_attach) # call subprogram
    else:
        grinder_tool_detach.RunCode(grinder_tool_detach) # call subfunction


    rdk.pause(3)  # to allow subprogram to complete
    
    # Note, the subfunctions change the reference frame, so you need to change it back
    # after calling them
    robot.setPoseFrame(world_frame)
    # you may also need to reset the toolframe
    robot.setPoseTool(robot.PoseTool())

    # and... move home to an existing target
    robot.MoveJ(target)



def home_to_filter_tool_pickup(is_pickup=True):
    # Joint angles
    #J_intermediatepoint = [-151.880896, -97.616411, -59.103383, -112.890980, 90.242082, -161.879346]

    # joint angles of final pos
    J_intermediatepoint = [-157.180000, -79.150000, -79.150000, -109.790000, 90.070000, -164.890000]

    # Convert a numpy array into a Mat (e.g.after calculation)
    T_filterapproach_np = np.array([[ 0.134329,    -0.990475,    -0.030247,  -394.847136],
     [0.990888,    -0.133955,    -0.014053,   -48.058050],
     [0.009867,     0.031859,    -0.999444,   523.956311],
     [0.000000,     0.000000,     0.000000,     1.000000 ]])


    T_filterapproach = rdk.Mat(T_filterapproach_np.tolist())


    robot.MoveJ(T_home, blocking=True)
    robot.MoveJ(J_intermediatepoint, blocking=True)
    robot.MoveL(T_filterapproach, blocking=True)

    if (is_pickup):
        filter_tool_attach.RunCode(filter_tool_attach) # call subprogram
    else:
        filter_tool_detach.RunCode(filter_tool_detach) # call subfunction

    rdk.pause(3)  # to allow subprogram to complete
    
    # Note, the subfunctions change the reference frame, so you need to change it back
    # after calling them
    robot.setPoseFrame(world_frame)
    # you may also need to reset the toolframe
    robot.setPoseTool(robot.PoseTool())

    # and... move home to an existing target
    robot.MoveJ(target)




def home_to_cup_tool_pickup(is_pickup=True):

    # joint angles of final pos
    J_intermediatepoint = [-182.540000, -79.150000, -79.150000, -109.790000, 90.070000, -164.890000]
    # Convert a numpy array into a Mat (e.g.after calculation)
    T_filterapproach_np = np.array([[-0.303018,    -0.952401,    -0.033351,  -377.380978],
     [-0.952934,     0.303178,     0.000257,   125.687846],
     [0.009867,     0.031859,    -0.999444,   523.956311],
     [0.000000,     0.000000,     0.000000,     1.000000 ]])

     
    T_filterapproach = rdk.Mat(T_filterapproach_np.tolist())


    robot.MoveJ(T_home, blocking=True)
    robot.MoveJ(J_intermediatepoint, blocking=True)
    robot.MoveL(T_filterapproach, blocking=True)

    if (is_pickup):
        cup_tool_attach.RunCode(cup_tool_attach) # call subprogram
    else:
        cup_tool_detach.RunCode(cup_tool_detach) # call subfunction

    rdk.pause(3)  # to allow subprogram to complete

    # Note, the subfunctions change the reference frame, so you need to change it back
    # after calling them
    robot.setPoseFrame(world_frame)

    # you may also need to reset the toolframe
    robot.setPoseTool(robot.PoseTool())

    # and... move home to an existing target
    robot.MoveJ(target)

    # BUG: This spawns another cup tool oops
    RDK.RunProgram(cup_tool_open, True)
    rdk.pause(3)  # to allow subprogram to complete
    RDK.RunProgram(cup_tool_close, True)
    rdk.pause(3)  # to allow subprogram to complete


def prog_steps():

    # Start at home

    # A.1 Get filter tool

    # A.2 Move to grinder

    # A.3 Place portafilter tool onto the grinder and detach tool

    # B.1 Get grinder tool

    # B.2 Move to grinder

    # B.3 Turn on the grinder, wait 3 seconds, turn the grinder off

    # C.1 Pull the grider dosing lever 3 times

    # D.1 Scrape coffee from the rim of the filter tool

    # E.1 Tamp the coffee

    # F.1 Move portafilter tool to the group head test point

    # F.2 Insert into coffee machine and rotat 45deg

    # F.3 Move portafilter tool a short distance and pause for 5s

    # G.1 Get the coffee cup tool

    # G.2 Pick up coffee cup with cup tool

    # H.1 Place coffee cup in drip tray of the coffee machine under the group head

    # I.1 Put back the cup tool

    # I.2 Get the grinder tool

    # I.3 Move to the espresso button 1 and turn the espresso on for 3 seconds

    # J.1 Put back the grinder tool

    # J.2 Get the cup tool

    # J.3 Move to the coffee machine and pick up the cup of coffee

    # J.4 Place steaming cup of coffee upright on the cup stand

    # J.5 Release coffee cup and put back the cup tool

    # J.6 Return Home

    pass



def main():

    #home_to_cup_stand()
    home_to_espresso()
    return

    home_to_cup_tool_pickup(is_pickup=True)
    home_to_cup_tool_pickup(is_pickup=False)

    home_to_grinder_tool_pickup(is_pickup=True)
    home_to_grinder_tool_pickup(is_pickup=False)

    home_to_filter_tool_pickup(is_pickup=True)
    home_to_filter_tool_pickup(is_pickup=False)

    

        

if __name__ == '__main__':
    # Execute program
    main()

