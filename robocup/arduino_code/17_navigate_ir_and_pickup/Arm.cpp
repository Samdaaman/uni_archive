#include "Arm.h"
#include <Servo.h>
#include <Arduino.h>
#include "Herkulex.h"

// TOOO make commands async

const int IO_PIN = 49;
const int ELBOW_SERVO_PIN = 4;
const int LEFT_CLAW_SERVO_ID = 0x02;
const int RIGHT_CLAW_SERVO_ID = 0x01;
const float CLAW_OPEN_LEFT = -20.0;  // between -160.0 and 160.0
const float CLAW_OPEN_RIGHT = 30.0;
const float CLAW_CLOSE_LEFT = 15.0;  // between -160.0 and 160.0
const float CLAW_CLOSE_RIGHT = -5.0;
const int CLAW_OPEN_TIME = 1000;
const int CLAW_CLOSE_TIME = 1000;
const int ELBOW_SPEED = 1;
const int ELBOW_INITIAL = 1500;
const int ELBOW_UP = 1900; //1700;  // 500 to 2500 which corresponds to degrees
const int ELBOW_DOWN = 600;  // 500 to 2500 which corresponds to degrees

static bool isInitialised = false;

Servo elbowServo = Servo();

void ArmClass::initialise()
{
    pinMode(IO_PIN, OUTPUT);
    digitalWrite(IO_PIN, 1);
    elbowServo.attach(ELBOW_SERVO_PIN);
    Herkulex.beginSerial2(115200);
    Herkulex.reboot(LEFT_CLAW_SERVO_ID);
    Herkulex.reboot(RIGHT_CLAW_SERVO_ID);
    delay(500);
    Herkulex.initialize();
    delay(200);
    backToStarting();
    isInitialised = true;
}

void ArmClass::clawOpen()
{
    if (!isInitialised) {
        return;
    }

    Herkulex.moveOneAngle(LEFT_CLAW_SERVO_ID, CLAW_OPEN_LEFT, CLAW_OPEN_TIME, LED_GREEN);
    Herkulex.moveOneAngle(RIGHT_CLAW_SERVO_ID, CLAW_OPEN_RIGHT, CLAW_OPEN_TIME, LED_RED);
    delay(CLAW_OPEN_TIME);
}

void ArmClass::clawClose()
{
    if (!isInitialised) {
        return;
    }

    Herkulex.moveOneAngle(LEFT_CLAW_SERVO_ID, CLAW_CLOSE_LEFT, CLAW_CLOSE_TIME, LED_GREEN);
    Herkulex.moveOneAngle(RIGHT_CLAW_SERVO_ID, CLAW_CLOSE_RIGHT, CLAW_CLOSE_TIME, LED_RED);
    delay(CLAW_CLOSE_TIME);
}

void ArmClass::elbowPickup()
{
    elbowMoveToMicroseconds(ELBOW_UP);
    delay(1000);
}

void ArmClass::elbowPutDown()
{
    elbowMoveToMicroseconds(ELBOW_DOWN);
    delay(1000);
}

void ArmClass::backToStarting()
{
    clawOpen();
    elbowPickup();
}

void ArmClass::elbowMoveToMicroseconds(int nextMicroseconds)
{
    static int lastElbowMicroseconds = ELBOW_INITIAL;
    if (!isInitialised) {
        return;
    }

    if (lastElbowMicroseconds != nextMicroseconds) {
        int currentMicroseconds = lastElbowMicroseconds;
        int change = nextMicroseconds > lastElbowMicroseconds ? ELBOW_SPEED : -ELBOW_SPEED;
        while (currentMicroseconds != nextMicroseconds) {
            currentMicroseconds += change;
            if (nextMicroseconds > lastElbowMicroseconds ? currentMicroseconds > nextMicroseconds : currentMicroseconds < nextMicroseconds) {
                currentMicroseconds = nextMicroseconds;
            }
            elbowServo.writeMicroseconds(currentMicroseconds);
            delay(2);
        }
        lastElbowMicroseconds = nextMicroseconds;
    }
}

ArmClass Arm;