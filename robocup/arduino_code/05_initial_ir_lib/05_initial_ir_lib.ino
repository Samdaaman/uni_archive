#include <Arduino.h>
#include <Servo.h>
#include "Movement.h"
#include "Sensor.h"

const bool SWAP_MOTORS = true;
const bool SWAP_LEFT_POLARITY = true;
const bool SWAP_RIGHT_POLARITY = true;

Sensor topSensor = Sensor(A0, IR_SMALL);
Sensor bottomSensor = Sensor(A1, IR_SMALL);

int foundWeightCounter = 0;
Side turningSide = LEFT;

void setup()
{
	Movement::initialise(SWAP_MOTORS, SWAP_LEFT_POLARITY, SWAP_RIGHT_POLARITY);
    Movement::stop();
    //Movement::test();
    Serial.begin(9600);
    Serial.print("Initialise");
    Serial.println();
    delay(5000);
    Serial.print("Going");
    Movement::spin(SLOW, turningSide);
}

void loop()
{
    Reading topReading = topSensor.getReading();
    Reading bottomReading = bottomSensor.getReading();
    bool foundWeight = bottomReading.foundObject && !topReading.foundObject;
    if (foundWeight)
    {
        foundWeightCounter ++;
    }
    else
    {
        foundWeightCounter = 0;
    }
    
    Serial.print(foundWeight);
    Serial.print(",");
    Serial.print(topReading.rawValue);
    Serial.print(",");
    Serial.print(bottomReading.rawValue);
    if (foundWeightCounter == 3)
    {
        foundWeightCounter = 0;
        Serial.print("Found weight");
        Movement::stop();
        delay(2000);
        turningSide = (Side)-turningSide;
        Movement::spin(SLOW, turningSide);
    }
    Serial.println();
    delay(10);
}
