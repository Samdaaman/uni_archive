"""
file: ik_solver.py
Author: Hank Wu 30 Oct 2021
"""
from math import atan, sqrt, atan2, pi, acos
import numpy


class ik_solver():
    """[summary]
    solve ik for a very specific case.
    """
    def __init__(self, x, y, l1, l2, cos_val) -> None:
        self.theta1_1 = 0.0
        self.theta1_2 = 0.0
        self.theta2_1 = 0.0
        self.theta2_2 = 0.0
        self.theta3_1 = 0.0
        self.theta3_2 = 0.0
        self.__calculate(x, y, l1, l2, cos_val)
    
    def __str__(self) -> str:
        my_str = ""
        my_str += f"theta1_1: {self.theta1_1:.1f}  \t"
        my_str += f"theta1_2: {self.theta1_2:.1f}\n"
        my_str += f"theta2_1: {self.theta2_1:.1f}  \t"
        my_str += f"theta2_2: {self.theta2_2:.1f}\n"
        my_str += f"theta3_1: {self.theta3_1:.1f}  \t"
        my_str += f"theta3_2: {self.theta3_2:.1f}\n"
        return my_str
        
    def __calculate(self, x, y, l1, l2, cos_val) -> None:
        #calculation:
        my_cos = (x**2 + y **2 - l1**2 - l2**2)/(2*l1*l2)
        my_sin = sqrt(1 - my_cos**2)
        self.theta2_1 = atan2(my_sin, my_cos) * 180 / pi
        self.theta2_2 = atan2(-my_sin, my_cos) * 180 / pi
        r = sqrt(x**2 + y**2)
        cos_phi = (l1**2 + r**2 - l2**2)/(2*l1*r)
        phi = acos(cos_phi) *180/pi
        beta = atan2(y,x)*180/pi
        self.theta1_1 = beta  - numpy.sign(self.theta2_1)*phi
        self.theta1_2 = beta  - numpy.sign(self.theta2_2)*phi
        beta1 = acos(cos_val) * 180/pi
        self.theta3_1 = beta1 - self.theta2_1 - self.theta1_1
        self.theta3_2 = beta1 - self.theta2_2 - self.theta1_2




if __name__ == "__main__":
    #one of the hw question
    x = 0.4254
    y = 0.2577
    l1 = 0.3
    l2 = 0.2
    cos_val = 0.4226
    ik = ik_solver(x, y, l1, l2, cos_val)
    print(ik)
    #2020 exam:
    x = 576.78
    y = 176.78
    l1 = 250
    l2 = 400
    cos_val = 0.7071
    ik = ik_solver(x, y, l1, l2, cos_val)
    print("2020 Exam Q2:")
    print(ik)

