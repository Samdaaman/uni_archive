#include <Arduino.h>
#include "Movement.h"
#include "Sensor.h"

const bool SWAP_MOTORS = false;
const bool SWAP_LEFT_POLARITY = false;
const bool SWAP_RIGHT_POLARITY = false;



enum State {SCANNING_LEFT, SCANNING_RIGHT, TURNING_LEFT, TURNING_RIGHT, SSTOPPED};
State state = SCANNING_LEFT;

enum Vision {FOUND_NONE, FOUND_OBSTACLE, FOUND_LEFT, FOUND_RIGHT};
Vision visionProtential = FOUND_NONE;
Vision visionCurrent = FOUND_NONE;
int visionCounter = 0;

Sensor irTopLeftSmall = Sensor(A0, IR_MEDIUM);
Sensor irTopRightSmall = Sensor(A1, IR_MEDIUM);
Sensor irBottomLeftMedium = Sensor(A2, IR_LONG);
Sensor irBottomRightMedium = Sensor(A3, IR_LONG);

void setup()
{
    Movement::initialise(SWAP_MOTORS, SWAP_LEFT_POLARITY, SWAP_RIGHT_POLARITY);
    Serial.begin(9600);
    Serial.print("Initialise");
    Serial.println();

    delay(2000);
    Movement::spin(SLOW, LEFT);
}

void loop()
{
    if (state == SSTOPPED) {
        return;
    }

    Reading topLeftReading = irTopLeftSmall.getReading();
    Reading topRightReading = irTopRightSmall.getReading();
    Reading leftReading = irBottomLeftMedium.getReading();
    Reading rightReading = irBottomRightMedium.getReading();
    bool doubleReading = abs(leftReading.distanceMM - rightReading.distanceMM) < 5 && leftReading.foundObject && rightReading.foundObject;

    Vision visionRead;
    if (doubleReading) {
        visionRead = FOUND_OBSTACLE;
    } else if (topLeftReading.foundObject || topRightReading.foundObject) {
        visionRead = FOUND_OBSTACLE;
    } else {
        if (leftReading.foundObject && rightReading.foundObject) {
            if (leftReading.distanceMM < rightReading.distanceMM) {
                rightReading.foundObject = false;
            } else {
                leftReading.foundObject = false;
            }
        }
        if (leftReading.foundObject && leftReading.distanceMM <= 800) {
            visionRead = FOUND_LEFT;
        } else if (rightReading.foundObject && rightReading.distanceMM <= 800) {
            visionRead = FOUND_RIGHT;
        } else {
            visionRead = FOUND_NONE;
        }
    }

    if (visionProtential == visionRead && visionRead != visionCurrent) {
        visionCounter ++;
        if (visionCounter > 10) {
            visionCurrent = visionRead;
            visionCounter = 0;

            switch (visionCurrent)
            {
            case FOUND_LEFT:
                Movement::slightTurn(MEDIUM, FORWARD, LEFT);
                state = TURNING_LEFT;
                break;
            
            case FOUND_RIGHT:
                Movement::slightTurn(MEDIUM, FORWARD, RIGHT);
                state = TURNING_RIGHT;
                break;

            case FOUND_OBSTACLE:
                if (state == TURNING_LEFT) {
                    Movement::spin(SLOW, RIGHT);
                    state = SCANNING_RIGHT;
                } else if (state == TURNING_RIGHT) {
                    Movement::spin(SLOW, LEFT);
                    state = SCANNING_LEFT;
                }
                break;

            default:
                break;
            }

            Serial.print(topLeftReading.distanceMM);
            Serial.print(",");
            Serial.print(topRightReading.distanceMM);
            Serial.print(",");
            Serial.print(leftReading.distanceMM);
            Serial.print(",");
            Serial.print(rightReading.distanceMM);
            Serial.print(",");
            Serial.print("Vision saw:");
            Serial.print(visionCurrent);
            Serial.print(" and State is:");
            Serial.print(state);
            Serial.println();
        }
    } else {
        visionProtential = visionRead;
        visionCounter = 1;
    }

    // Serial.print(topReading.distanceMM);
    // Serial.print(",");
    // Serial.print(leftReading.distanceMM);
    // Serial.print(",");
    // Serial.print(rightReading.distanceMM);
    // Serial.print(",");
    // Serial.print(state);
    // Serial.println();

    delay(10);
}
