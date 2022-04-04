#include <Arduino.h>
#include "Movement.h"
#include "Sensor.h"

const bool SWAP_MOTORS = false;
const bool SWAP_LEFT_POLARITY = false;
const bool SWAP_RIGHT_POLARITY = false;

unsigned long stateCounter = 0;
enum State {S_STOPPED, S_NOT_STOPPED};
State currentState = S_NOT_STOPPED  ;

enum Vision {V_FOUND_NONE, V_FOUND_WEIGHT_LEFT, V_FOUND_WEIGHT_RIGHT, V_FOUND_WALL_LEFT, V_FOUND_WALL_RIGHT};
Vision visionProtential = V_FOUND_NONE;
Vision visionCurrent = V_FOUND_NONE;
int visionCounter = 0;

Sensor irTopLeftSmall = Sensor(A1, IR_MEDIUM);
Sensor irTopRightSmall = Sensor(A3, IR_MEDIUM);
Sensor irBottomLeftMedium = Sensor(A0, IR_LONG);
Sensor irBottomRightMedium = Sensor(A2, IR_LONG);

void setup()
{
    //Movement::initialise(SWAP_MOTORS, SWAP_LEFT_POLARITY, SWAP_RIGHT_POLARITY);
    Serial.begin(9600);
    delay(2000);
    Movement::spin(SLOW, LEFT);
}

void onStateUpdate()
{
    switch (visionCurrent)
    {
    case V_FOUND_NONE:
        /* code */
        break;

    case V_FOUND_WEIGHT_LEFT:
        Movement::slightTurn(MEDIUM, FORWARD, LEFT);
        break;

    case V_FOUND_WEIGHT_RIGHT:
        Movement::slightTurn(MEDIUM, FORWARD, RIGHT);
        break;

    default:
        break;
    }
}

void loop()
{
    Reading topLeftReading = irTopLeftSmall.getReading();
    Reading topRightReading = irTopRightSmall.getReading();
    Reading bottomLeftReading = irBottomLeftMedium.getReading();
    Reading bottomRightReading = irBottomRightMedium.getReading();

    if (currentState == S_STOPPED) {
        return;
    }

    Vision visionRead = V_FOUND_NONE;
    bool shouldUpdateState = false;

    if (bottomLeftReading.foundObject && bottomRightReading.foundObject) {
        if (abs(bottomLeftReading.distanceMM - bottomRightReading.distanceMM) > 300 /* todo tune depth threshold */) {
            // Found two obstacles at different depths
            visionRead = bottomLeftReading.distanceMM < bottomRightReading.distanceMM ? V_FOUND_WEIGHT_LEFT : V_FOUND_WEIGHT_RIGHT;
        } else {
            // Found two obstacles at the same depth
            visionRead = V_FOUND_NONE;
        }
    } else if (bottomLeftReading.foundObject && topLeftReading.foundObject && abs(bottomLeftReading.distanceMM - topLeftReading.distanceMM) < 300 /* todo tune depth */) {
        visionRead = V_FOUND_WALL_LEFT;
    } else if (bottomRightReading.foundObject && topRightReading.foundObject && abs(bottomRightReading.distanceMM - topRightReading.distanceMM) < 300 /* todo tune depth */) {
        visionRead = V_FOUND_WALL_RIGHT;
    } else if (bottomLeftReading.foundObject && bottomLeftReading.distanceMM <= 700) {
        // Found something on the left side only
        visionRead = V_FOUND_WEIGHT_LEFT;
    } else if (bottomRightReading.foundObject && bottomRightReading.distanceMM <= 700) {
        // Found something on the right side only
        visionRead = V_FOUND_WEIGHT_RIGHT;
    }

    if (visionProtential == visionRead && visionRead != visionCurrent) {
        visionCounter ++;
        if (visionCounter > 10) {
            visionCounter = 0;
            shouldUpdateState = true;
        }
    } else {
        visionProtential = visionRead;
        visionCounter = 1;
    }

    if (shouldUpdateState) {
        visionCurrent = visionRead;
        onStateUpdate();
    }

    if (false) {
        Reading readings[] = {topLeftReading, topRightReading, bottomLeftReading, bottomRightReading};
        for (int i = 0; i < sizeof(readings) / sizeof(Reading); i++) {
            Serial.print(readings[i].distanceMM);
            Serial.print(',');
        }
    } else {
        //Serial.print(visionRead);
        Serial.print(bottomLeftReading.distanceMM);
        Serial.print(",");
        Serial.print(bottomRightReading.distanceMM);
    }
    Serial.println();

    delay(10);
}
