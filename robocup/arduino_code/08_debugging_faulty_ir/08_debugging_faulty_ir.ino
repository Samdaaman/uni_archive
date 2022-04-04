#include <Arduino.h>
#include <Servo.h>
#include "Movement.h"
#include "Sensor.h"

const bool SWAP_MOTORS = false;
const bool SWAP_LEFT_POLARITY = false;
const bool SWAP_RIGHT_POLARITY = false;

Sensor topSensor = Sensor(A0, IR_SMALL_TOP);
Sensor bottomSensor = Sensor(A2, IR_SMALL_BOTTOM);

void setup()
{
	Movement::initialise(SWAP_MOTORS, SWAP_LEFT_POLARITY, SWAP_RIGHT_POLARITY);
    Serial.begin(9600);
    Serial.print("Initialise");
}

void loop()
{
    Reading topReading = topSensor.getReading();
    Reading bottomReading = bottomSensor.getReading();
    
    Serial.print(topReading.processedValue);
    //Serial.print(",");
    //Serial.print(650);
    Serial.println();
    delay(10);
}
