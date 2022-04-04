#include <Servo.h>

#ifndef Movement_h
#define Movement_h

enum Speed {SPEED_UNINITIALISED = 0, STOPPED, SLOW, MEDIUM, FAST};
enum Motor {LEFT_MOTOR, RIGHT_MOTOR};
enum Direction {DIRECTION_UNINITIALISED = 0, FORWARD = 1, REVERSE = -1};
enum Side {LEFT = -1, RIGHT = 1};

struct Command {
    Speed leftSpeed;
    Direction leftDirection;
    Speed rightSpeed;
    Direction rightDirection;
};

class Movement
{
public:
    static void initialise(bool swapMotors, bool swapLeftPolarity, bool swapRightPolarity);
    static void initialise(bool swapMotors, bool swapLeftPolarity, bool swapRightPolarity, int speedSlow, int speedMedium, int speedFast);
    static void together(Speed speed, Direction direction);
    static void together(Speed speed) {Movement::together(speed, FORWARD);}
    static void slightTurn(Speed speed, Direction direction, Side side);
    static void largeTurn(Speed speed, Direction direction, Side side);
    static void spin(Speed speed, Side side);
    static void stop(void);
    static void test(void);
private:
    static void moveMotors(Command command);
    static void moveMotors(Speed speed, Direction direction);
    static void moveMotors(Speed leftSpeed, Direction leftDirection, Speed rightSpeed, Direction rightDirection);
    static void moveMotor(Motor motor, Speed speed, Direction direction);
    static Speed decreaseSpeed(Speed speed);
};

#endif