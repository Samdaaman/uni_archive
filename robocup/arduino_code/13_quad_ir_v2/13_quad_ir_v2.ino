#include <Arduino.h>
#include "Movement.h"
#include "Sensor.h"

const bool SWAP_MOTORS = false;
const bool SWAP_LEFT_POLARITY = false;
const bool SWAP_RIGHT_POLARITY = false;

unsigned long stateCounter = 0;
enum State {S_STOPPED, S_SCANNING_LEFT, S_SCANNING_RIGHT, S_TURNING_LEFT, S_TURNING_RIGHT, S_BOUNCING};
State currentState = S_SCANNING_LEFT;

enum Vision {V_FOUND_NONE, V_FOUND_OBSTACLE_FRONT, V_FOUND_WEIGHT_LEFT, V_FOUND_WEIGHT_RIGHT, V_FOUND_WEIGHT_CLOSE};
Vision visionProtential = V_FOUND_NONE;
Vision visionCurrent = V_FOUND_NONE;
int visionCounter = 0;

Sensor irTopLeftSmall = Sensor(A1, IR_MEDIUM);
Sensor irTopRightSmall = Sensor(A3, IR_MEDIUM);
Sensor irBottomLeftMedium = Sensor(A0, IR_LONG);
Sensor irBottomRightMedium = Sensor(A2, IR_LONG);

void setup()
{
    Movement::initialise(SWAP_MOTORS, SWAP_LEFT_POLARITY, SWAP_RIGHT_POLARITY);
    Serial.begin(9600);
    delay(2000);
    Movement::spin(SLOW, LEFT);
}

void loop()
{
    Reading topLeftReading = irTopLeftSmall.getReading();
    Reading topRightReading = irTopRightSmall.getReading();
    Reading bottomLeftReading = irBottomLeftMedium.getReading();
    Reading bottomRightReading = irBottomRightMedium.getReading();

    Reading readings[] = {topLeftReading, topRightReading, bottomLeftReading, bottomRightReading};
    for (int i = 0; i < sizeof(readings) / sizeof(Reading); i++) {
        Serial.print(readings[i].distanceMM);
        Serial.print(',');
    }
    Serial.println();

    if (currentState == S_STOPPED) {
        return;
    }

    Vision visionRead = V_FOUND_NONE;
    bool forceStateUpdate = false;

    // if ((bottomLeftReading.foundObject && bottomLeftReading.distanceMM < 200) || (bottomRightReading.foundObject && bottomRightReading.distanceMM < 200)) {
    //     visionRead = V_FOUND_WEIGHT_CLOSE;
    //     forceStateUpdate = true;
    // } else 
    if ((topLeftReading.foundObject && topLeftReading.distanceMM < 100) || (topRightReading.foundObject && topRightReading.distanceMM < 100)) {
        visionRead = V_FOUND_OBSTACLE_FRONT;
    } else if (bottomLeftReading.foundObject && bottomRightReading.foundObject) {
        if (abs(bottomLeftReading.distanceMM - bottomRightReading.distanceMM) > 300 /* todo tune depth threshold */) {
            visionRead = bottomLeftReading.distanceMM < bottomRightReading.distanceMM ? V_FOUND_WEIGHT_LEFT : V_FOUND_WEIGHT_RIGHT;
        } else {
            //visionRead = min(bottomLeftReading.distanceMM, bottomRightReading.distanceMM) > 300 ? V_FOUND_NONE : V_FOUND_OBSTACLE_FRONT;
        }
    } else if (bottomLeftReading.foundObject && bottomLeftReading.distanceMM <= 500) {
        visionRead = V_FOUND_WEIGHT_LEFT;
    } else if (bottomRightReading.foundObject && bottomRightReading.distanceMM <= 500) {
        visionRead = V_FOUND_WEIGHT_RIGHT;
    }

    if (visionProtential == visionRead && visionRead != visionCurrent) {
        visionCounter ++;
        if (visionCounter > 10) {
            visionCounter = 0;
            forceStateUpdate = true;
        }
    } else {
        visionProtential = visionRead;
        visionCounter = 1;
    }

    if (forceStateUpdate) {
        visionCurrent = visionRead;
        switch (visionRead)
        {
        case V_FOUND_NONE:
            /* code */
            break;

        case V_FOUND_OBSTACLE_FRONT:
            Movement::stop();
            currentState = S_STOPPED;
            break;

        case V_FOUND_WEIGHT_LEFT:
            Movement::slightTurn(MEDIUM, FORWARD, LEFT);
            break;

        case V_FOUND_WEIGHT_RIGHT:
            Movement::slightTurn(MEDIUM, FORWARD, RIGHT);
            break;
        
        case V_FOUND_WEIGHT_CLOSE:
            Movement::stop();
            currentState = S_STOPPED;
            break;

        default:
            break;
        }
    }

    delay(10);
}
