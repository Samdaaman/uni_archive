# First Attempt at Gains
## LQR Methodology (see pendulum_v04.m file)
The aim of the LQR-based gains was to control the pendulum with as little controller effort as possible. The maximum rising time of 1-3 seconds was the first parameter that was designed for. A conservative limit of 2 seconds maximum rise time was aimed for because it allowed for parameter uncertainity and non-linear dynamics of the system (such as backlash). Using an ItErAtIVE process the 4 gains (one for each system) were designed with Q = [30, 0....0] and R = [1]. This was the minimum value of Q(1,1) (least controller effort) that satisfied the 2 second maximum rise time limit. 

### Results
```
Optimised gains for setup #1 are:
   -5.4772  -20.4428   -9.1631   -4.2073

Optimised gains for setup #2 are:
   -5.4772  -24.5829   -9.5415   -5.8829

Optimised gains for setup #3 are:
   -5.4772  -25.3136   -9.6031   -6.1870

Optimised gains for setup #4 are:
   -5.4772  -25.8797   -9.6491   -6.4199
```

### Final Choices
The final gains were decided by taking a weighted average of the mean and median of each set of 4 gains:  
```Final LQR Gains: -5.48, -24.5, -9.55, -6.00```
N=-5.48

---

## Pole Placement Methodology (pendulum_v05.m file)
The aim of the gains derived from pole placement was to operate at the limits of controller performance (and theoretically "fastest" control). 

From the LQR method above it was found that setup #3 was a good representation of all the setups as it's calculated gains closely matched the final gains.

Using an iterative process, it was found that poles that were "separated a lot" required lots of controller effort (to make one large pole and some smaller ones) so clusted poles was aimed for. It was found that poles of ```p = [-8, -8.1, -8.2, -8.3];``` which corresponded to **gains of ```K = [-21.3, -32.5, -13.8, -7.76]```**  achieved a considerable performance improvement whilst still being within the slew rate limit for 3/4 setups. The 1/4 setup (setup #1) which breached the S/R limit did so with the first 0.2 seconds and because this was sufficiently far away from settingly it was deemed that instability was unlikely to occur.

The pole placement methodology found similiar (but notably higher) gains to the LQR method which is expected as that PP method focused on performance rather than controller effort so has increased gains.


# Second Attempt at Gains
## LQR #1 (pendulum_v04) - low performance
Using a "weak" (small) Q matrix to optimise for least controller effort which meets the design requirements. This does not overshoot and "is a bit slow"  
```Final LQR Gains: K=[-5.48, -24.5, -9.55, -6.00] N=-5.48```

## LQR #2 (pendulum_v04b) - high performance
Using a larger Q matrix to optimise for performance we wanted to aim for under 1 second settling time with overshoot 
Optimised gains for setup #3 are:  
```K = [-70.7107 -109.1248  -43.0149  -26.8460]  N=-70.7107```

## Hinf EXTREMLY MILD (pendulum_v06)
Noticed that it appeared to "respond quicker" intially which is good because it needs to react in time before the pendulum falls
It also had lots of damping immediately after the initial first response
Note: stopped simulation after second Hinf iteration because this was lowest gamma and used a 5x5x5x5 grid
Tis was based off the high performance LQR gains  
```K = [-84.85284 -115.2631 -48.12292 -21.4768] N = -84.8528```

## Hinf PRETTY MILD (pendulum_v06)
As above but based of the low performance LQR gains  
```K = [-7.3067  -18.8039   -8.3562   -4.0000] N = -7.3067```
