import numpy as np
import os
import robodk as rdk
import sys
from typing import Literal, Optional
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
    [temp[1][0], temp[1][1], temp[1][2], temp[1][3]], [temp[2][0], temp[2][1], temp[2][2], temp[2][3]], [temp[3][0], temp[3][1], temp[3][2], temp[3][3]]])

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
R_NO_ROT = np.identity(3)
P_NO_DISP = np.zeros((3,1))


def get_R_given_theta(theta, axis: Literal['x', 'y', 'z'], radians=False):
    """Returns the rotation matrix for a single-axis rotation"""
   
    # convert theta to radians
    if not radians:
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


def get_T_given_P_and_R(P_BA: Optional[np.ndarray], R_AB: Optional[np.ndarray]) -> np.ndarray:
    """Constructs T transform given displacement and rotation matrix, accepts 'None' as a matrix represensting no translation of rotation"""
    if P_BA is None:
        P_BA = np_col(0, 0, 0)
    if R_AB is None:
        R_AB = np.eye(3)

    T_AB = np.concatenate((R_AB, P_BA), axis=1)
    T_AB = np.concatenate((T_AB, np.array([[0,0,0,1]])), axis=0)
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
    P_BQ = P_AQ-P_AB
    # theta is angle about x-axis of frame A to frame B (first element)
    # if (force_theta):
    #     theta=force_theta
    # else: ie param -> force_theta=np.cos(np.deg2rad(90)
    theta = ( np.arctan2(P_BQ[1], P_BQ[0]) - np.arctan2(P_QB[1],P_QB[0]) )[0]  

    # 2. Determine rotation matrix between frames (Z-axis rotation)
    R_AB = get_R_given_theta(theta, axis='z', radians=True)

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


#_____________________________________________________________________________________
#                                                                                     #
#                              Debugging functions                                    #
#                                                                                     #
#_____________________________________________________________________________________#
class COLOURS:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

depth = 0

def trace(frame, event, arg):
    # Inspired by https://stackoverflow.com/a/5104943 https://stackoverflow.com/a/32607930
    global depth
    filename = frame.f_code.co_filename
    if os.getcwd() in filename and 'util.py' != os.path.split(filename)[1]:
        if event == "call":
            print(f'{"|--" * depth}| {COLOURS.OKBLUE}{frame.f_code.co_name}()'.ljust(50) + f'{COLOURS.OKGREEN}[{os.path.split(filename)[1]}]{COLOURS.ENDC}')
            depth += 1
        elif event == 'return':
            depth -= 1
    return trace

debug_enable = lambda: sys.settrace(trace)
debug_disable = lambda: sys.settrace(None)