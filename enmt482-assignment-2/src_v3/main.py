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

from robolink.robolink import Robolink  # Robolink API
import robodk.robodk as rdk  # Robot toolbox
import numpy as np
import sys
import time


# _____________________________________________________________________________________
#                                                                                     #
#                          NUMPY WRAPPER & UTILITY FUNCTIONS                          #
#                                                                                     #
# ____________________________________________________________________________________#

# These functions help to keep the code tidy and readable
def get_T2RDK(T_AB):
    """Returns RDK matrix of numpy T_AB matrix"""
    return rdk.Mat(T_AB.tolist())


def get_RDK2T(T_AB_RDK):
    """Returns numpy T matrix given an RDK Mat matrix"""

    temp = T_AB_RDK.rows
    T_np = np.array([[temp[0][0], temp[0][1], temp[0][2], temp[0][3]],
                     [temp[1][0], temp[1][1], temp[1][2], temp[1][3]],
                     [temp[2][0], temp[2][1], temp[2][2], temp[2][3]],
                     [temp[3][0], temp[3][1], temp[3][2], temp[3][3]]])

    return T_np


def np_col(*vals):
    """Simple function to return numpy columnvector"""
    return np.reshape(np.array(vals), (3, 1))


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

def pythag(a, b):
    """Simple a^2 + b^2 = c^2 solver"""
    return np.sqrt(a**2 + b**2)


# _____________________________________________________________________________________
#                                                                                     #
#                      TRANSFORM MATRIX PROCESSING FUNCTIONS                          #
#                                                                                     #
# ____________________________________________________________________________________#
R_NO_ROT = np.identity(3)
P_NO_DISP = np.zeros((3, 1))


def get_R_given_theta(theta, axis, radians=False):
    """Returns the rotation matrix for a single-axis rotation"""

    # convert theta to radians
    if not radians:
        theta = np.deg2rad(theta)
    # generate matrix for z rotation
    if axis == 'z':
        R_AB = np.array([[np.cos(theta), -np.sin(theta), 0],
                         [np.sin(theta), np.cos(theta), 0],
                         [0, 0, 1]])
    elif axis == 'y':
        R_AB = np.array([[np.cos(theta), 0, np.sin(theta)],
                         [0, 1, 0],
                         [-np.sin(theta), 0, np.cos(theta)]])

    elif axis == 'x':
        R_AB = np.array([[1, 0, 0],
                         [0, np.cos(theta), -np.sin(theta)],
                         [0, np.sin(theta), np.cos(theta)]])
    # else:
    #     raise Exception(f'Unknown axis: {axis}')

    # Return rotation matrix rounded to 10 decimal places
    # this prevents massive negative numbers from degree conversions
    return np.around(R_AB, 10)


def get_T_given_P_and_R(P_BA, R_AB):
    """Constructs T transform given displacement and rotation matrix, accepts 'None' as a matrix represensting no translation of rotation"""
    if P_BA is None:
        P_BA = np_col(0, 0, 0)
    if R_AB is None:
        R_AB = np.eye(3)

    T_AB = np.concatenate((R_AB, P_BA), axis=1)
    T_AB = np.concatenate((T_AB, np.array([[0, 0, 0, 1]])), axis=0)
    return T_AB


def get_T_to_foreign_frame(P_AQ, P_AB, P_QB):
    """
    Returns a transform function of point Q in frame B (typically base frame) given three points:
    P_AQ -> point Q in frame A (ie second point on tool stand)
    P_AB -> frame B origin in frame A
    P+QB -> point Q in frame B
    """
    # It can be seen (in docs) that frame rotations only ever occur about the Z axis
    # Therefore we can determine the row vector
    # 1. determine angle between x-axes of frame A and B (rotation about Z)
    P_BQ = P_AQ - P_AB
    # theta is angle about x-axis of frame A to frame B (first element)
    theta = (np.arctan2(P_BQ[1], P_BQ[0]) - np.arctan2(P_QB[1], P_QB[0]))[0]

    # 2. Determine rotation matrix between frames (Z-axis rotation)
    R_AB = get_R_given_theta(theta, axis='z', radians=True)

    # 3. Create frame transform matrix
    T_BQ = np.concatenate((R_AB, P_AB), axis=1)
    T_BQ = np.concatenate((T_BQ, np.array([[0, 0, 0, 1]])), axis=0)

    return T_BQ


def get_R_from_x_and_y_directions(x, y):
    """Gets a rotation matrix from x and y direction vectors. The vectors don't have to be unit length"""
    # Normalise and reshape x and y
    x = (x / np.linalg.norm(x)).reshape(3,)
    y = (y / np.linalg.norm(y)).reshape(3,)

    # z = cross product of x and y
    z = np.cross(x, y)

    return np.array([x, y, z]).T  # transpose to convert to coloumn matrix with x, y, z vectors as coloumns


# _____________________________________________________________________________________
#                                                                                     #
#                           ROBODK POSE MANIPULATION FUNCTIONS                        #
#                                                                                     #
# ____________________________________________________________________________________#

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
    T_C_M2[:3, 3:] += deltaP_C  # Add deltaP displacement to the P column
    T_C_M2[:3, :3] = np.matmul(deltaR_C, T_C_M2[:3, :3])

    # 4. Determine pose of M2 in the base frame (B)
    T_B_M2 = mm(T_B_C, T_C_M2)

    # Return executable code
    return get_T2RDK(T_B_M2)


# _____________________________________________________________________________________
#                                                                                     #
#                              Debugging functions                                    #
#                                                                                     #
# ____________________________________________________________________________________#
# class COLOURS:
#     HEADER = '\033[95m'
#     OKBLUE = '\033[94m'
#     OKCYAN = '\033[96m'
#     OKGREEN = '\033[92m'
#     WARNING = '\033[93m'
#     FAIL = '\033[91m'
#     ENDC = '\033[0m'
#     BOLD = '\033[1m'
#     UNDERLINE = '\033[4m'


# depth = 0
# def debug_enable():
#     def trace(frame, event, arg):
#         # Inspired by https://stackoverflow.com/a/5104943 https://stackoverflow.com/a/32607930
#         global depth
#         filename = frame.f_code.co_filename
#         try:
#             if __file__ == filename:
#                 function = frame.f_code.co_name
#                 if event == "call" and function not in ['inv', 'get_T2RDK', 'mm', 'get_RDK2T']:
#                     print(f'{"|--" * depth}| {COLOURS.OKBLUE}{function}(){COLOURS.ENDC}')
#                     depth += 1
#                 elif event == 'return':
#                     if depth > 0:
#                         depth -= 1

#         except:
#             pass
#         return trace
        
#     sys.settrace(trace)


# def debug_disable():
#     sys.settrace(None)



#_______________________|   FRAME KEYS   |_________________________#

#   B -> baseframe
#   M -> master tool point frame
#   TCP -> tool centre point frame

#   E -> espresso machine frame
#   S -> toolstand frame
#   G -> grinder frame
#   T -> tamper frame

#   cupTool -> cup tool frame
#   grinderTool -> grinder tool


#__________________|   BASE FRAME TRANSFORMS   |___________________#

# Directly use the RDK Matrix object from to hold pose (its an HT)
T_home = rdk.Mat([[     0.000000,     0.000000,     1.000000,   523.370000 ],
    [-1.000000,     0.000000,     0.000000,  -109.000000 ],
    [-0.000000,    -1.000000,     0.000000,   607.850000 ],
    [0.000000,     0.000000,     0.000000,     1.000000 ]])


#_____________________________________________________________________________________
#                                                                                     #
#                              INITIALISE ROBODK API CONFIG                           #
#                                                                                     #
#_____________________________________________________________________________________#

# Initialise robolink
RDK = Robolink()

# Initialise robot & starting parameters
robot = RDK.Item('UR5')
home = RDK.Item('Home')   # existing target in station

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



def goto_global_home():
    robot.MoveJ(home, blocking=True)


def run_program_cup_tool_open():

    RDK.RunProgram("Cup Tool Open", True)
    #better_wait_finished()  # to allow subprogram to complete


def run_program_cup_tool_close():
    RDK.RunProgram("Cup Tool Close", True)
    #better_wait_finished()  # to allow subprogram to complete
                             # Note, pausing after this routine causes RoboDK to fail
                             # when running on the actual robot


def better_wait_finished():
    """
    Better version of robot.WaitFinished(). Waits for the robot to finish it's program (and stay finished after checking again after 1 sec\n
    I believe the "Pause" instructions in the tool subprograms cause the robot to say it isn't busy when really it is in the middle of a subprogram. This causes the original robot.WaitFinished() to return ealier than expected.\n
    The solution was to call WaitFinished() and then recheck the value after 1 second (scaled by simulation speed)
    """
    while True:
        robot.WaitFinished()
        sim_scale_capped_at_1 = min(1, RDK.SimulationSpeed())  # cap simulation scale so we never wait shorter times (for safety)
        wait_time = 1.1  # Wait 1.1s (just a bit more than 2x500ms)
        wait_time / sim_scale_capped_at_1 if sim_scale_capped_at_1 > 0 else 1  # scaled by simulation speed if the sim is running slower than realtime
        
        time.sleep(wait_time) 

        if not robot.Busy():
            # The below reset is likely not needed, as routines reset the world frame, but it is included for safety
            robot.setPoseFrame(world_frame) # Reset frame to base frame (B)
            robot.setPoseTool(robot.PoseTool()) # Reset tool frame
            return



# _____________________________________________________________________________________
#                                                                                     #
#                              Modularised Classes                                    #
#                                                                                     #
# ____________________________________________________________________________________#
class Tools:
    """Class for access Tool transforms and getting tools"""

    # There is 50degree offset between TCP and master frames
    M_TCP_theta = -50 # degrees 
    T_M_TCP = get_T_given_P_and_R(None, get_R_given_theta(M_TCP_theta, 'z'))

    # Grinder tool transformations
    P_TCP_grinderToolPush = np.array([[0],[0],[102.82]])
    R_TCP_grinderToolPush = R_NO_ROT
    T_TCP_grinderToolPush = get_T_given_P_and_R(P_TCP_grinderToolPush, R_TCP_grinderToolPush)

    P_TCP_grinderToolLever = np_col(-50, 0, 67.06)
    R_TCP_grinderToolLever = mm(get_R_given_theta(90, 'y'), get_R_given_theta(90, 'x'))
    T_TCP_grinderToolLever = get_T_given_P_and_R(P_TCP_grinderToolLever, R_TCP_grinderToolLever)

    # Filter tool transformations
    P_TCP_filterToolEdge = np.array([[-32.0],[0],[27.56]])
    R_TCP_filterToolEdge = get_R_given_theta(-7.5, 'y')
    T_TCP_filterToolEdge = get_T_given_P_and_R(P_TCP_filterToolEdge, R_TCP_filterToolEdge)

    P_TCP_filterToolCentre = np.array([[4.71],[0],[144.76]])
    R_TCP_filterToolCentre = get_R_given_theta(-7.5, 'y')
    T_TCP_filterToolCentre = get_T_given_P_and_R(P_TCP_filterToolCentre, R_TCP_filterToolCentre)

    dP_TCP_filterToolTop = np_col(22.0, 0, 0)  # relative position from P_TCP_filterToolCentre to top in 7.5 deg rotated frame
    dT_TCP_filterToolTop = get_T_given_P_and_R(dP_TCP_filterToolTop, None)  # relative transform from T_TCP_filterToolCentre to top
    T_TCP_filterToolTop = mm(T_TCP_filterToolCentre, dT_TCP_filterToolTop)

    # Cup tool transformations
    P_TCP_cupToolCentre = np.array([[-47.0],[0],[186.11]])
    R_TCP_cupToolCentre = R_NO_ROT
    T_TCP_cupTool = get_T_given_P_and_R(P_TCP_cupToolCentre, R_TCP_cupToolCentre)

    J_tool_approach = [-156.784696, -81.045663, -75.493125, -113.658042, 89.509670, -183.065198]

    # change in posisitions for smoother cup grabs and releases
    # Height of cup (add this to points on espresso @ cup stand)
    dP_B_cupHeight = np_col(0, 0, 80)
    # Small change in height to move to ensure weight of cup isn't on tool
    dP_E_cupReleaseDrop = np_col(0, 0, -5)
    # clear cup range for linear move so we don't knock over cup
    dP_E_cupReleaseRetreat = np_col(0, 80, 0) # Note, this is a lateral move

    @staticmethod
    def get_filter_tool(is_pickup=True):
        # Move to filter tool pick up position
        robot.MoveJ(Tools.J_tool_approach, blocking=True)

        # Pick up / drop off tool
        if is_pickup:
            filter_tool_attach.RunCode(filter_tool_attach) # call subprogram
        else:
            filter_tool_detach.RunCode(filter_tool_detach) # call subprogram

        better_wait_finished()  # to allow subprogram to complete

    @staticmethod
    def get_grinder_tool(is_pickup=True):
        # Move to filter tool pick up position
        robot.MoveJ(Tools.J_tool_approach, blocking=True)
        # Pick up / drop off tool

        if is_pickup:
            grinder_tool_attach.RunCode(grinder_tool_attach) # call subprogram
        else:
            grinder_tool_detach.RunCode(grinder_tool_detach) # call subprogram

        better_wait_finished()  # to allow subprogram to complete

    @staticmethod
    def get_cup_tool(is_pickup=True):
        
        # Move to filter tool pick up position
        robot.MoveJ(Tools.J_tool_approach, blocking=True)

        # Pick up / drop off tool
        if is_pickup:
            cup_tool_attach.RunCode(cup_tool_attach) # call subprogram
        else:
            cup_tool_detach.RunCode(cup_tool_detach) # call subprogram

        better_wait_finished()  # to allow subprogram to complete


    @staticmethod
    def smooth_cup_collect(T_BC):
        """Smooth cup collect for a given foreign frame"""
        # Open cup tool
        run_program_cup_tool_open()

        # Move to coffee cup
        robot.MoveL(transform_RDK_pose(robot.Pose(), T_BC, -Tools.dP_E_cupReleaseRetreat), blocking=True)
        # Raise slightly to pickup
        robot.MoveL(transform_RDK_pose(robot.Pose(), T_BC, -Tools.dP_E_cupReleaseDrop), blocking=True)

        # Close cup tool
        run_program_cup_tool_close()

        # Clear cup area
        robot.MoveL(transform_RDK_pose(robot.Pose(), T_BC, Tools.dP_E_cupReleaseRetreat), blocking=True)


class Grinder:
    """Class for accessing Grinder related transforms and tasks"""
    # Grinder frame finding points
    P0_B_grinder = np_col(482.7, -432.1, 316.1)
    P1_B_grinder = np_col(370.5, -322.5, 65.9)
    P1_grinder_G = np_col(157.61, 0, -250.45)

    T_B_G = get_T_to_foreign_frame(P1_B_grinder, P0_B_grinder, P1_grinder_G)

    # Place portafilter positions
    P_G_PFdropoff = np_col(157.61 + 5, 0, -250.45)  # +5 fudge factor added by @Sam after watching Sim
    R_G_PFdropoff = get_R_given_theta(-90, 'y')
    T_G_PFdropoff = get_T_given_P_and_R(P_G_PFdropoff, R_G_PFdropoff)

    # Lever approach transforms
    P_G_leverDepressed = np_col(-35.82, 83.80, -153.00+25)
    R_G_leverDepressed = R_NO_ROT
    T_G_leverDepressed = get_T_given_P_and_R(P_G_leverDepressed, R_G_leverDepressed)

    # Lever circular move transform params
    dP_leverMidway = np_col(60, -15 - 10, 0)
    dR_leverMidway = get_R_given_theta(-10, axis='z')

    dP_leverDepressed = np_col(60 - 5 - 6, -55 - 15 + 6, 0) # add +ve to the -55 if we lose the lever
    dR_leverDepressed = get_R_given_theta(-30, axis='z')
    
    
    # Button positions
    dP_pushTool_btnClearance = 10  # amount to clear each button by
    T_pushTool_btnClearance = get_T_given_P_and_R(np_col(0, 0, dP_pushTool_btnClearance), None)  # transform matrix for clearing buttons
    dP_pushTool_depressGrinderBtn1 = np_col(-6 - dP_pushTool_btnClearance, 0, 0)  # amount to move to depress button1 (must include clearance)
    dP_pushTool_depressGrinderBtn2 = np_col(-4 - dP_pushTool_btnClearance, 0, 0)  # amount to move to depress button2 (must include clearance)
    dP_Gbtn_btnSpace = np_col(0, 0, -20) # Horizontal spacing of buttons

    P_G_btn1 = np_col(-64.42, 89.82, -227.68)
    P_G_btn2 = np_col(-80.71, 94.26, -227.68)
    R_G_btn1 = mm(get_R_given_theta(-90, 'y'), get_R_given_theta(80, 'x'))
    T_G_btn1 = get_T_given_P_and_R(P_G_btn1, R_G_btn1)
    T_B_Gbtn1 = mm(T_B_G, inv(T_G_btn1))

    @staticmethod
    def goto_local_home(rotate_wrist_360=False):
        """Localised "home" position that is a safe distance away from the grinder.
        rotate_wrist_360 is used after picking the portfilter up"""
        waypoint = [-4.303339, -63.101234, -142.488178, -145.305617, -49.420037, 134.374286]
        if rotate_wrist_360:
            waypoint[5] = waypoint[5] -360
        robot.MoveJ(waypoint, blocking=True)

    @staticmethod
    def routine():
        """Routine which covers steps A-C including picking portafilter up from grinder. Starts at global home and finishes at grinder local home (with wrist rotated -360 deg)"""
        # Waypoints to avoid expresso machine while getting tools
        waypoint_to_toolstand_1 = [-67.440000, -77.380000, -84.030000, -128.480000, 13.750000, -28.010000]
        waypoint_to_toolstand_2 = [-106.860000, -77.380000, -84.030000, -128.480000, 13.750000, -28.010000]

        # # A.1 Get filter tool
        robot.MoveJ(waypoint_to_toolstand_1, blocking=True)
        robot.MoveJ(waypoint_to_toolstand_2, blocking=True)
        Tools.get_filter_tool(is_pickup=True)
        robot.MoveJ(waypoint_to_toolstand_2, blocking=True)
        robot.MoveJ(waypoint_to_toolstand_1, blocking=True)
        Grinder.goto_local_home()

        # # A.2 Move to grinder & place portafilter tool onto the grinder and detach tool
        Grinder.place_portafilter(is_pickup=False)

        # # B.1 Get grinder tool
        robot.MoveJ(waypoint_to_toolstand_1, blocking=True)
        robot.MoveJ(waypoint_to_toolstand_2, blocking=True)
        Tools.get_grinder_tool(is_pickup=True)
        robot.MoveJ(waypoint_to_toolstand_2, blocking=True)
        robot.MoveJ(waypoint_to_toolstand_1, blocking=True)

        # # B.2 Move to grinder & turn on the grinder, wait 3 seconds, turn the grinder off
        Grinder.press_buttons()

        # C.1 Pull the grider dosing lever 3 times
        Grinder.pull_doser()
        

        # C.2 Return the grinder tool
        robot.MoveJ(waypoint_to_toolstand_1, blocking=True)
        robot.MoveJ(waypoint_to_toolstand_2, blocking=True)
        Tools.get_grinder_tool(is_pickup=False)
        robot.MoveJ(waypoint_to_toolstand_2, blocking=True)
        robot.MoveJ(waypoint_to_toolstand_1, blocking=True)

        # C.3 Pick up the portafilter
        Grinder.place_portafilter(is_pickup=True)

        # Finishes at local home (with rotate_wrist_360=True)

    @staticmethod
    def place_portafilter(is_pickup=False):
        # Waypoints
        waypoint_1 = [-13.287812, -96.568288, -146.814161, -108.497109, -58.303928, 136.038647]
        waypoint_1_360_deg_wrist = waypoint_1[0:5] + [waypoint_1[5] - 360] # same as waypoint_1 but -360 deg wrist rotation for pickup of portafilter

        # Detach filter tool
        if is_pickup:
            # Waypoint
            robot.MoveJ(waypoint_1_360_deg_wrist, blocking=True)

            # BUG @sam may need a 1-1.5mm move to the right
            # Move to the pickup point (extracted from routine)
            waypoint_grinderRestApproachPickup = [-17.172086, -99.768847, -141.220021, -111.198773, -62.150759, -223.341274]
            robot.MoveJ(waypoint_grinderRestApproachPickup, blocking=True)

            # Call subprogram 
            filter_tool_attach_grinder.RunCode(filter_tool_attach_grinder) # call subprogram
            better_wait_finished()  # wait for subprogram to finish

            # Move away
            robot.MoveJ(waypoint_1_360_deg_wrist, blocking=True)
            Grinder.goto_local_home(rotate_wrist_360=True)
            
        else:
            # Waypoint
            robot.MoveJ(waypoint_1, blocking=True)
            
            # Move to the dropoff point
            T_B_M__grinderPFdropoff = mm(Grinder.T_B_G, Grinder.T_G_PFdropoff, inv(Tools.T_TCP_filterToolEdge), inv(Tools.T_M_TCP))
            robot.MoveJ(get_T2RDK(T_B_M__grinderPFdropoff), blocking=True)

            # Call subprogram
            filter_tool_detach_grinder.RunCode(filter_tool_detach_grinder) # call subfunction
            better_wait_finished()  # wait for subprogram to finish
            
            # Move away (routine moves a little away automatically)
            Grinder.goto_local_home()

    @staticmethod
    def press_buttons():
        # Waypoints to avoid grinder
        waypoint_1 = [-66.290000, -101.830000, -113.700000, -144.460000, -156.660000, 140.000000]
        waypoint_2 = [-63.160624, -149.181309, -49.372189, -161.446502, -188.832230, 140.000000]
        robot.MoveJ(waypoint_1, blocking=True)
        robot.MoveJ(waypoint_2, blocking=True)

        # Move to target position
        T_B_M__grinderBt1Cleared = get_T2RDK(mm(Grinder.T_B_G, Grinder.T_G_btn1, inv(Grinder.T_pushTool_btnClearance), inv(Tools.T_TCP_grinderToolPush), inv(Tools.T_M_TCP)))
        robot.MoveJ(T_B_M__grinderBt1Cleared, blocking=True)

        # Depress button
        robot.MoveL(transform_RDK_pose(robot.Pose(), Grinder.T_B_Gbtn1, -Grinder.dP_pushTool_depressGrinderBtn1), blocking=True)
        robot.MoveL(transform_RDK_pose(robot.Pose(), Grinder.T_B_Gbtn1, Grinder.dP_pushTool_depressGrinderBtn1), blocking=True)

        # Wait
        rdk.pause(3)

        # Turn button off
        robot.MoveJ(transform_RDK_pose(robot.Pose(), Grinder.T_B_Gbtn1, Grinder.dP_Gbtn_btnSpace), blocking=True)
        robot.MoveL(transform_RDK_pose(robot.Pose(), Grinder.T_B_Gbtn1, -Grinder.dP_pushTool_depressGrinderBtn2), blocking=True)
        robot.MoveL(transform_RDK_pose(robot.Pose(), Grinder.T_B_Gbtn1, Grinder.dP_pushTool_depressGrinderBtn2), blocking=True)

        # Waypoints to avoid grinder (reverse)
        robot.MoveJ(waypoint_2, blocking=True)
        robot.MoveJ(waypoint_1, blocking=True)

        # Finish at local home
        Grinder.goto_local_home()

    @staticmethod
    def pull_doser():
        # Waypoints to avoid grinder
        waypoint_1 = [-48.013939, -102.406795, -116.698620, -140.894585, -93.685545, 230.000000]
        waypoint_2 = [-47.128214, -116.453451, -96.410577, -147.135972, -92.799820, 230.000000]
        robot.MoveJ(waypoint_1, blocking=True)
        robot.MoveJ(waypoint_2, blocking=True)

        T_B_M__grinderLeverApproach= mm(Grinder.T_B_G, Grinder.T_G_leverDepressed, inv(Tools.T_TCP_grinderToolLever), inv(Tools.T_M_TCP))
        
        # Move to behind the lever
        robot.MoveJ(get_T2RDK(T_B_M__grinderLeverApproach), blocking=True)

        # moveC solution
        lever_start_pos = robot.Pose()
        lever_midpoint_pos = transform_RDK_pose(robot.Pose(), Grinder.T_B_G, Grinder.dP_leverMidway, deltaR_C=Grinder.dR_leverMidway)
        lever_end_pos = transform_RDK_pose(lever_midpoint_pos, Grinder.T_B_G, Grinder.dP_leverDepressed, deltaR_C=Grinder.dR_leverDepressed)
        
         
        for _ in range(3):
            
            # Pull the lever
            robot.MoveC(lever_midpoint_pos, lever_end_pos, blocking=True)
            
            rdk.pause(1)

            # Return the lever
            robot.MoveC(lever_midpoint_pos, lever_start_pos, blocking=True)

            rdk.pause(1)

        # Waypoints to avoid grinder
        robot.MoveJ(waypoint_2, blocking=True)
        robot.MoveJ(waypoint_1, blocking=True)
        
        # Finish at local home
        Grinder.goto_local_home()


class Tamper:
    """Class for accessing Tamper related transforms and tasks"""
    # Tamper frame finding points
    P0_B_tamperStand = np_col(599.9, 53.0, 254.4)   # origin
    P1_B_tamperStand = np_col(582.7, 128.6, 235.8)  # negative x axis
    P2_B_tamperStand = np_col(677.9, 69.9, 249.8)   # positive y axis

    V_B_tamperStand_x_direction = P0_B_tamperStand - P1_B_tamperStand
    V_B_tamperStand_y_direction = P2_B_tamperStand - P0_B_tamperStand

    R_T_tamperStand = get_R_from_x_and_y_directions(V_B_tamperStand_x_direction, V_B_tamperStand_y_direction)
    T_B_T = get_T_given_P_and_R(P0_B_tamperStand, R_T_tamperStand)

    # Scraper position
    P_SCRAPER_Z_OFFSET = 9
    P_T_scraperBottomRef = np_col(70.0, 0, -32.0 + P_SCRAPER_Z_OFFSET)  
    R_T_scraperBottomRef = mm(get_R_given_theta(-90, 'z'), get_R_given_theta(-90, 'y'))
    T_T_scraperBottomRef = get_T_given_P_and_R(P_T_scraperBottomRef, R_T_scraperBottomRef)  # postition of the bottom middle of the scraper

    scrapeDistanceHalf = 60  # half the distance which scraper is pulled through (plus and minus the neutral position)
    scrapeClearance = 20  # amount to clear the bottom of the scraper by when not scraping
    dT_scraperDifferenceWaypoint1 = get_T_given_P_and_R(np_col(0, scrapeDistanceHalf, -scrapeClearance), None)  # back bottom
    dT_scraperDifferenceWaypoint2 = get_T_given_P_and_R(np_col(0, scrapeDistanceHalf, 0), None)  # back top
    dT_scraperDifferenceWaypoint3 = get_T_given_P_and_R(np_col(0, -scrapeDistanceHalf, 0), None)  # front top
    dT_scraperDifferenceWaypoint4 = get_T_given_P_and_R(np_col(0, -scrapeDistanceHalf, -scrapeClearance), None)  # front bottom

    # Tamper position
    P_TAMPER_X_OFFSET = 3
    P_T_tamper = np_col(-80.0 + P_TAMPER_X_OFFSET, 0, -55.0)
    R_T_tamper = mm(get_R_given_theta(-90, 'z'), get_R_given_theta(-90, 'y'))
    T_T_tamper = get_T_given_P_and_R(P_T_tamper, R_T_tamper)
    
    # TODO increase to positive once the tamper alignment is checked, making it more positive will make it push the portafilter up more towards the tamper
    tamperInside = 10.5 #BUG  # how much to push the tamper up when tamping  (ie how far the tamper goes inside the portafilter)
    T_T_tamperInside = mm(get_T_given_P_and_R(np_col(0, 0, tamperInside), None), T_T_tamper)

    tamperClearance = 20  # how much to clear the tamper by in the z direction when not tamping
    T_T_tamperCleared = mm(get_T_given_P_and_R(np_col(0, 0, -tamperClearance), None), T_T_tamper)

    def goto_local_home():
        robot.MoveJ([20.276476, -71.494847, -153.615137, -127.896757, -80.798714, -220.797674], blocking=True)

    def routine():
        """Routine which covers steps D-E. Starts and finishes at Tamper local home"""      
        # D.1 Scrape the portafilter on the tamper
        Tamper.scrape_portafilter()

        # E.1 Tap the portafilter on the tamper
        Tamper.tap_tamper()

        # Finishes at local home

    def scrape_portafilter():
        # Move to target positions (scrape portafilter)
        T_B_M__waypoints = [
            get_T2RDK(mm(Tamper.T_B_T, T_T_differenceWaypoint, Tamper.T_T_scraperBottomRef, inv(Tools.T_TCP_filterToolTop), inv(Tools.T_M_TCP)))
            for T_T_differenceWaypoint in 
            [Tamper.dT_scraperDifferenceWaypoint1, Tamper.dT_scraperDifferenceWaypoint2, Tamper.dT_scraperDifferenceWaypoint3, Tamper.dT_scraperDifferenceWaypoint4]
        ]
        robot.MoveJ(T_B_M__waypoints[3], blocking=True)
        rdk.pause(0.5) # TODO remove this
        robot.MoveL(T_B_M__waypoints[0], blocking=True)
        rdk.pause(0.5) # TODO remove this
        robot.MoveL(T_B_M__waypoints[1], blocking=True)
        rdk.pause(0.5) # TODO remove this
        robot.MoveL(T_B_M__waypoints[2], blocking=True)
        rdk.pause(0.5) # TODO remove this
        robot.MoveL(T_B_M__waypoints[3], blocking=True)
        rdk.pause(0.5) # TODO remove this

        # Finish at local home
        Tamper.goto_local_home()

    def tap_tamper():
        # Define positions as RDK matrices in base frame
        T_B_M__tamperCleared = get_T2RDK(mm(Tamper.T_B_T, Tamper.T_T_tamperCleared, inv(Tools.T_TCP_filterToolTop), inv(Tools.T_M_TCP)))
        T_B_M__tamperInside = get_T2RDK(mm(Tamper.T_B_T, Tamper.T_T_tamperInside, inv(Tools.T_TCP_filterToolTop), inv(Tools.T_M_TCP)))
        
        # Tamp
        robot.MoveJ(T_B_M__tamperCleared, blocking=True)
        rdk.pause(1) # TODO remove
        robot.MoveL(T_B_M__tamperInside, blocking=True)
        rdk.pause(1) # TODO remove
        robot.MoveL(T_B_M__tamperCleared, blocking=True)
        rdk.pause(1) # TODO remove

        # Finish at local home
        Tamper.goto_local_home()
        
class ToolStand:
    """Class for accessing Tamper related transforms and tasks"""
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
    GH_INSERSION_RAISE = 0 # 30 #21.6988 #mm # Z = 188.6988 is about where it crosses threshold
    GH_OFFSET_X = 0     # @Zach TODO BUG : Calibrate grouphead
    GH_OFFSET_Y = -1

    P_S_groupHeadApproach = np_col(9.5 + 6.047 - 2.6, 67.3 + 5.531865 + GH_OFFSET_Y, 167.0)
    R_S_groupHeadApproach = get_R_given_theta(-90, 'y')


    P_S_groupHeadInserted = P_S_groupHeadApproach + np_col(0, 0, GH_INSERSION_RAISE)
    R_S_groupHeadInserted = R_S_groupHeadApproach

    T_S_groupHeadApproach = get_T_given_P_and_R(P_S_groupHeadApproach, R_S_groupHeadApproach)
    T_S_groupHeadInserted = get_T_given_P_and_R(P_S_groupHeadInserted, R_S_groupHeadInserted)
    
    T_S_groupHeadHalfRotated = get_T_given_P_and_R(P_S_groupHeadInserted, mm(R_S_groupHeadInserted, get_R_given_theta(15, 'x')))
    T_S_groupHeadRotated = get_T_given_P_and_R(P_S_groupHeadInserted, mm(R_S_groupHeadInserted, get_R_given_theta(30, 'x')))

    T_B_GHwayPoint = np.array([[     0.225069,     0.064360,    -0.972215,  -386.505056],
      [0.765282,     0.605917,     0.217275,   -48.791066],
      [0.603066,    -0.792920,     0.087119,   207.748804],
      [0.000000,     0.000000,     0.000000,     1.000000 ]])
    
    T_S_GH_waypt = mm(inv(T_B_S), T_B_GHwayPoint)

    def goto_local_home():
        robot.MoveJ([-138.87, -88.94, -142.67, -115.05, -34.46, -231.06], blocking=True)

    @staticmethod
    def insert_portafilter_in_group_head_and_release():
        # Move up to insersion position
        T_B_M__groupHeadApproach = mm(ToolStand.T_B_S, ToolStand.T_S_groupHeadApproach, inv(Tools.T_TCP_filterToolCentre), inv(Tools.T_M_TCP))
        robot.MoveJ(get_T2RDK(T_B_M__groupHeadApproach), blocking=True)

        # Raise into insersion position
        T_B_M__groupHeadInserted = mm(ToolStand.T_B_S, ToolStand.T_S_groupHeadInserted, inv(Tools.T_TCP_filterToolCentre), inv(Tools.T_M_TCP))
        robot.MoveL(get_T2RDK(T_B_M__groupHeadInserted), blocking=True)

        rdk.pause(1)

        # Rotate 30 degrees
        T_B_M__groupHeadHalfRotated = mm(ToolStand.T_B_S, ToolStand.T_S_groupHeadHalfRotated, inv(Tools.T_TCP_filterToolCentre), inv(Tools.T_M_TCP))
        T_B_M__groupHeadRotated = mm(ToolStand.T_B_S, ToolStand.T_S_groupHeadRotated, inv(Tools.T_TCP_filterToolCentre), inv(Tools.T_M_TCP))

        # MoveC rotation of the grouphead
        robot.MoveC(get_T2RDK(T_B_M__groupHeadHalfRotated), get_T2RDK(T_B_M__groupHeadRotated), blocking=True)
        
        rdk.pause(1)

        # Unwind the 30deg rotation
        robot.MoveC(get_T2RDK(T_B_M__groupHeadHalfRotated), get_T2RDK(T_B_M__groupHeadInserted), blocking=True)

        rdk.pause(1)


        # Drop away from group head
        robot.MoveL(get_T2RDK(T_B_M__groupHeadApproach), blocking=True)

        rdk.pause(1)

        # Move a short distance away for TA pickup
        robot.MoveJ(transform_RDK_pose(robot.Pose(), Tools.T_M_TCP, deltaP_C =np_col(0,60,0) ,  deltaR_C=get_R_given_theta(-75, 'z')), blocking=True)

        # Wait while TA detach tool
        rdk.pause(5)
        
        # Finish at local home
        ToolStand.goto_local_home()

class CupStand:
    """Class for accessing CupStand related transforms and tasks"""
    # Cupstand frame finding points
    P0_B_cupStand = np.array([[-1.5],[-600.3],[-20]]) # cup stand frame origin in base frame coords
    R_B_cupStandBase = get_R_given_theta(90, 'z')

    T_B_CS = get_T_given_P_and_R(P0_B_cupStand, R_B_cupStandBase)

    # Make sure cup tool is rotated 180 deg for pickup
    CS_cupPickup_OFFSET_X = 1 # off by 1mm right 
    CS_cupPickup_OFFSET_Y = 0
    CS_cupPickup_OFFSET_Z = -30 - 9.2
    CS_cupApproach_OFFSET_X = 120
    
    # Pick up position where actually pickup the cup
    P_CS_cupPickup = np.array([[0+CS_cupPickup_OFFSET_X],[0+CS_cupPickup_OFFSET_Y],[217 + CS_cupPickup_OFFSET_Z]])
    R_CS_cupPickup = np.matmul(get_R_given_theta(-90, 'y'), get_R_given_theta(180, 'z'))

    T_CS_cupPickup = get_T_given_P_and_R(P_CS_cupPickup, R_CS_cupPickup)

    # Approach position where we open the cup tool
    P_CS_cupPickupApproach = P_CS_cupPickup + np_col(CS_cupApproach_OFFSET_X,0,0)
    T_CS_cupPickupApproach = get_T_given_P_and_R(P_CS_cupPickupApproach, R_CS_cupPickup)
    
    # DROP OFF POSITIONS (this is 180deg flipped frame to pickup)
    # Cup drop off position (offset by cup)
    CS_cupDropoff_OFFSET_Z = 0
    # having 0 will yeild ~4mm above

    P_CS_cupDropoff = P_CS_cupPickup + np_col(0, 0, CS_cupDropoff_OFFSET_Z - CS_cupPickup_OFFSET_Z + 80)
    R_CS_cupDropoff = get_R_given_theta(-90, 'y')
    T_CS_cupDropoff = get_T_given_P_and_R(P_CS_cupDropoff, R_CS_cupDropoff)

    # Approach from above the cup stand
    P_CS_cupDropoffApproach = P_CS_cupPickup + Tools.dP_B_cupHeight + np_col(0, 0, 150)
    T_CS_cupDropoffApproach = get_T_given_P_and_R(P_CS_cupDropoffApproach, R_CS_cupDropoff)

    @staticmethod
    def pickup_coffee_cup_from_stand():
        # Define waypoints
        waypoint_1 = [-179.480621, -76.857593, -78.635022, -114.590793, -10, -205.648890]
        waypoint_2 = [-94.410000, -76.850000, -78.630000, -114.590000, -62.510000, -40.000000]
        waypoint_3 = [-69.510000, -72.720000, -153.080000, -134.190000, -53.970000, -40.000000]
        waypoint_4 = [-54.990000, -72.720000, -153.080000, -134.190000, -54.990000, -40.000000]

        # Loop over expresso
        robot.MoveJ(waypoint_1, blocking=True)
        robot.MoveJ(waypoint_2, blocking=True)
        robot.MoveJ(waypoint_3, blocking=True)
        robot.MoveJ(waypoint_4, blocking=True)

        T_B_M__cupStackPickupApproach = mm(CupStand.T_B_CS, CupStand.T_CS_cupPickupApproach, inv(Tools.T_TCP_cupTool), inv(Tools.T_M_TCP))
        robot.MoveJ(get_T2RDK(T_B_M__cupStackPickupApproach), blocking=True)

        # Open cup tool
        run_program_cup_tool_open()
        rdk.pause(2)
        
        # Linear approach cup with tool open
        T_B_M__cupStackPickupPos = mm(CupStand.T_B_CS, CupStand.T_CS_cupPickup, inv(Tools.T_TCP_cupTool), inv(Tools.T_M_TCP))
        robot.MoveL(get_T2RDK(T_B_M__cupStackPickupPos), blocking=True)
        rdk.pause(2)

        # Close cup tool
        run_program_cup_tool_close()
        rdk.pause(2)

        # Clear cup tool
        robot.MoveL(transform_RDK_pose(robot.Pose(), CupStand.T_B_CS, Tools.dP_B_cupHeight), blocking=True)
        # Retreat from cup
        robot.MoveL(transform_RDK_pose(robot.Pose(), CupStand.T_B_CS, Tools.dP_E_cupReleaseRetreat), blocking=True)

        # Rotate cup as having it inverted messes up navigation
        robot.MoveJ(transform_RDK_pose(robot.Pose(), CupStand.T_B_CS, deltaR_C=get_R_given_theta(-60, 'z')), blocking=True)
        
        # TODO: Rotate 6th joint 180degrees to flip cup right way
        # @-Zach TODO after lab if there is time
        # I tried this but it didn't work. I want a 180 deg rotation about the z axis in the tool frame
        # robot.MoveJ(transform_RDK_pose(robot.Pose(), np.eye(3), deltaR_C=get_R_given_theta(-180, 'z')), blocking=True)

        # This was the solution to rotate cup 180 deg, but surely we can fix transform_RDK_pose unless it's me using it wrong
        joint_angles = list(np.array(robot.Joints()).reshape(6,))  # extract joint positions
        joint_angles[5] += 180                                     # rotate joint 6 by 180 deg
        robot.MoveJ(joint_angles, blocking=True)


    @staticmethod
    def dropoff_coffee_cup_on_stand():
        
        # Approach cup stand from above the dropoff
        T_B_M__cupStackDropoffApproach = mm(CupStand.T_B_CS, CupStand.T_CS_cupDropoffApproach, inv(Tools.T_TCP_cupTool), inv(Tools.T_M_TCP))
        robot.MoveJ(get_T2RDK(T_B_M__cupStackDropoffApproach), blocking=True)

        # Linear move vertically downwards to the cup dropoff position
        T_B_M__cupStackDropoff = mm(CupStand.T_B_CS, CupStand.T_CS_cupDropoff, inv(Tools.T_TCP_cupTool), inv(Tools.T_M_TCP))
        robot.MoveL(get_T2RDK(T_B_M__cupStackDropoff), blocking=True)

        # Open cup tool
        run_program_cup_tool_open()
        rdk.pause(2)

        # Move down slightly to remove weight from holder
        #robot.MoveL(transform_RDK_pose(robot.Pose(), CupStand.T_B_CS, np_col(0, 0, -10)), blocking=True)
        #rdk.pause(1)
        # Clear cup area (move back by 120mm)
        robot.MoveL(transform_RDK_pose(robot.Pose(), CupStand.T_B_CS, np_col(CupStand.CS_cupApproach_OFFSET_X, 0, 0)), blocking=True)
        
        rdk.pause(1)
        # Close cup again
        run_program_cup_tool_close()


class EspressoMachine:
    """Class for accessing EspressoMachine related transforms and tasks"""
    # Espresso frame finding points
    P0_B_espresso = np.array([[-365.5],[-386.7],[349.8]]) # Point 0 (origin) of espresso machine
    P1_B_espresso = np.array([[-577.8],[-441.6],[349.4]])
    P1_espresso_E = np.array([[0],[218],[0]])

    # Position of first button
    E_BTN_OFFSET_Y = 5 # (Y+ve is right and -ve is left)
    E_BTN_OFFSET_Z = 6+4 # (+ve is up and -ve is down)
    P_E_btn1 = np.array([[50.67],[35.25 + E_BTN_OFFSET_Y],[-27.89 + E_BTN_OFFSET_Z]])

    # Espresso frame in base frame
    T_B_E = get_T_to_foreign_frame(P1_B_espresso, P0_B_espresso, P1_espresso_E)

    # Button 1 frame in espresso frame
    T_E_btn1 = get_T_given_P_and_R(P_E_btn1, get_R_given_theta(-90, 'y'))
    dP_pushTool_depressBtn = np_col(-8, 0, 0)
    dP_pushTool_Btn1MovDown = np_col(0, 0, -10)

    # Cup position frames
    E_DROPOFF_OFFSET_X = 7.5 
    E_DROPOFF_OFFSET_Y = -2
    E_DROPOFF_OFFSET_Z = -2 # Account for how we grip the cup ie change this to make cup base come in as close to the tray as possible

    P_E_cupTray = np.array([[-12.68 + E_DROPOFF_OFFSET_X],[72.0 + E_DROPOFF_OFFSET_Y],[-290]]) # Position ON TRAY to place the coffee
    P_E_cupTrayDropPos = P_E_cupTray + Tools.dP_B_cupHeight + np_col(0,0,E_DROPOFF_OFFSET_Z) # Position of cup tool to pplace the coffee
    R_E_cupTrayDropPos = np.matmul(get_R_given_theta(-90, 'y'), get_R_given_theta((-50-16), 'x'))

    T_E_cupDropoff = get_T_given_P_and_R(P_E_cupTrayDropPos, R_E_cupTrayDropPos)
    
    T_B_cupEspressoDropoff = mm(T_B_E, inv(T_E_cupDropoff))
    
    T_E_cupPickupApproach = get_T_given_P_and_R(P_E_cupTrayDropPos + Tools.dP_E_cupReleaseRetreat, R_E_cupTrayDropPos)

    # Reused Expresso waypoints
    waypoint_left = [-69.510000, -57.960000, -147.540000, -154.480000, -53.970000, -220.000000]
    waypoint_front_left = [-61.531184, -49.226202, -150.522433, -160.251364, -10.030043, -220.000000]
    
    waypoint_left_flipped_wrist = waypoint_left[:5] + [waypoint_left[5] + 360]
    waypoint_front_left_flipped_wrist = waypoint_front_left[:5] + [waypoint_front_left[5] + 360]

    @staticmethod
    def press_espresso_button():
        # Define task-specific transforms
        T_B_M__espressoBtn1 = mm(EspressoMachine.T_B_E, EspressoMachine.T_E_btn1, inv(Tools.T_TCP_grinderToolPush), inv(Tools.T_M_TCP))

        # Waypooints to avoid top left corner of espresso machine
        waypoint_1 = Tools.J_tool_approach
        waypoint_2 = [-154.600000, -81.000000, -89.180000, -113.000000, 89.000000, -220.000000]
        waypoint_3 = [-154.604733, -98.575329, -89.184891, -172.239780, 190.896409, -220.000000]
        robot.MoveJ(waypoint_1, blocking=True)
        robot.MoveJ(waypoint_2, blocking=True)
        robot.MoveJ(waypoint_3, blocking=True)

        # Move to target position
        robot.MoveJ(get_T2RDK(T_B_M__espressoBtn1), blocking=True)

        # Depress button
        robot.MoveL(transform_RDK_pose(robot.Pose(), EspressoMachine.T_B_E, EspressoMachine.dP_pushTool_depressBtn), blocking=True)
        robot.MoveL(transform_RDK_pose(robot.Pose(), EspressoMachine.T_B_E, -EspressoMachine.dP_pushTool_depressBtn), blocking=True)
        rdk.pause(3)
        # Turn button off
        robot.MoveL(transform_RDK_pose(robot.Pose(), EspressoMachine.T_B_E, EspressoMachine.dP_pushTool_Btn1MovDown), blocking=True)
        robot.MoveL(transform_RDK_pose(robot.Pose(), EspressoMachine.T_B_E, EspressoMachine.dP_pushTool_depressBtn), blocking=True)
        robot.MoveL(transform_RDK_pose(robot.Pose(), EspressoMachine.T_B_E, -EspressoMachine.dP_pushTool_depressBtn), blocking=True)
        rdk.pause(1)
        robot.MoveJ(waypoint_3, blocking=True)
        robot.MoveJ(waypoint_2, blocking=True)
        robot.MoveJ(waypoint_1, blocking=True)

    @staticmethod
    def dropoff_cup_at_espresso():
        # Waypoints
        waypoint_1 = [-65.360000, -51.900000, -147.420000, -160.620000, -42.540000, 139.990000]

        robot.MoveJ(waypoint_1, blocking=True)
        robot.MoveJ(EspressoMachine.waypoint_left_flipped_wrist, blocking=True)
        robot.MoveL(EspressoMachine.waypoint_front_left_flipped_wrist, blocking=True)

        # Move to target position
        T_B_M__espressoCupDropoff = mm(EspressoMachine.T_B_E, EspressoMachine.T_E_cupDropoff, inv(Tools.T_TCP_cupTool), inv(Tools.T_M_TCP))
        robot.MoveL(get_T2RDK(T_B_M__espressoCupDropoff), blocking=True)

        # Open cup tool
        run_program_cup_tool_open()

        # Move up slightly to remove weight from holder (cup is light and easily gets stuck)
        robot.MoveL(transform_RDK_pose(robot.Pose(), EspressoMachine.T_E_cupPickupApproach, np_col(1, 0, 0)), blocking=True)
        rdk.pause(2)
        # Clear cup area
        robot.MoveL(transform_RDK_pose(robot.Pose(), EspressoMachine.T_E_cupPickupApproach, Tools.dP_E_cupReleaseRetreat), blocking=True)
        rdk.pause(2)
        # Close cup again
        run_program_cup_tool_close()

        # Move out of the way of expresso
        robot.MoveJ(EspressoMachine.waypoint_front_left_flipped_wrist, blocking=True)
        robot.MoveJ(EspressoMachine.waypoint_left_flipped_wrist, blocking=True)

        # Define waypoints for looping over expresso in reverse
        waypoint_4 = [-94.410000, -76.850000, -78.630000, -114.590000, -62.510000, -40.000000]
        waypoint_5 = [-179.480621, -76.857593, -78.635022, -114.590793, -10, -205.648890]
        waypoint_6 = waypoint_5[:4] + [90] + [waypoint_5[5]]  # rotate only joint 5

        # Loop over expresso (reverse)
        robot.MoveJ(waypoint_4, blocking=True)
        robot.MoveJ(waypoint_5, blocking=True)
        robot.MoveJ(waypoint_6, blocking=True)

    @staticmethod
    def pickup_cup_at_espresso():
        # Define waypoints
        waypoint_1 = [-179.480621, -76.857593, -78.635022, -114.590793, -10, -205.648890]
        waypoint_2 = [-94.410000, -76.850000, -78.630000, -114.590000, -62.510000, -220.000000]


        # Loop over expresso
        robot.MoveJ(waypoint_1, blocking=True)
        robot.MoveJ(waypoint_2, blocking=True)

        # Position in front of Espresso
        robot.MoveJ(EspressoMachine.waypoint_left, blocking=True)
        robot.MoveL(EspressoMachine.waypoint_front_left, blocking=True)

        # Open cup tool
        run_program_cup_tool_open()

        # Linear move to cup collection posiiton on the espresso machine tray
        T_B_M__espressoCupDropoff = mm(EspressoMachine.T_B_E, EspressoMachine.T_E_cupDropoff, inv(Tools.T_TCP_cupTool), inv(Tools.T_M_TCP))
        robot.MoveL(get_T2RDK(T_B_M__espressoCupDropoff), blocking=True)

        # Close cup again
        run_program_cup_tool_close()

        # Clear cup area
        robot.MoveL(transform_RDK_pose(robot.Pose(), EspressoMachine.T_E_cupPickupApproach, Tools.dP_E_cupReleaseRetreat), blocking=True)
        
        rdk.pause(2)

        waypoint_7 = [-67.671244, -72.054990, -108.802789, -179.142220, -50, -220.000000] # same z level as cup stand

        robot.MoveL(EspressoMachine.waypoint_front_left, blocking=True)
        robot.MoveJ(EspressoMachine.waypoint_left, blocking=True)
        robot.MoveL(waypoint_7, blocking=True)  # rise to just above cup stand height (MoveL for non spilt coffee)




#_____________________________________________________________________________________
#                                                                                     #
#                                     PROGRAM TASKS                                   #
#                                                                                     #
#_____________________________________________________________________________________#

def main():

    # Start at global_home
    goto_global_home()

    # -----------------------------------------------------------------------------------------
    # Steps A-C including picking portafilter up from grinder
    # -----------------------------------------------------------------------------------------
    # Waypoint to not hit on grinder when moving from global home to grinder local home
    robot.MoveJ([-10.530580, -68.163175, -135.196270, -148.262561, -55.575207, 135.566662], blocking=True)
   
    # Routine
    Grinder.goto_local_home() # Start at Grinder local home
    Grinder.routine()

    # -----------------------------------------------------------------------------------------
    # Steps E-D
    # -----------------------------------------------------------------------------------------
    Tamper.goto_local_home() # Start to Tamper local home
    Tamper.routine()

    # -----------------------------------------------------------------------------------------
    # Step F
    # -----------------------------------------------------------------------------------------
    # Waypoint to avoid express machine from Tamper local home
    robot.MoveJ([-111.697098, -66.949527, -156.211706, -129.799260, -79.417478, -220.961677], blocking=True)
    
    ToolStand.goto_local_home() # Start at ToolStand local home
    ToolStand.insert_portafilter_in_group_head_and_release()

    # Waypoint to avoid hitting on grouphead
    robot.MoveJ([-141.265710, -75.517999, -115.260901, -126.107561, 31.985853, -197.333130], blocking=True)

    # G.1 Get the coffee cup tool
    Tools.get_cup_tool(is_pickup=True)

    # G.2 Pick up coffee cup with cup tool
    CupStand.pickup_coffee_cup_from_stand()

    # H.1 Place coffee cup in drip tray of the coffee machine under the group head
    EspressoMachine.dropoff_cup_at_espresso()

    # # I.1 Put back the cup tool
    Tools.get_cup_tool(is_pickup=False)

    # I.2 Get the grinder tool
    Tools.get_grinder_tool(is_pickup=True)

    # I.3 Move to the espresso button 1 and turn the espresso on for 3 seconds
    EspressoMachine.press_espresso_button()

    # J.1 Put back the grinder tool
    Tools.get_grinder_tool(is_pickup=False)

    # J.2 Get the cup tool
    Tools.get_cup_tool(is_pickup=True)

    # J.3 Move to the coffee machine and pick up the cup of coffee
    EspressoMachine.pickup_cup_at_espresso()

    # J.4 Place steaming cup of coffee upright on the cup stand
    # J.5 Release coffee cup and put back the cup tool
    CupStand.dropoff_coffee_cup_on_stand()

    # # J.6 Return Cup tool
    # TODO WAYPOINT
    Tools.get_cup_tool(is_pickup=False)

    # # J.7 Return Home
    robot.MoveJ(home, blocking=True)




def test_routine():
    # Start at home
    robot.MoveJ(T_home, blocking=True)

     # G.1 Get the coffee cup tool
    Tools.get_cup_tool(is_pickup=True)

    # G.2 Pick up coffee cup with cup tool
    CupStand.pickup_coffee_cup_from_stand()

    # H.1 Place coffee cup in drip tray of the coffee machine under the group head
    EspressoMachine.dropoff_cup_at_espresso()

    # I.1 Put back the cup tool
    Tools.get_cup_tool(is_pickup=False)



def grouphead_test():
    # -----------------------------------------------------------------------------------------
    # Steps A-C including picking portafilter up from grinder
    # -----------------------------------------------------------------------------------------
    # Waypoint to not hit on grinder when moving from global home to grinder local home
    robot.MoveJ([-10.530580, -68.163175, -135.196270, -148.262561, -55.575207, 135.566662], blocking=True)
   
    # Routine
    Grinder.goto_local_home() # Start at Grinder local home
    Grinder.goto_local_home(rotate_wrist_360=True)  # rotate 360 deg
    #Grinder.routine()

    # -----------------------------------------------------------------------------------------
    # Steps E-D
    # -----------------------------------------------------------------------------------------
    Tamper.goto_local_home() # Start to Tamper local home
    Tamper.routine()

    # -----------------------------------------------------------------------------------------
    # Step F
    # -----------------------------------------------------------------------------------------
    # Waypoint to avoid express machine from Tamper local home @ Sam collision with espresso machine
    #robot.MoveJ([-111.697098, -66.949527, -156.211706, -129.799260, -79.417478, -220.961677], blocking=True)
    robot.MoveJ([-111.279706, -65.677757, -156.849177, -130.423804, -79.003237, -221.013786], blocking=True)
    
    ToolStand.goto_local_home() # Start at ToolStand local home
    ToolStand.insert_portafilter_in_group_head_and_release()




def cup_espresso_test_routine():
    # G.1 Get the coffee cup tool
    Tools.get_cup_tool(is_pickup=True)

    # G.2 Pick up coffee cup with cup tool
    CupStand.pickup_coffee_cup_from_stand()

    # H.1 Place coffee cup in drip tray of the coffee machine under the group head
    EspressoMachine.dropoff_cup_at_espresso()

    # # I.1 Put back the cup tool
    Tools.get_cup_tool(is_pickup=False)

    # I.2 Get the grinder tool
    Tools.get_grinder_tool(is_pickup=True)

    # I.3 Move to the espresso button 1 and turn the espresso on for 3 seconds
    EspressoMachine.press_espresso_button()

    # J.1 Put back the grinder tool
    Tools.get_grinder_tool(is_pickup=False)

    # J.2 Get the cup tool
    Tools.get_cup_tool(is_pickup=True)

    # J.3 Move to the coffee machine and pick up the cup of coffee
    EspressoMachine.pickup_cup_at_espresso()

    # J.4 Place steaming cup of coffee upright on the cup stand
    # J.5 Release coffee cup and put back the cup tool
    CupStand.dropoff_coffee_cup_on_stand()





if __name__ == '__main__':
    # goto_global_home()
    main()
    #cup_espresso_test_routine()

    
    # Tool Returns
    #Tools.get_filter_tool(is_pickup=False)
    #Tools.get_cup_tool(is_pickup=False)
    #Tools.get_grinder_tool(is_pickup=False)

    # # TEST #1 picking cup up from cupstand and dropping off at espresso
    #Tools.get_cup_tool(is_pickup=True)
    #CupStand.pickup_coffee_cup_from_stand()
    #EspressoMachine.dropoff_cup_at_espresso()
    #Tools.get_cup_tool(is_pickup=False)

    # TEST # 2 Espresso button press test
    # I.2 Get the grinder tool
    #Tools.get_grinder_tool(is_pickup=True)
    #EspressoMachine.press_espresso_button()
    #Tools.get_grinder_tool(is_pickup=False)
    

    #Tools.get_filter_tool(is_pickup=True)

    # # Test #2 picking cup up from expresso and dropping off
    # Tools.get_cup_tool(is_pickup=True)
    #EspressoMachine.pickup_cup_at_espresso()
    #CupStand.dropoff_coffee_cup_on_stand()



    #_______________________
    # TEST SUCCESSFUL
    
    # TEST SUCCESSFUL
    # # TEST #3 Tamper
    #robot.MoveJ([30.080000, -72.000000, -146.280000, -119.530000, -9.450000, -120.030000], blocking=True) # temporary waypoint for testing
    #Tamper.goto_local_home()
    #Tamper.scrape_portafilter()
    #Tamper.tap_tamper()

    # TEST SUCCESSFUL
    # Test #2b Doser lever pull 
    ###Tools.get_grinder_tool(is_pickup=True)
    #robot.MoveJ([-10.530580, -68.163175, -135.196270, -148.262561, -55.575207, 135.566662], blocking=True)
    #Grinder.goto_local_home() # Start at Grinder local home
    #Grinder.pull_doser()
    #Grinder.goto_local_home()


    # # Test 2c grouphead isolated test
    # Tools.get_filter_tool(is_pickup=True)
    # # Waypoint to avoid express machine from Tamper local home
    # robot.MoveJ([-111.279706, -65.677757, -156.849177, -130.423804, -79.003237, -221.013786], blocking=True)
    # ToolStand.goto_local_home() # Start at ToolStand local home
    # ToolStand.insert_portafilter_in_group_head_and_release()
    # # Waypoint to avoid hitting on grouphead
    # robot.MoveJ([-141.265710, -75.517999, -115.260901, -126.107561, 31.985853, -197.333130], blocking=True)

    # # G.1 Get the coffee cup tool
    # Tools.get_cup_tool(is_pickup=True)

    # # G.2 Pick up coffee cup with cup tool
    # CupStand.pickup_coffee_cup_from_stand()

    # # H.1 Place coffee cup in drip tray of the coffee machine under the group head
    # EspressoMachine.dropoff_cup_at_espresso()
    

    

    # # Test #4 Grinder drop off and pickup (with portafilter already attached)
    # Tools.get_grinder_tool(is_pickup=True)
    # Grinder.goto_local_home()
    # Grinder.place_portafilter(is_pickup=False)
    # Grinder.goto_local_home()
    # Grinder.pull_doser()
    # rdk.pause(3)
    # Grinder.place_portafilter(is_pickup=True)

    # # Test #5 Grouphead
    # Tamper.goto_local_home() # Start to Tamper local home
    # # Waypoint to avoid express machine from Tamper local home
    #robot.MoveJ([-111.697098, -66.949527, -156.211706, -129.799260, -79.417478, -220.961677], blocking=True)
    #ToolStand.goto_local_home() # Start at ToolStand local home
    #ToolStand.insert_portafilter_in_group_head_and_release()

    # # Test #6 Grouphead calibration
    #grouphead_test()


    # # J.7 Return Home
    #robot.MoveJ(home, blocking=True)

    # Test #5 Main
    # main()
    

