from ultimateMatrix import tMatrix
from math import acos, pi, sqrt, cos, sin, atan2
# print("2020 Exam")
# t1 = tMatrix("th1", 0, 0, 0).get_matrix()
# t2 = tMatrix(0, "d2", 90, 0).get_matrix()
# t3 = tMatrix("th3", 0, 90, "cos(45)a").get_matrix()
# t4 = tMatrix("th4", 0, 0, 0).get_matrix()
# print(t1)
# print(t2)
# print(t3)
# print(t4)

# overall = t1.multiply(t2).multiply(t3).multiply(t4)
# print(overall)
i ="str"
len(i)

t1 = tMatrix("th1", 1, 90, 0)
t2 = tMatrix("th2", 0, 0, 1)
t3 = tMatrix("th3", 0, 0, 0)
t4 = tMatrix(0, 0, 0, 0.5)
print(t1)
print(t2)
print(t3)
print(t4)

overall = t1*t2*t3*t4
print(overall)


x = 0.8062
y = 0.8062
z = 1.9570
r = sqrt(x**2 + y**2)
l1 = 1
l2 = 1
l3 = 0.5
h = z - l1
rr = sqrt(r**2 + h**2)
costh3 = ((rr**2 - l2**2 - l3**2) / (2*l2*l3))
sinth3_1 = sqrt(1-costh3**2)
sinth3_2 = -sqrt(1-costh3**2)
th3_1 = atan2(sinth3_1, costh3) *180/pi
th3_2 = atan2(sinth3_2, costh3) *180/pi

print(th3_1)
# print(atan2(0.5, 0.8660)*180/pi)
print((h - 0.5*0.5)/l2)
print()