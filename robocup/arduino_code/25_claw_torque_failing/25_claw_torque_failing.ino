#include <Arduino.h>
#include "Movement.h"
#include "Sensor.h"
#include "Arm.h"
#include "Brain.h"

Sensor irTopLeftLong = Sensor(A1, IR_LONG);
Sensor irTopRightLong = Sensor(A3, IR_LONG);
Sensor irBottomLeftMedium = Sensor(A0, IR_MEDIUM);
Sensor irBottomRightMedium = Sensor(A2, IR_MEDIUM);
Sensor irProximity = Sensor(45, IR_PROXIMITY);

void setup()
{
    Serial.begin(9600);
    // Serial.print("Initialise");
    // Serial.println();

    Arm.initialise();
    Arm.backToStarting();
    delay(1000);
    Arm.elbowPutDown();
    while (true) {
        delay(2000);
        Arm.clawClose();
        delay(2000);
        Arm.clawOpen();
    }

    Brain.sensorsTick(true);

    if (true) {
        Movement::initialise(false, false, false);
        Arm.initialise();
        Arm.backToStarting();
        Brain.InitialiseStates();
    } else {
        while (1) {
            Brain.sensorsTick(false, false);
        }
    }
}

void loop()
{
    Brain.tick(true);
}
