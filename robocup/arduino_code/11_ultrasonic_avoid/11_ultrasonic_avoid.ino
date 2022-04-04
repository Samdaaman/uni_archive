#include <Arduino.h>
#include "Movement.h"
#include "Sensor.h"

const bool SWAP_MOTORS = false;
const bool SWAP_LEFT_POLARITY = false;
const bool SWAP_RIGHT_POLARITY = false;



enum State {MOVING_FORWARD, SCANNING_LEFT, SCANNING_RIGHT, TURNING_LEFT, TURNING_RIGHT, SSTOPPED};
State state = SCANNING_LEFT;
Side nextSide = RIGHT;
bool forceStateUpdate = false;

enum Vision {AVOIDED, FOUND_NONE, FOUND_OBSTACLE_FRONT, FOUND_OBSTACLE_SIDE, FOUND_LEFT, FOUND_RIGHT};
Vision visionProtential = FOUND_NONE;
Vision visionCurrent = FOUND_NONE;
int visionCounter = 0;

int foundNoneCounter = 0;

Sensor irTopLeftSmall = Sensor(A0, IR_MEDIUM);
Sensor irTopRightSmall = Sensor(A1, IR_MEDIUM);
Sensor irBottomLeftMedium = Sensor(A2, IR_LONG);
Sensor irBottomRightMedium = Sensor(A3, IR_LONG);
Sensor ultraLeftExp = Sensor(A4, ULTRA_EXP);
Sensor ultraRightExp = Sensor(A5, ULTRA_EXP);

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

    Reading irTopLeftReading = irTopLeftSmall.getReading();
    Reading irTopRightReading = irTopRightSmall.getReading();
    Reading irBottomLeftReading = irBottomLeftMedium.getReading();
    Reading irBottomRightReading = irBottomRightMedium.getReading();
    Reading ultraLeftReading = ultraLeftExp.getReading();
    Reading ultraRightReading = ultraRightExp.getReading();

    bool doubleReading = abs(irBottomLeftReading.distanceMM - irBottomRightReading.distanceMM) < 5 && irBottomLeftReading.foundObject && irBottomRightReading.foundObject;

    Vision visionRead;
    if (ultraLeftReading.foundObject || ultraRightReading.foundObject) {
        // one of the top ultrasonics detected something, obstacle on left/right
        visionRead = FOUND_OBSTACLE_SIDE;
    } else if (doubleReading) {
        // both bottom IRs detected an obstacle, probably a wall or ramp
        visionRead = FOUND_OBSTACLE_FRONT;
    } else if (irTopLeftReading.foundObject || irTopRightReading.foundObject) {
        // one of the top IRs detected something, obstacle at front
        visionRead = FOUND_OBSTACLE_FRONT;
    } else {
        if (irBottomLeftReading.foundObject && irBottomRightReading.foundObject) {
            if (irBottomLeftReading.distanceMM < irBottomRightReading.distanceMM) {
                irBottomRightReading.foundObject = false;
            } else {
                irBottomLeftReading.foundObject = false;
            }
        }
        if (irBottomLeftReading.foundObject && irBottomLeftReading.distanceMM <= 800) {
            visionRead = FOUND_LEFT;
        } else if (irBottomRightReading.foundObject && irBottomRightReading.distanceMM <= 800) {
            visionRead = FOUND_RIGHT;
        } else {
            visionRead = FOUND_NONE;
        }
    }

    if (visionCurrent == FOUND_NONE && !(state == SCANNING_LEFT || state == SCANNING_RIGHT)) {
        // FIXME need one of these for coming out of a scan
        foundNoneCounter ++;
        if (foundNoneCounter > 300) {
            Movement::spin(SLOW, nextSide);
            state = nextSide == LEFT ? SCANNING_LEFT : SCANNING_RIGHT;
            foundNoneCounter = 0;
        }
    } else {
        foundNoneCounter = 0;
    }

    if (visionProtential == visionRead && visionRead != visionCurrent) {
        visionCounter ++;
        if (visionCounter > 10) {
            visionCurrent = visionRead;
            visionCounter = 0;
            forceStateUpdate = true;
        }
    } else {
        visionProtential = visionRead;
        visionCounter = 1;
    }

    if (forceStateUpdate = true) {
        forceStateUpdate = false;
        switch (visionCurrent)
            {
            case FOUND_LEFT:
                Movement::slightTurn(MEDIUM, FORWARD, LEFT);
                state = TURNING_LEFT;
                nextSide = RIGHT;
                break;
            
            case FOUND_RIGHT:
                Movement::slightTurn(MEDIUM, FORWARD, RIGHT);
                state = TURNING_RIGHT;
                nextSide = LEFT;
                break;

            case FOUND_OBSTACLE_SIDE:
                if (ultraLeftReading.foundObject || ultraRightReading.foundObject) {
                    Command command1;
                    Command command2;
                    if (ultraLeftReading.foundObject && ultraRightReading.foundObject) {
                        command1 = {SLOW, REVERSE};
                        if (nextSide == LEFT) {
                            command2 = {SLOW, REVERSE, SLOW, FORWARD};
                        } else {    
                            command2 = {SLOW, FORWARD, SLOW, REVERSE};
                        } 
                    } else if (ultraLeftReading.foundObject) {
                        command1 = {MEDIUM, REVERSE, SLOW, REVERSE};
                        command2 = {SLOW, FORWARD, SLOW, REVERSE};
                        nextSide = RIGHT;
                    } else /*if (ultraRightReading.foundObject)*/ {
                        command1 = {SLOW, REVERSE, MEDIUM, REVERSE};
                        command2 = {SLOW, REVERSE, SLOW, FORWARD};
                        nextSide = LEFT;
                    }
                    avoidWithCommands(command1, command2);
                    visionCurrent = AVOIDED;
                } 
                break;

            case FOUND_OBSTACLE_FRONT:
                if (nextSide == RIGHT) {
                    Movement::spin(SLOW, RIGHT);
                    state = SCANNING_RIGHT;
                } else /*if (nextSide == LEFT)*/ {
                    Movement::spin(SLOW, LEFT);
                    state = SCANNING_LEFT;
                }
                break;

            case FOUND_NONE:
                Movement::together(SLOW, FORWARD);
                state = MOVING_FORWARD;
                break;

            default:
                break;
            }

            Serial.print("Vision saw:");
            Serial.print(visionCurrent);
            Serial.print(" and State is:");
            Serial.print(state);
            Serial.println();
    }

    // Serial.print(ultraRightReading.processedValue);
    // Serial.println();

    delay(10);
}

void avoidWithCommands(Command command1, Command command2) {
    Movement::moveMotors(command1);
    delay(1300);
    Movement::moveMotors(command2);
    delay(800);
}
