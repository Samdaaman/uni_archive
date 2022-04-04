#include <Arduino.h>
#include "Movement.h"
#include "Sensor.h"
#include "Arm.h"
#include "Watchdog.h"

static bool IMOBILISED = false;
const bool SWAP_MOTORS = false;
const bool SWAP_LEFT_POLARITY = false;
const bool SWAP_RIGHT_POLARITY = false;

unsigned long stateCounter = 0;
enum State {
    S_NULL, S_BOUNCING_AROUND, S_SCANNING, S_DRIVING_TOWARDS, S_PICKING_UP_LEFT, S_PICKING_UP_RIGHT, S_AVOIDING
};
State state = S_NULL;

enum Vision {
    V_FOUND_NONE, V_FOUND_WEIGHT_LEFT_FAR, V_FOUND_WEIGHT_LEFT_CLOSE, V_FOUND_WEIGHT_RIGHT_FAR, V_FOUND_WEIGHT_RIGHT_CLOSE, V_FOUND_WALL_LEFT, V_FOUND_WALL_RIGHT, V_FOUND_WALL_FRONT
};
Vision visionCurrent = V_FOUND_NONE;
int visionCounter = 0;

Sensor irTopLeftSmall = Sensor(A1, IR_MEDIUM);
Sensor irTopRightSmall = Sensor(A3, IR_MEDIUM);
Sensor irBottomLeftMedium = Sensor(A0, IR_LONG);
Sensor irBottomRightMedium = Sensor(A2, IR_LONG);

Reading topLeftReading;
Reading topRightReading;
Reading bottomLeftReading;
Reading bottomRightReading;
 
void resetToState(State state, bool resetAllWatchdogs);
void resetToScaning() {resetToState(S_SCANNING, false);}
void resetToBouncingAround() {resetToState(S_BOUNCING_AROUND, false);}
void resetToReverseThenScan() {Movement::together(MEDIUM, REVERSE); myDelay(1000, false); Movement::spin(MEDIUM, RIGHT); myDelay(1000, true); resetToState(S_NULL, false); resetToState(S_SCANNING, false);}
Watchdog WD_bouncingTooLong = Watchdog(10000, resetToScaning);
Watchdog WD_scanningTooLong = Watchdog(10000, resetToBouncingAround);
Watchdog WD_forwardTooLong = Watchdog(2500, resetToReverseThenScan);

void setup()
{
    if (!IMOBILISED) {
        Movement::initialise(SWAP_MOTORS, SWAP_LEFT_POLARITY, SWAP_RIGHT_POLARITY);
        Arm.initialise();
        Arm.backToStarting();
    }
    // Serial.begin(9600);
    Watchdog::enableAll();
    delay(1000);
    resetToState(S_BOUNCING_AROUND, true);
}

void myDelay(unsigned long ms, bool allowVisionChange) {updateVision(ms, allowVisionChange);}

void updateVision(unsigned long repeatForMilliseconds, bool allowVisionChange)
{
    unsigned long function_start = millis();
    Vision visionRead;

    while (true) {
        topLeftReading = irTopLeftSmall.getReading();
        topRightReading = irTopRightSmall.getReading();
        bottomLeftReading = irBottomLeftMedium.getReading();
        bottomRightReading = irBottomRightMedium.getReading();

        visionRead = getVisionFromReadings();
        static Vision visionProtential = visionRead;

        if (allowVisionChange) {
            if (visionProtential == visionRead && visionRead != visionCurrent) {
                visionCounter ++;
                if (visionCounter > 10) {
                    visionCounter = 0;
                    visionCurrent = visionRead;
                }
            } else {
                visionProtential = visionRead;
                visionCounter = 1;
            }
        }

        bool stop = false; // visionCurrent == V_FOUND_WEIGHT_LEFT || visionCurrent == V_FOUND_WEIGHT_RIGHT;
        static unsigned int print_counter = 0;
        // Serial.print("vision=");
        // Serial.print(visionCurrent);
        // Serial.println();
        // Serial.print("sensors=");
        // Reading readings[] = {topLeftReading, topRightReading, bottomLeftReading, bottomRightReading};
        // for (int i = 0; i < sizeof(readings) / sizeof(Reading); i++) {
        //     Serial.print(readings[i].distanceMM);
        //     if (i >= sizeof(readings) / sizeof(Reading) - 1) {
        //         Serial.println();
        //     } else {
        //         Serial.print(",");
        //     } 
        // }

        if (stop) {
            Movement::stop();
            IMOBILISED = true;
        }

        if (repeatForMilliseconds == 0) {
            break;
        } else if (millis() - function_start > repeatForMilliseconds) {
            break;
        } else {
            delay(10);
        }
    }
}

Vision getVisionFromReadings() {
    if ((topLeftReading.foundObject && topLeftReading.distanceMM < 200) && (topRightReading.foundObject && topRightReading.distanceMM < 200)) {
        // Found a close tall object
        return V_FOUND_WALL_FRONT;
    }

    if (topLeftReading.foundObject && topLeftReading.distanceMM < 200) {
        // Found a close tall object on the left
        return V_FOUND_WALL_LEFT;
    }
    
    if (topRightReading.foundObject && topRightReading.distanceMM < 200) {
        // Found a close tall object on the right
        return V_FOUND_WALL_RIGHT;
    }

    if (topLeftReading.foundObject && (bottomLeftReading.foundObject && abs(bottomLeftReading.distanceMM - topLeftReading.distanceMM) < 150) /* todo tune depth */) {
        // Found a tall object on the left but it is far away so ignore
        return V_FOUND_NONE;
    }

    if (topRightReading.foundObject && (bottomRightReading.foundObject && abs(bottomRightReading.distanceMM - topRightReading.distanceMM) < 150) /* todo tune depth */) {
        // Found a tall object on the right but it is far away so ignore
        return V_FOUND_NONE;
    }
    
    if (bottomLeftReading.foundObject && bottomRightReading.foundObject) {
        if (abs(bottomLeftReading.distanceMM - bottomRightReading.distanceMM) > 200 /* todo tune depth threshold */) {
            // Found two obstacles at different depths
            if (bottomLeftReading.distanceMM < bottomRightReading.distanceMM) {
                if (bottomLeftReading.distanceMM <= 200) {
                    return V_FOUND_WEIGHT_LEFT_CLOSE;
                } else {
                    return V_FOUND_WEIGHT_LEFT_FAR;
                }
            } else {
                if (bottomRightReading.distanceMM <= 200) {
                    return V_FOUND_WEIGHT_RIGHT_CLOSE;
                } else {
                    return V_FOUND_WEIGHT_RIGHT_FAR;
                }
            }
        } else {
            // Found two obstacles at the same depth
            if (min(bottomLeftReading.distanceMM, bottomRightReading.distanceMM) < 200) {
                return V_FOUND_WALL_FRONT;
            } else {
                // Ignore walls that are more than Xcm away
                return V_FOUND_NONE;
            }
        }
    }
    
    if (bottomLeftReading.foundObject && bottomLeftReading.distanceMM <= 700) {
        // Found something on the left side only
        if (bottomLeftReading.distanceMM <= 200) {
            return V_FOUND_WEIGHT_LEFT_CLOSE;
        } else {
            return V_FOUND_WEIGHT_LEFT_FAR;
        }
    }
    
    if (bottomRightReading.foundObject && bottomRightReading.distanceMM <= 700) {
        // Found something on the right side only
        if (bottomRightReading.distanceMM <= 200) {
            return V_FOUND_WEIGHT_RIGHT_CLOSE;
        } else {
            return V_FOUND_WEIGHT_RIGHT_FAR;
        }
    }

    return V_FOUND_NONE;
}

void updateStateWithVision(Vision vision)
{
    switch (vision)
    {
    case V_FOUND_NONE:
        break;

    case V_FOUND_WEIGHT_LEFT_FAR:
        Movement::slightTurn(MEDIUM, FORWARD, LEFT);
        resetToState(S_DRIVING_TOWARDS, true);
        break;

    case V_FOUND_WEIGHT_LEFT_CLOSE:
        resetToState(S_PICKING_UP_LEFT, true);
        break;

    case V_FOUND_WEIGHT_RIGHT_CLOSE:
        resetToState(S_PICKING_UP_RIGHT, true);
        break;

    case V_FOUND_WEIGHT_RIGHT_FAR:
        Movement::slightTurn(MEDIUM, FORWARD, RIGHT);
        resetToState(S_DRIVING_TOWARDS, true);
        break;

    case V_FOUND_WALL_FRONT:
        resetToState(S_AVOIDING, false);
        Movement::together(MEDIUM, REVERSE);
        myDelay(1000, true);
        resetToState(S_SCANNING, true);
        break;

    case V_FOUND_WALL_LEFT:
        resetToState(S_AVOIDING, false);
        Movement::slightTurn(MEDIUM, REVERSE, LEFT);
        myDelay(1000, true);
        Movement::spin(SLOW, RIGHT);
        myDelay(1000, true);
        Movement::stop();
        resetToState(S_BOUNCING_AROUND, true);
        break;

    case V_FOUND_WALL_RIGHT:
        resetToState(S_AVOIDING, false);
        Movement::slightTurn(MEDIUM, REVERSE, RIGHT);
        myDelay(1000, true);
        Movement::spin(SLOW, LEFT);
        myDelay(1000, true);
        Movement::stop();
        resetToState(S_BOUNCING_AROUND, true);
        break;

    default:
        break;
    }
}

void pickupRoutine(Side foundSide)
{
    if (IMOBILISED) {
        myDelay(2000, false);
        resetToState(S_SCANNING, true);
    }
    // TODO watchdogs
    Movement::spin(SLOW, foundSide);
    bottomLeftReading = irBottomLeftMedium.getReading();
    bottomRightReading = irBottomRightMedium.getReading();
    while ((foundSide == LEFT ? bottomLeftReading.distanceMM : bottomRightReading.distanceMM) < 300) {
        bottomLeftReading = irBottomLeftMedium.getReading();
        bottomRightReading = irBottomRightMedium.getReading();
        delay(1);
    }
    unsigned long startTime = millis();
    while ((foundSide == LEFT ? bottomRightReading.distanceMM : bottomLeftReading.distanceMM) < 300) {
        bottomLeftReading = irBottomLeftMedium.getReading();
        bottomRightReading = irBottomRightMedium.getReading();
        delay(1);
    }
    unsigned long endTime = millis();
    Movement::spin(SLOW, (Side)-foundSide);
    while (millis() < (endTime - startTime) / 2 + endTime) {}
    Movement::stop();
    myDelay(1000, false);
    Movement::together(SLOW, REVERSE);
    myDelay(1500, false);
    Movement::stop();
    Arm.elbowPutDown();
    Movement::together(SLOW, FORWARD);
    myDelay(3000, false);
    Movement::stop();
    Arm.clawClose();
    Arm.elbowPickup();
    Arm.clawOpen();
    resetToState(S_SCANNING, true);
}

void resetToState(State nextState, bool resetAllWatchdogs)
{
    if (state == nextState) {
        return;
    }

    // Serial.print("state=");
    // Serial.print(nextState);
    // Serial.println();
    switch (nextState)
    {
    case S_SCANNING:
        Movement::spin(SLOW, LEFT);
        WD_bouncingTooLong.reset();
        break;
    
    case S_BOUNCING_AROUND:
        Movement::together(MEDIUM);
        myDelay(1000, true);
        WD_scanningTooLong.reset();
        break;

    case S_DRIVING_TOWARDS:
        // Handled by vision based off direction
        break;

    case S_PICKING_UP_LEFT:
        pickupRoutine(LEFT);
        break;

    case S_PICKING_UP_RIGHT:
        pickupRoutine(RIGHT);
        break;

    case S_AVOIDING:
        // Handled by vision
        break;

    default:
        break;
    }

    state = nextState;
    if (resetAllWatchdogs) {
        WD_scanningTooLong.reset();
        WD_bouncingTooLong.reset();
        WD_forwardTooLong.reset();
    }
}

void loop()
{
    updateVision(0, true);
    updateStateWithVision(visionCurrent);

    // Update watchdogs
    if (state == S_BOUNCING_AROUND) {
        WD_bouncingTooLong.update();
    } else {
        WD_bouncingTooLong.reset();
    }
    if (state == S_SCANNING) {
        WD_scanningTooLong.update();
    } else {
        WD_bouncingTooLong.reset();
    }
    if (state == S_DRIVING_TOWARDS) {
        WD_forwardTooLong.update();
    } else {
        WD_forwardTooLong.reset();
    }

    delay(10);
}
