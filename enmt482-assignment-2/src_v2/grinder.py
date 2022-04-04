import robodk as rdk

import robot_lib
import util
import tools

robot = robot_lib.robot


# Grinder frame finding points
P0_B_grinder = util.np_col(482.7, -432.1, 316.1)
P1_B_grinder = util.np_col(370.5, -322.5, 65.9)
P1_grinder_G = util.np_col(157.61, 0, -250.45)

T_B_G = util.get_T_to_foreign_frame(P1_B_grinder, P0_B_grinder, P1_grinder_G)

# Place portafilter positions
P_G_PFdropoff = util.np_col(157.61, 0, -250.45)
R_G_PFdropoff = util.get_R_given_theta(-90, 'y')
T_G_PFdropoff = util.get_T_given_P_and_R(P_G_PFdropoff, R_G_PFdropoff)

# Lever transforms
P_G_leverDepressed = util.np_col(-35.82, 83.80, -153.00)
R_G_leverDepressed = util.R_NO_ROT
T_G_leverDepressed = util.get_T_given_P_and_R(P_G_leverDepressed, R_G_leverDepressed)

LEVER_PULL_OFFSET = 50
P_G_leverPressed = util.np_col(-35.82 + LEVER_PULL_OFFSET, 83.80, -153.00)
R_G_leverPressed = util.R_NO_ROT
T_G_leverPressed = util.get_T_given_P_and_R(P_G_leverPressed, R_G_leverPressed)

# Button positions
dP_pushTool_depressGrinderBtn1 = util.np_col(-6,0,0)
dP_pushTool_depressGrinderBtn2 = util.np_col(-4,0,0)
dP_Gbtn_btnSpace = util.np_col(0,0,-20) # Horizontal spacing of buttons

P_G_btn1 = util.np_col(-64.42, 89.82, -227.68)
P_G_btn2 = util.np_col(-80.71, 94.26, -227.68)
R_G_btn1 = util.mm(util.get_R_given_theta(-90, 'y'), util.get_R_given_theta(80, 'x')) #TODO: should be 72deg
T_G_btn1 = util.get_T_given_P_and_R(P_G_btn1, R_G_btn1)
T_B_Gbtn1 = util.mm(T_B_G, util.inv(T_G_btn1))


def goto_local_home():
    """Localised "home" position that is a safe distance away from the grinder"""
    robot.MoveJ([-17.172086, -99.768847, -141.220021, -111.198773, -62.150759, 136.658726], blocking=True)


def routine(pickup_and_dropoff_tools=True):
    """Routine which covers steps A-C including picking portafilter up from grinder. Starts at global home and finishes at grinder local home"""
    util.debug_enable()
    # Start at global home
    robot_lib.goto_global_home()

    # A.1 Get filter tool
    tools.get_filter_tool(is_pickup=True)

    # A.2 Move to grinder & place portafilter tool onto the grinder and detach tool
    place_portafilter(is_pickup=False)

    # B.1 Get grinder tool
    tools.get_grinder_tool(is_pickup=True)

    # B.2 Move to grinder & turn on the grinder, wait 3 seconds, turn the grinder off
    press_buttons()

    # C.1 Pull the grider dosing lever 3 times
    pull_doser()

    # C.2 Return the grinder tool
    tools.get_grinder_tool(is_pickup=False)

    # C.3 Pick up the portafilter
    place_portafilter(is_pickup=True)

    # Finish at local_home
    goto_local_home()
    util.debug_disable()


def place_portafilter(is_pickup=False):
    # Waypoints
    
    # # Move to target position
    # T_B_M__grinderPFdropoff = util.mm(T_B_G, T_G_PFdropoff, util.inv(tools.T_TCP_filterToolEdge), util.inv(tools.T_M_TCP))
    # robot.MoveJ(util.get_T2RDK(T_B_M__grinderPFdropoff), blocking=True)

    # robot.MoveJ(util.transform_RDK_pose(robot.Pose(), tools.T_M_TCP, util.np_col(20,0,100)), blocking=True)

    robot.MoveJ([-17.172086, -99.768847, -141.220021, -111.198773, -62.150759, 136.658726], blocking=True)

    # Detach filter tool
    if (is_pickup):
        robot_lib.filter_tool_attach_grinder.RunCode(robot_lib.filter_tool_attach_grinder) # call subprogram
    else:
        robot_lib.filter_tool_detach_grinder.RunCode(robot_lib.filter_tool_detach_grinder) # call subfunction

    robot_lib.better_wait_finished()
    

def press_buttons():
    # Waypoint
    robot.MoveJ([-60.16, -147.65, -52.07, -160.28, 174.16, -220.00], blocking=True)

    # Move to target position
    T_B_M__grinderBtn1 = util.mm(T_B_G, T_G_btn1, util.inv(tools.T_TCP_grinderToolPush), util.inv(tools.T_M_TCP))
    robot.MoveJ(util.get_T2RDK(T_B_M__grinderBtn1), blocking=True)
    
    # Depress button
    rdk.pause(1)
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_B_Gbtn1, -dP_pushTool_depressGrinderBtn1), blocking=True)
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_B_Gbtn1, dP_pushTool_depressGrinderBtn1), blocking=True)
    
    # Wait
    rdk.pause(3)

    # Turn button off
    robot.MoveJ(util.transform_RDK_pose(robot.Pose(), T_B_Gbtn1, dP_Gbtn_btnSpace), blocking=True)
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_B_Gbtn1, -dP_pushTool_depressGrinderBtn2), blocking=True)
    robot.MoveL(util.transform_RDK_pose(robot.Pose(), T_B_Gbtn1, dP_pushTool_depressGrinderBtn2), blocking=True)


def pull_doser():
    # Waypoints
    robot.MoveJ([-47.128214, -116.453451, -96.410577, -147.135972, -92.799820, 230.000000], blocking=True)

    for _ in range(3):
        # Move to behind the lever
        T_B_M__grinderLeverDepressed = util.mm(T_B_G, T_G_leverDepressed, util.inv(tools.T_TCP_grinderToolLever), util.inv(tools.T_M_TCP))
        robot.MoveJ(util.get_T2RDK(T_B_M__grinderLeverDepressed), blocking=True)

        rdk.pause(1)

        # Pull the lever
        T_B_M__grinderLeverPressed = util.mm(T_B_G, T_G_leverPressed, util.inv(tools.T_TCP_grinderToolLever), util.inv(tools.T_M_TCP))
        robot.MoveJ(util.get_T2RDK(T_B_M__grinderLeverPressed), blocking=True)

        rdk.pause(1)
