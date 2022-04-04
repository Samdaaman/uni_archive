# Ultimate matrix

If you want someone to be added to the repo please pm hank~

Contributors:

- Hank Wu
- Sam Hogan

Please share your useful python tools for tackling the exam!

use `ctrl+shift+b` in vscode to auto-configure the python environment.(tested on MacOS)

The altimate matrix convert DH parameters into a manipulatable matrix!!! 

```python
from ultimateMatrix import tMatrix
theta = "theta_i"
d = "d_i"
alpha = "alpha_i"
a = "a_i"
dh_mat = tMatrix(theta, d, alpha, a)
print(dh_mat)
```

```bash
_______________________________uMatrix______________________________

cos(θᵢ)     -sin(θᵢ)⋅cos(αᵢ)     sin(αᵢ)⋅sin(θᵢ)      aᵢ⋅cos(θᵢ)     
sin(θᵢ)     cos(αᵢ)⋅cos(θᵢ)      -sin(αᵢ)⋅cos(θᵢ)     aᵢ⋅sin(θᵢ)     
0           sin(αᵢ)              cos(αᵢ)              dᵢ             
0           0                    0                    1              
_____________________________________________________________________
```

It supports symbolic expressions and it deal with trigonometry identities automatically

*the output is guaranteed correct!*

Usage: Numeric + symbolic expressions:

```python
from ultimateMatrix import tMatrix
t1 = tMatrix("Hank", 0, 0, 250)
t2 = tMatrix("Jeff", 0, 0, 400)
t3 = tMatrix(0, "Henry", 0, 0)
t4 = tMatrix("Zach", 0, 0, 0)
overall = t1*t2*t3*t4
print(overall)
```

output:

```bash
_________________________________________________uMatrix________________________________________________

cos(Hank + Jeff + Zach)     -sin(Hank + Jeff + Zach)     0     250⋅cos(Hank) + 400⋅cos(Hank + Jeff)     
sin(Hank + Jeff + Zach)     cos(Hank + Jeff + Zach)      0     250⋅sin(Hank) + 400⋅sin(Hank + Jeff)     
0                           0                            1     Henry                                    
0                           0                            0     1                                        
_________________________________________________________________________________________________________
```

usage: numeric expression:
```python
from ultimateMatrix import tMatrix
t1 = tMatrix(-10.92, 0, 0, 250)
t2 = tMatrix(45, 0, 0, 400)
t3 = tMatrix(0, 100, 0, 0)
t4 = tMatrix(10.92, 0, 0, 0)

overall = t1*t2*t3*t4
overall.precision = 4 #specify how many digits want to print default is 3
print(overall)
```
output: 
```bash
2020 Exam
__________________uMatrix_________________

0.7071     -0.7071     0     576.7756     
0.7071     0.7071      0     176.7804     
0          0           1     100          
0          0           0     1            
___________________________________________
```


A more complex example:

```python
from ultimateMatrix import tMatrix
print("2016 Exam Q1")
t1 = tMatrix("theta1", 0, 0, 0)
t2 = tMatrix(0, "d2+a2*sin(30)", 90, "a2*cos(30)")
t3 = tMatrix("theta3", 0, 90, "a3")
t4 = tMatrix("theta4", 0, 0, 0)
overall = t1*t2*t3*t4
print(overall)
```

output:

It is printed out in a nicer format!!

```bash
______________________uMatrix_______________________________

row 0, col0: sin(θ₁)⋅sin(θ₄) + cos(θ₁)⋅cos(θ₃)⋅cos(θ₄)
row 0, col1: sin(θ₁)⋅cos(θ₄) - sin(θ₄)⋅cos(θ₁)⋅cos(θ₃)
row 0, col2: sin(θ₃)⋅cos(θ₁)
row 0, col3: a₂⋅cos(30)⋅cos(θ₁) + a₃⋅cos(θ₁)⋅cos(θ₃)

row 1, col0: sin(θ₁)⋅cos(θ₃)⋅cos(θ₄) - sin(θ₄)⋅cos(θ₁)
row 1, col1: -sin(θ₁)⋅sin(θ₄)⋅cos(θ₃) - cos(θ₁)⋅cos(θ₄)
row 1, col2: sin(θ₁)⋅sin(θ₃)
row 1, col3: a₂⋅sin(θ₁)⋅cos(30) + a₃⋅sin(θ₁)⋅cos(θ₃)

row 2, col0: sin(θ₃)⋅cos(θ₄)
row 2, col1: -sin(θ₃)⋅sin(θ₄)
row 2, col2: -cos(θ₃)
row 2, col3: a₂⋅sin(30) + a₃⋅sin(θ₃) + d₂

row 3, col0: 0
row 3, col1: 0
row 3, col2: 0
row 3, col3: 1

____________________________________________________________
```
# Inverse kinematics:

This package is able to solve inverse kinematic.

usage:

```python
from ultimateMatrix import tMatrix
from math import pi

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

```

result: 

```bash
theta1 = 45.00
theta2 = 45.00
theta3 = -14.98
```

# Jacobian matrix

credit: Sam Hogan


```python
print("2019 Exam Q2b")
t0 = tMatrix(0, 0, 0, 0)
t1 = tMatrix("theta1", 1, 90, 0)
t2 = tMatrix("theta2", 0,0,1)
t3 = tMatrix("theta3", 0,0,0)
# t4 = tMatrix(0,0,0,"L3")
overall = t1*t2*t3
print(overall)

jacobian = jacobian_god_function([t0, t1, t2, t3], (False, False, False))
```


```bash
2019 Exam Q2b
_______________________________________uMatrix______________________________________

cos(θ₁)⋅cos(θ₂ + θ₃)     -sin(θ₂ + θ₃)⋅cos(θ₁)     sin(θ₁)      cos(θ₁)⋅cos(θ₂)     
sin(θ₁)⋅cos(θ₂ + θ₃)     -sin(θ₁)⋅sin(θ₂ + θ₃)     -cos(θ₁)     sin(θ₁)⋅cos(θ₂)     
sin(θ₂ + θ₃)             cos(θ₂ + θ₃)              0.0          sin(θ₂) + 1         
0                        0                         0            1                   
____________________________________________________________________________________

i =  0 __________________________________________________________________________________________
T0   =
_________uMatrix________

1     0     0     0     
0     1     0     0     
0     0     1     0     
0     0     0     1     
_________________________

Z0   = 
			--vector--
			0
			0
			1
			----------

P0   = 
			--vector--
			0
			0
			0
			----------

r0   = 
			--vector--
			cos(θ₁)⋅cos(θ₂)
			sin(θ₁)⋅cos(θ₂)
			sin(θ₂) + 1
			----------

zxr0 = 
			--vector--
			-sin(θ₁)⋅cos(θ₂)
			cos(θ₁)⋅cos(θ₂)
			0
			----------

col0 = 
			--vector--
			-sin(θ₁)⋅cos(θ₂)
			cos(θ₁)⋅cos(θ₂)
			0
			0
			0
			1
			----------

____________________________________________________________________________________________________
i =  1 __________________________________________________________________________________________
T1   =
_______________uMatrix______________

cos(θ₁)     0     sin(θ₁)      0     
sin(θ₁)     0     -cos(θ₁)     0     
0           1     0.0          1     
0           0     0            1     
_____________________________________

Z1   = 
			--vector--
			sin(θ₁)
			-cos(θ₁)
			0.0
			----------

P1   = 
			--vector--
			0
			0
			1
			----------

r1   = 
			--vector--
			cos(θ₁)⋅cos(θ₂)
			sin(θ₁)⋅cos(θ₂)
			sin(θ₂)
			----------

zxr1 = 
			--vector--
			-sin(θ₂)⋅cos(θ₁)
			-sin(θ₁)⋅sin(θ₂)
			cos(θ₂)
			----------

col1 = 
			--vector--
			-sin(θ₂)⋅cos(θ₁)
			-sin(θ₁)⋅sin(θ₂)
			cos(θ₂)
			sin(θ₁)
			-cos(θ₁)
			0.0
			----------

____________________________________________________________________________________________________
i =  2 __________________________________________________________________________________________
T2   =
__________________________________uMatrix_________________________________

cos(θ₁)⋅cos(θ₂)     -sin(θ₂)⋅cos(θ₁)     sin(θ₁)      cos(θ₁)⋅cos(θ₂)     
sin(θ₁)⋅cos(θ₂)     -sin(θ₁)⋅sin(θ₂)     -cos(θ₁)     sin(θ₁)⋅cos(θ₂)     
sin(θ₂)             cos(θ₂)              0.0          sin(θ₂) + 1         
0                   0                    0            1                   
__________________________________________________________________________

Z2   = 
			--vector--
			sin(θ₁)
			-cos(θ₁)
			0.0
			----------

P2   = 
			--vector--
			cos(θ₁)⋅cos(θ₂)
			sin(θ₁)⋅cos(θ₂)
			sin(θ₂) + 1
			----------

r2   = 
			--vector--
			0
			0
			0
			----------

zxr2 = 
			--vector--
			0
			0
			0
			----------

col2 = 
			--vector--
			0
			0
			0
			sin(θ₁)
			-cos(θ₁)
			0.0
			----------

____________________________________________________________________________________________________
====================================================================================================
JACOBIAN BELOW
====================================================================================================
-sin(θ₁)⋅cos(θ₂)               | -sin(θ₂)⋅cos(θ₁)               | 0                             
cos(θ₁)⋅cos(θ₂)                | -sin(θ₁)⋅sin(θ₂)               | 0                             
0                              | cos(θ₂)                        | 0                             
0                              | sin(θ₁)                        | sin(θ₁)                       
0                              | -cos(θ₁)                       | -cos(θ₁)                      
1                              | 0                              | 0                             

```