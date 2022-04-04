"""
file: forward_kinematic.py
Author: Hank Wu 30 Oct 2021
"""
from ultimateMatrix import tMatrix





if __name__ == "__main__":
    theta = "theta1"
    d = 0
    alpha = 90
    a = "L1"
    h1 = tMatrix(theta, d, alpha, a)
    theta = "theta2"
    d = 0
    alpha = 0
    a = "L2"
    h2 = tMatrix(theta, d, alpha, a)
    theta = "theta3"
    d = 0
    alpha = 0
    a = 0
    h3 = tMatrix(theta, d, alpha, a)
    print(h1)
    print(h2)
    print(h3)
    print(h1*h2*h3)


    theta = "theta_i"
    d = "d_i"
    alpha = "alpha_i"
    a = "a_i"
    dh_mat = tMatrix(theta, d, alpha, a)
    print(dh_mat)
    #this is the correct answer but without applying trigonometry trick.
    #Note chris made an typo in the solution for this HW question.


