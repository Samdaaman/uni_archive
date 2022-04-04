from ultimateMatrix import tMatrix
from math import pi
print("2020 Exam")
t1 = tMatrix("Hank", 0, 0, 250)
t2 = tMatrix("Jeff", 0, 0, 400)
t3 = tMatrix(0, "Henry", 0, 0)
t4 = tMatrix("Zach", 0, 0, 0)
print(t1)
print(t2)
print(t3)
print(t4)

overall = t1*t2*t3*t4
print(overall)

print("4.0 jacobian HW")
t1 = tMatrix("theta1", "L1", 90, 0)
t2 = tMatrix("theta2", 0, -90, 0)
t3 = tMatrix("theta3", "L2", 0, 0)

print(t1)
print(t2)
print(t3)


overall = t1*t2*t3
print(t1*t2)
print(overall)

print("4.0 jacobian HW 2")
t1 = tMatrix("Jason", 0, 0, "Zach")
t2 = tMatrix("Randy", 0, 0, "Sam")


print(t1)
print(t2)


print(t1*t2)


print("2020 Exam")
t1 = tMatrix(-10.92, 0, 0, 250)
t2 = tMatrix(45, 0, 0, 400)
t3 = tMatrix(0, 100, 0, 0)
t4 = tMatrix(10.92, 0, 0, 0)

overall = t1*t2*t3*t4
overall.precision = 4
print(overall)


print("2020 Exam")
t1 = tMatrix(45, 0, 0, 250)
t2 = tMatrix(-45, 0, 0, 400)
t3 = tMatrix(0, 100, 0, 0)
t4 = tMatrix(45, 0, 0, 0)
overall = t1*t2*t3*t4
print(overall)

t1 = tMatrix("theta1", 0, 0, "L1")
t2 = tMatrix("theta2", 0, 0, "L2")

print(t1)
print(t2)

overall = t1*t2
overall.precision = 4
print(overall)


print("2016 Exam Q1")
t1 = tMatrix("theta1", 0, 0, 0)
t2 = tMatrix(0, "d2+a2*sin(30)", 90, "a2*cos(30)")
t3 = tMatrix("theta3", 0, 90, "a3")
t4 = tMatrix("theta4", 0, 0, 0)
overall = t1*t2*t3*t4
print(t1)
print(t2)
print(t3)
print(t4)
print(overall)


print("2019 Exam Q2")
t1 = tMatrix("theta1", "L1", 90, 0)
t2 = tMatrix("theta2", 0,0,"L2")
t3 = tMatrix("theta3", 0,0,0)
t4 = tMatrix(0,0,0,"L3")
overall = t1*t2*t3
print(t1)
print(t2)
print(t3)
print(overall)


# ik example:
print("2019 exam")
t1 = tMatrix("theta1", 1, 90 ,0)
t2 = tMatrix("theta2", 0, 0 , 1)
t3 = tMatrix("theta3", 0, 0, 0)
t4 = tMatrix(0, 0, 0, 0.5)
fk: tMatrix = t1*t2*t3*t4

target = [
[0.612,     -0.354,     0.707 ,     0.806],     
[0.612,     -0.354,     -0.707,     0.806],     
[0.500,     0.866 ,     0.0   ,     1.957],     
[0    ,     0     ,     0     ,     1    ]]

ik = fk.ik(target)
for key in ik:
    if f"{key}".find("theta") == -1:
        print(f"{key} = {ik[key]:.2f}")
    else:
        print(f"{key} = {ik[key]*180/pi:.2f}")




print("2020 exam")
t1 = tMatrix("theta1", 0, 0 ,250)
t2 = tMatrix("theta2", 0, 0 , 400)
t3 = tMatrix(0, "d3", 0, 0)
t4 = tMatrix("theta4", 0, 0, 0.5)
fk: tMatrix = t1*t2*t3*t4

target = [
[0.7071,     -0.7071,     0 ,    576.78],     
[0.7071,     0.7071,     0,     176.78],     
[0,     0 ,     1   ,     100],     
[0    ,     0     ,     0     ,     1    ]]

ik = fk.ik(target)
for key in ik:
    if f"{key}".find("theta") == -1:
        print(f"{key} = {ik[key]:.2f}")
    else:
        print(f"{key} = {ik[key]*180/pi:.2f}")
