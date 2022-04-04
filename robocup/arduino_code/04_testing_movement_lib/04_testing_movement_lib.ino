#include <Arduino.h>
#include "Movement.h"
#include <Servo.h>

const bool SWAP_MOTORS = false;
const bool SWAP_LEFT_POLARITY = false;
const bool SWAP_RIGHT_POLARITY = false;

void setup()
{
	Movement::initialise(SWAP_MOTORS, SWAP_LEFT_POLARITY, SWAP_RIGHT_POLARITY);
    Movement::test();
}

void loop()
{
}
