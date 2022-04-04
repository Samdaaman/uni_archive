#include "Arm.h"
#include "Movement.h"
#include "Arduino.h"
#include "Switch.h"


void setup() {
    Movement::initialise(false, false, false);
    Arm.initialise();
    Arm.backToStarting();
    Arm.elbowPutDown();
    waitForSwitch();
    Arm.clawClose();

    delay(200);

    Movement::together(FAST, REVERSE);
    delay(200);
    Arm.yeet();
    Movement::stop();
}

void loop() {}