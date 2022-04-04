#include <Arduino.h>
#include <Servo.h>

const bool SWAP_MOTORS = true;
const bool SWAP_LEFT_POLARITY = true;
const bool SWAP_RIGHT_POLARITY = true;
const int IO_PIN = 49;

Servo baseMotor = Servo();
Servo tuneeMotor = Servo();

const int NUMBER_SPEEDS = 6;
int baseConstants[NUMBER_SPEEDS] = {
    200,
    300,
    500,
    -200,
    -300,
    -500
};

int tuneeConstants[NUMBER_SPEEDS] = {};

void setup()
{
    Serial.begin(9600);
    Serial.print("Initialise");
    pinMode(IO_PIN, OUTPUT);
    digitalWrite(IO_PIN, 1);
    baseMotor.attach(3);
    tuneeMotor.attach(2);
    delay(1000);
    baseMotor.writeMicroseconds(2000);
    delay(1000);
    tuneeMotor.writeMicroseconds(2000);
    delay(1000);
    baseMotor.writeMicroseconds(1500);
    tuneeMotor.writeMicroseconds(1500);
    delay(1000);
}

void loop()
{
    for (int i = 0; i < NUMBER_SPEEDS; i++)
    {
        tune(baseConstants[i], &tuneeConstants[i]);
    }
}

void tune(int baseConstant, int *p_tuneeConstant)
{

}