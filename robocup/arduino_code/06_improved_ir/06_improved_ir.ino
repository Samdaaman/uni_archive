#include <Arduino.h>
#include <Servo.h>
#include "Movement.h"
#include "Sensor.h"

const bool SWAP_MOTORS = false;
const bool SWAP_LEFT_POLARITY = false;
const bool SWAP_RIGHT_POLARITY = false;

Sensor topSensor = Sensor(A0, IR_SMALL_TOP);
Sensor bottomSensor = Sensor(A1, IR_SMALL_BOTTOM);

enum State {INITIAL, SCANNING_LEFT, SCANNING_RIGHT, MOVING_ON_LEFT, MOVING_ON_RIGHT, STOPPEDD};
State state = INITIAL;

int weightStateCounter = 0;
const int STATE_CHANGE_THRESHOLD = 5;
bool foundWeight = false;

void setup()
{
	Movement::initialise(SWAP_MOTORS, SWAP_LEFT_POLARITY, SWAP_RIGHT_POLARITY);
    //Movement::test();
    Serial.begin(9600);
    Serial.print("Initialise");
}

void loop()
{
    Reading topReading = topSensor.getReading();
    Reading bottomReading = bottomSensor.getReading();
    if (bottomReading.foundObject && bottomReading.processedValue - 40 > topReading.processedValue)
    {
        if (!foundWeight)
        {
            weightStateCounter ++;
            if (weightStateCounter == STATE_CHANGE_THRESHOLD)
            {
                foundWeight = true;
                weightStateCounter = 0;
            }
        }
        else
        {
            weightStateCounter = 0;
        }
    }
    else
    {
        if (foundWeight)
        {
            weightStateCounter ++;
            if (weightStateCounter == STATE_CHANGE_THRESHOLD)
            {
                foundWeight = false;
                weightStateCounter = 0;
            }
        }
        else
        {
            weightStateCounter = 0;
        }
        
    }
    
    
    if (bottomReading.processedValue >= 575)
    {
        state = STOPPEDD;
        Movement::stop();
    }
    
    Serial.print(foundWeight);
    Serial.print(",");
    Serial.print(topReading.processedValue);
    Serial.print(",");
    Serial.print(bottomReading.processedValue);
    Serial.print(",");
    Serial.print(state);
    Serial.println();
    delay(10);

    switch (state)
    {
    case INITIAL:
        state = SCANNING_LEFT;
        Movement::spin(SLOW, LEFT);
        break;
    case SCANNING_LEFT:
        if (foundWeight)
        {
            state = MOVING_ON_LEFT;
            Movement::slightTurn(MEDIUM, FORWARD, LEFT);
        }
        break;
    case SCANNING_RIGHT:
        if (foundWeight)
        {
            state = MOVING_ON_RIGHT;
            Movement::slightTurn(MEDIUM, FORWARD, RIGHT);
        }
        break;
    case MOVING_ON_LEFT:
        if (!foundWeight)
        {
            state = SCANNING_RIGHT;
            Movement::spin(SLOW, RIGHT);
        }
        break;
    case MOVING_ON_RIGHT:
        if (!foundWeight)
        {
            state = SCANNING_LEFT;
            Movement::spin(SLOW, LEFT);
        }
        break;
    case STOPPEDD:
        return;
    default:
        break;
    }
    
}
