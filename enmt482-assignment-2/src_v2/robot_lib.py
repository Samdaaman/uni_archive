import robodk as rdk
from robolink.robolink import Robolink  # Robolink API

import time
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

cup_tool_open = "Cup Tool Open" #RDK.Item("Cup Tool Open")
cup_tool_close = "Cup Tool Close"


# # OBSELETE SEE better_wait_finished() Call this after running subprogram
# def reset_robot_frame():
#     robot.setPoseFrame(world_frame) # Reset frame to base frame (B)
#     robot.setPoseTool(robot.PoseTool()) # Reset tool frame


def goto_global_home():
    robot.MoveJ(home, blocking=True)


def run_program_cup_tool_open():
    RDK.RunProgram(cup_tool_open, True)
    better_wait_finished()  # to allow subprogram to complete


def run_program_cup_tool_close():
    RDK.RunProgram(cup_tool_close, True)
    better_wait_finished()  # to allow subprogram to complete


def better_wait_finished():
    """
    Better version of robot.WaitFinished(). Waits for the robot to finish it's program (and stay finished after checking again after 1 sec\n
    I believe the "Pause" instructions in the tool subprograms cause the robot to say it isn't busy when really it is in the middle of a subprogram. This causes the original robot.WaitFinished() to return ealier than expected.\n
    The solution was to call WaitFinished() and then recheck the value after 1 second (scaled by simulation speed) 
    """
    while True:
        robot.WaitFinished()
        sim_scale_capped_at_1 = min(1, RDK.SimulationSpeed())  # cap simulation scale so we never wait shorter times (for safety)
        time.sleep(0.6 / sim_scale_capped_at_1)  # Wait 600ms (just a bit more than 500ms), scaled by simulation speed if the sim is running slower than realtime
        if not robot.Busy():
            robot.setPoseFrame(world_frame) # Reset frame to base frame (B)
            robot.setPoseTool(robot.PoseTool()) # Reset tool frame
            return