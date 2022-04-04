#include <Arduino.h>
#include "Movement.h"
#include "Sensor.h"
#include "Arm.h"
#include "Brain.h"
#include "Switch.h"

#define SENSORS_ONLY false

Sensor irTopLeftLong = Sensor(A1, IR_LONG);
Sensor irTopRightLong = Sensor(A3, IR_LONG);
Sensor irBottomLeftMedium = Sensor(A0, IR_MEDIUM);
Sensor irBottomRightMedium = Sensor(A2, IR_MEDIUM);
Sensor irProximity = Sensor(33, IR_PROXIMITY);

void setup()
{
    Serial.begin(9600);
    // Serial.print("Initialise");
    // Serial.println();

    Brain.sensorsTick(true);

    if (!SENSORS_ONLY) {
        Movement::initialise(false, false, false);
        Arm.initialise();
        Arm.backToStarting();

        waitForSwitch();
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
