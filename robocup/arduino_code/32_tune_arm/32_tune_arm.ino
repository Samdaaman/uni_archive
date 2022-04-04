#include <Arduino.h>
#include "Movement.h"
#include "Sensor.h"
#include "Arm.h"
#include "Brain.h"
#include "Switch.h"
#include "Accelerometer.h"

#define SENSORS_ONLY false

Sensor irTopLeftLong = Sensor(A3, IR_LONG);
Sensor irTopRightLong = Sensor(A1, IR_LONG);
Sensor irBottomLeftMedium = Sensor(A2, IR_MEDIUM);
Sensor irBottomRightMedium = Sensor(A0, IR_MEDIUM);
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
        Accelerometer.initialise();
        waitForSwitch();
        Brain.InitialiseStates();
    } else {
        while (1) {
            Brain.sensorsTick(false, false);
            // Brain.sensorsTick();

            // static bool first = true;
            // if (first) {
            //     Serial.println("FL,TL,FR,TR");
            // }
            // Serial.print(topLeftReading.foundObject*600);
            // Serial.print(",");
            // Serial.print(topLeftReading.distanceMM);
            // Serial.print(",");
            // Serial.print(topRightReading.foundObject*500);
            // Serial.print(",");
            // Serial.print(topRightReading.distanceMM);
            // Serial.println();
        }
    }
}

void loop()
{
    Brain.tick(true);
}
