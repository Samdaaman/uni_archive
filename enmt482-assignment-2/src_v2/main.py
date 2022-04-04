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


import robodk as rdk  # Robot toolbox

import cup_stand
import expresso_machine
import grinder
import robot_lib
import tools
import tool_stand
import util

robot = robot_lib.robot


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


#_____________________________________________________________________________________
#                                                                                     #
#                                     PROGRAM TASKS                                   #
#                                                                                     #
#_____________________________________________________________________________________#

def main():
    # Start from home position
    robot.MoveJ(robot_lib.home, blocking=True)

    # Steps A-C including picking portafilter up from grinder
    grinder.routine()

    # D.2 Move to tamper and scrape coffee from the rim of the filter tool


    # E.1 Tamp the coffee


    # F.1 Get portafilter tool to the group head test point
    # F.2 Insert into coffee machine and rotat 45deg
    # F.3 Move portafilter tool a short distance and pause for 5s
    tool_stand.insert_portafilter_in_group_head_and_release()
    tools.get_filter_tool(is_pickup=False)# REMOVE FOR ACTUAL THING

    # G.1 Get the coffee cup tool
    tools.get_cup_tool(is_pickup=True)

    # G.2 Pick up coffee cup with cup tool
    cup_stand.pickup_coffee_cup_from_stand()

    # H.1 Place coffee cup in drip tray of the coffee machine under the group head
    expresso_machine.dropoff_cup_at_espresso()

    # I.1 Put back the cup tool
    tools.get_cup_tool(is_pickup=False)

    # I.2 Get the grinder tool
    tools.get_grinder_tool(is_pickup=True)

    # I.3 Move to the espresso button 1 and turn the espresso on for 3 seconds
    expresso_machine.press_espresso_button()
    
    # J.1 Put back the grinder tool
    tools.get_grinder_tool(is_pickup=False)

    # J.2 Get the cup tool
    tools.get_cup_tool(is_pickup=True)

    # J.3 Move to the coffee machine and pick up the cup of coffee
    expresso_machine.pickup_cup_at_espresso()

    # J.4 Place steaming cup of coffee upright on the cup stand
    # J.5 Release coffee cup and put back the cup tool
    cup_stand.dropoff_coffee_cup_on_stand() 
    
    # J.6 Return Cup tool
    tools.get_cup_tool(is_pickup=False)
    
    # J.7 Return Home
    robot.MoveJ(robot_lib.home, blocking=True)


# def tool_test_routine():
#     # Start at home
#     robot.MoveJ(T_home, blocking=True)

#     # A.1 Get filter tool
#     get_filter_tool(is_pickup=True)
#     get_filter_tool(is_pickup=False)

#     get_grinder_tool(is_pickup=True)
#     get_grinder_tool(is_pickup=False)

#     get_cup_tool(is_pickup=True)
#     get_cup_tool(is_pickup=False)


# def test_routine():
#     # Start at home
#     robot.MoveJ(T_home, blocking=True)
    
#     get_cup_tool(is_pickup=True)


if __name__ == '__main__':
    # Execute program
    util.debug_enable()
    # main()
    robot.MoveJ(robot_lib.home, blocking=True)

    # Testing grinder
    grinder.routine()
