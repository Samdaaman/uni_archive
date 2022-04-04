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
enum State {S_BOUNCING_AROUND, S_SCANNING, S_DRIVING_TOWARDS, S_PICKING_UP_LEFT, S_PICKING_UP_RIGHT};
State state = S_BOUNCING_AROUND;

enum Vision {V_FOUND_NONE, V_FOUND_WEIGHT_LEFT, V_FOUND_WEIGHT_RIGHT, V_FOUND_WALL_LEFT, V_FOUND_WALL_RIGHT, V_FOUND_WALL_FRONT};
Vision visionProtential = V_FOUND_NONE;
Vision visionCurrent = V_FOUND_NONE;
int visionCounter = 0;

Sensor irTopLeftSmall = Sensor(A1, IR_MEDIUM);
Sensor irTopRightSmall = Sensor(A3, IR_MEDIUM);
Sensor irBottomLeftMedium = Sensor(A0, IR_LONG);
Sensor irBottomRightMedium = Sensor(A2, IR_LONG);

void resetToState(State state, bool resetAllWatchdogs);
void resetToScaning() {resetToState(S_SCANNING, false);}
void resetToBouncingAround() {resetToState(S_BOUNCING_AROUND, false);}
Watchdog WD_bouncingTooLong = Watchdog(3000, resetToScaning);
Watchdog WD_scanningTooLong = Watchdog(5000, resetToBouncingAround);

void setup()
{
    if (!IMOBILISED) {
        Movement::initialise(SWAP_MOTORS, SWAP_LEFT_POLARITY, SWAP_RIGHT_POLARITY);
        Arm.initialise();
        Arm.backToStarting();
    }
    Serial.begin(9600);
    delay(2000);
    resetToState(S_BOUNCING_AROUND, true);
}

void updateStateWithVision(Vision vision)
{
    switch (vision)
    {
    case V_FOUND_NONE:
        /* code */
        break;

    case V_FOUND_WEIGHT_LEFT:
        Movement::slightTurn(MEDIUM, FORWARD, LEFT);
        resetToState(S_DRIVING_TOWARDS, true);
        break;

    case V_FOUND_WEIGHT_RIGHT:
        Movement::slightTurn(MEDIUM, FORWARD, RIGHT);
        resetToState(S_DRIVING_TOWARDS, true);
        break;

    case V_FOUND_WALL_FRONT:
        Movement::together(MEDIUM, REVERSE);
        delay(1000);
        resetToState(S_SCANNING, true);
        break;

    case V_FOUND_WALL_LEFT:
        Movement::slightTurn(MEDIUM, REVERSE, LEFT);
        delay(1000);
        Movement::spin(SLOW, RIGHT);
        delay(1000);
        Movement::stop();
        resetToState(S_BOUNCING_AROUND, true);
        break;

    case V_FOUND_WALL_RIGHT:
        Movement::slightTurn(MEDIUM, REVERSE, RIGHT);
        delay(1000);
        Movement::spin(SLOW, LEFT);
        delay(1000);
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
        // delay(200);
        return;
    }
    // TODO watchdogs
    Movement::spin(SLOW, foundSide);
    Reading bottomLeftReading = irBottomLeftMedium.getReading();
    Reading bottomRightReading = irBottomRightMedium.getReading();
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
    delay(1000);
    Movement::together(SLOW, REVERSE);
    delay(1500);
    Movement::stop();
    Arm.elbowPutDown();
    Movement::together(SLOW, FORWARD);
    delay(3000);
    Movement::stop();
    Arm.clawClose();
    Arm.elbowPickup();
    Arm.clawOpen();
    while (1) {};
    resetToState(S_BOUNCING_AROUND, true);
}

void resetToState(State nextState, bool resetAllWatchdogs)
{
    Serial.print("state=");
    Serial.print(nextState);
    Serial.println();
    switch (nextState)
    {
    case S_SCANNING:
        Movement::spin(SLOW, LEFT);
        WD_bouncingTooLong.reset();
        break;
    
    case S_BOUNCING_AROUND:
        Movement::together(MEDIUM);
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

    default:
        break;
    }
    state = nextState;
    if (resetAllWatchdogs) {
        WD_scanningTooLong.reset();
        WD_bouncingTooLong.reset();
    }
}

Vision getVisionFromReadings(Reading topLeftReading, Reading topRightReading, Reading bottomLeftReading, Reading bottomRightReading)
{
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

    if (topLeftReading.foundObject && (bottomLeftReading.foundObject && abs(bottomLeftReading.distanceMM - topLeftReading.distanceMM) < 300) /* todo tune depth */) {
        // Found a tall object on the left but it is far away so ignore
        return V_FOUND_NONE;
    }

    if (topRightReading.foundObject && (bottomRightReading.foundObject && abs(bottomRightReading.distanceMM - topRightReading.distanceMM) < 300) /* todo tune depth */) {
        // Found a tall object on the right but it is far away so ignore
        return V_FOUND_NONE;
    }
    
    if (bottomLeftReading.foundObject && bottomRightReading.foundObject) {
        if (abs(bottomLeftReading.distanceMM - bottomRightReading.distanceMM) > 300 /* todo tune depth threshold */) {
            // Found two obstacles at different depths
            return bottomLeftReading.distanceMM < bottomRightReading.distanceMM ? V_FOUND_WEIGHT_LEFT : V_FOUND_WEIGHT_RIGHT;
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
            resetToState(S_PICKING_UP_LEFT, true);
        }
        return V_FOUND_WEIGHT_LEFT;
    }
    
    if (bottomRightReading.foundObject && bottomRightReading.distanceMM <= 700) {
        // Found something on the right side only
        if (bottomRightReading.distanceMM <= 200) {
            resetToState(S_PICKING_UP_RIGHT, true);
        }
        return V_FOUND_WEIGHT_RIGHT;
    }

    return V_FOUND_NONE;
}

void loop()
{
    Reading topLeftReading = irTopLeftSmall.getReading();
    Reading topRightReading = irTopRightSmall.getReading();
    Reading bottomLeftReading = irBottomLeftMedium.getReading();
    Reading bottomRightReading = irBottomRightMedium.getReading();

    Vision visionRead = getVisionFromReadings(topLeftReading, topRightReading, bottomLeftReading, bottomRightReading);
    bool shouldUpdateState = false;

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
        updateStateWithVision(visionCurrent);
    }

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

    bool stop = false; // visionCurrent == V_FOUND_WEIGHT_LEFT || visionCurrent == V_FOUND_WEIGHT_RIGHT;
    static unsigned int print_counter = 0;
    if (print_counter ++ % 40 == 0 || stop) {
        Serial.print("state=");
        Serial.print(state);
        Serial.println();
        Serial.print("vision=");
        Serial.print(visionCurrent);
        Serial.println();
        Serial.print("sensors=");
        Reading readings[] = {topLeftReading, topRightReading, bottomLeftReading, bottomRightReading};
        for (int i = 0; i < sizeof(readings) / sizeof(Reading); i++) {
            Serial.print(readings[i].distanceMM);
            if (i >= sizeof(readings) / sizeof(Reading) - 1) {
                Serial.println();
            } else {
                Serial.print(",");
            } 
        }

        if (stop) {
            Movement::stop();
            IMOBILISED = true;
        }
    }
    


    delay(10);
}
