#include <Arduino.h>
#include "Movement.h"
#include "Sensor.h"
#include "Arm.h"
#include "Brain.h"

Sensor irTopLeftSmall = Sensor(A1, IR_MEDIUM);
Sensor irTopRightSmall = Sensor(A3, IR_MEDIUM);
Sensor irBottomLeftMedium = Sensor(A0, IR_LONG);
Sensor irBottomRightMedium = Sensor(A2, IR_LONG);
Sensor irProximity = Sensor(45, IR_PROXIMITY);

void setup()
{
    Serial.begin(9600);
    Serial.print("Initialise");
    Serial.println();

    if (true) {
        Movement::initialise(false, false, false);
        Arm.initialise();
        Arm.backToStarting();
    }
    
    Brain.InitialiseStates();
    //delay(1000);
    Brain.sensorsTick(true);
    //delay(1000);
    //Movement::stop();
}

void loop()
{
    //Brain.sensorsTick(true);
    Brain.tick(true);
}
