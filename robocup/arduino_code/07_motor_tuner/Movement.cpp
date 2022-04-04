#include <Arduino.h>
#include <Servo.h>
#include "Movement.h"

const int STOP_SPEED = 1500;
const int IO_PIN = 49;
const int DEFAULT_SPEED_SLOW = 200;
const int DEFAULT_SPEED_MEDIUM = 300;
const int DEFAULT_SPEED_FAST = 500;

static bool initialised;
static bool s_swapMotors;
static int s_swapLeftPolarity;
static int s_swapRightPolarity;
static int s_speedSlow;
static int s_speedMedium;
static int s_speedFast;
static Servo s_firstServo = Servo();
static Servo s_secondServo = Servo();

void Movement::initialise(bool swapMotors, bool swapLeftPolarity, bool swapRightPolarity, int speedSlow, int speedMedium, int speedFast)
{
    s_swapMotors = swapMotors;
    s_swapLeftPolarity = swapLeftPolarity;
    s_swapRightPolarity = swapRightPolarity;
    s_speedSlow = speedSlow;
    s_speedMedium = speedMedium;
    s_speedFast = speedFast;

    

    initialised = true;
    Movement::stop();
}

void Movement::initialise(bool swapMotors, bool swapLeftPolarity, bool swapRightPolarity)
{
    initialise(swapMotors, swapLeftPolarity, swapRightPolarity, DEFAULT_SPEED_SLOW, DEFAULT_SPEED_MEDIUM, DEFAULT_SPEED_FAST);
}

void Movement::together(Speed speed, Direction direction)
{
    moveMotors(speed, direction);
}

void Movement::slightTurn(Speed speed, Direction direction, Side side)
{
    Speed slowSpeed;
    slowSpeed = decreaseSpeed(speed);
    moveMotors(side == LEFT ? slowSpeed : speed, direction, side == RIGHT ? slowSpeed : speed, direction);
}

void Movement::largeTurn(Speed speed, Direction direction, Side side)
{
    Speed slowSpeed;
    slowSpeed = decreaseSpeed(decreaseSpeed(speed));
    moveMotors(side == LEFT ? slowSpeed : speed, direction, side == RIGHT ? slowSpeed : speed, direction);
}

void Movement::spin(Speed speed, Side side)
{
    moveMotors(speed, side == LEFT ? REVERSE : FORWARD, speed, side == RIGHT ? REVERSE : FORWARD);
}

void Movement::stop(void)
{
    moveMotors(STOPPED, FORWARD);
}

void Movement::test(void)
{
    Command commands[] = {
        {STOPPED, FORWARD},
        {SLOW, FORWARD},
        {MEDIUM, FORWARD},
        {FAST, FORWARD},
        {STOPPED, FORWARD},
        {SLOW, REVERSE},
        {MEDIUM, REVERSE},
        {FAST, REVERSE},
        {FAST, FORWARD, STOPPED, FORWARD},
        {STOPPED, FORWARD, FAST, FORWARD}
    };

    delay(3000);
    for (int i = 0; i < sizeof(commands) / sizeof(Command); i++)
    {
        moveMotors(commands[i]);
        delay(1000);
        moveMotors(STOPPED, FORWARD);
        delay(500);
    }
    exit(0);
}

void Movement::moveMotors(Command command)
{
    if (command.rightDirection == DIRECTION_UNINITIALISED || command.rightSpeed == SPEED_UNINITIALISED)
    {
        moveMotors(command.leftSpeed, command.leftDirection);
    }
    else
    {
        moveMotors(command.leftSpeed, command.leftDirection, command.rightSpeed, command.rightDirection);
    }
    
}
void Movement::moveMotors(Speed speed, Direction direction)
{
    moveMotors(speed, direction, speed, direction);
}
void Movement::moveMotors(Speed leftSpeed, Direction leftDirection, Speed rightSpeed, Direction rightDirection)
{
    moveMotor(LEFT_MOTOR, leftSpeed, leftDirection);
    moveMotor(RIGHT_MOTOR, rightSpeed, rightDirection);
}

void Movement::moveMotor(Motor motor, Speed speed, Direction direction)
{
    if (!initialised && speed != STOPPED)
        return;
   
    if (motor == LEFT_MOTOR ? s_swapLeftPolarity : s_swapRightPolarity)
    {
        direction = (Direction)-direction;  // revsere the direction as motor polarity is switched
    }

    int rawSpeed;
    switch (speed)
    {
    case SLOW:
        rawSpeed = s_speedSlow;
        break;
    case MEDIUM:
        rawSpeed = s_speedMedium;
        break;
    case FAST:
        rawSpeed = s_speedFast;
        break;
    default:
        rawSpeed = 0;
        break;
    }

    int speedValue;
    speedValue = STOP_SPEED + direction * rawSpeed;

    if ((motor == LEFT_MOTOR) != (s_swapMotors))
    {
        s_firstServo.writeMicroseconds(speedValue); // move "first" motor
    }
    else
    {
        s_secondServo.writeMicroseconds(speedValue); // move "second" motor
    }    
}

Speed Movement::decreaseSpeed(Speed speed)
{
    if (speed == STOPPED)
    {
        return speed;
    }
    else
    {
        return (Speed)(speed - 1);
    }
}