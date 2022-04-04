#include "Arm.h"
#include <Arduino.h>

void setup()
{
    Arm.initialise();
    delay(2000);

    while (true) {
        delay(5000);
        Arm.elbowPutDown();
        delay(1500);
        Arm.clawClose();
        Arm.elbowPickup();
        Arm.clawOpen();
    }
}

void loop()
{

}