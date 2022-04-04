from math import pi, sqrt, atan2, cos, sin


class odomCalculator():
    def __init__(self, odompose1: list, odompose2: list, robotpose: list) -> None:
        self.odomPose1 = odompose1
        self.odomPose2 = odompose2
        self.robotPose = robotpose
        self.rot1 = 0
        self.rot2 = 0
        self.trans = 0
        self.calculate()

    def __str__(self) -> str:
        string = f"rot1 = {self.rot1*180/pi:.1f} degree, trans = {self.trans:.1f}, rot2 = {self.rot2*180/pi:.1f} degree\n"
        string += f"Robot's new pose: x = {self.robotPose[0]:.1f}, y = {self.robotPose[1]:.1f} , theta = {self.robotPose[2]*180/pi:.1f} degree"
        return string


    def calculate(self):
        theta = self.odomPose2[2]
        theta_prev = self.odomPose1[2]
        x_prev = self.odomPose1[0]
        y_prev = self.odomPose1[1]
        dx = self.odomPose2[0] - self.odomPose1[0]
        dy = self.odomPose2[1] - self.odomPose1[1]
        dtheta = theta - theta_prev
        # use the rot->trans->rot scheme.
        self.trans = sqrt(dx**2+dy**2)
        self.rot1 = atan2(dy, dx) - theta_prev
        self.rot2 = dtheta - self.rot1
        self.robotPose[0] +=  self.trans*cos(self.robotPose[2]+self.rot1)
        self.robotPose[1] +=  self.trans*sin(self.robotPose[2]+self.rot1)
        self.robotPose[2] +=  self.rot1 + self.rot2


    
if __name__ == "__main__":
    odomPose1 = [1,0,0]
    odomPose2 = [2,0,0]
    robotPose = [6,1,pi/2]
    cal = odomCalculator(odomPose1,odomPose2,robotPose)
    print (cal)

    odomPose1 = [0,1,0]
    odomPose2 = [4,4,0]
    robotPose = [0,0,0]
    cal = odomCalculator(odomPose1,odomPose2,robotPose)
    print (cal)