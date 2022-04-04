# Project Requirements
**This is not an official handout. It is a list of deliverables derrived from the Project Overview and Scope document.**

FreeRTOS tasks running on a Tiva MCU will allow a user to control the yaw and height of a HeliRig emulator using a closed control loop. Both takeoff and yaw or height control must be demonstrated. At least one mutex, semaphore, and queue must be used. 

1. **Emulator Interface** The Tiva MCU will interface with the HeliRig and emulator. 
    - The height feedback data is an analogue interface varying between 1 and 2 V. 
    - The Yaw feedback data is communicated using quadrature encoding.
    - Both the main and tail rotors will be controlled by PWM signals, varying between 2 and 98% duty cycle.
<br><br>

1. **Data Filtering** The feedback data will be scaled, buffered and filtered. 

1. **Control Algorithm** A PI or PID control algorithm will control the height and yaw of the HeliRig. 

1. **User Interface** The user will be able to view and control the yaw and height set points. 
    - The Tiva Display will show the height and the yaw angle.
    - The four buttons on the Tiva MCU (UP, DOWN, CW, CCW) will be used to change the height and yaw set points.
<br><br>

1. **Serial Debug** Debug information will be reported using the Tiva MCU's serial port.

1. **FreeRTOS** The scheduler used for this project will be FreeRTOS. Tasks should be modularised and prioritised sensibly. This should be discussed in the report.
