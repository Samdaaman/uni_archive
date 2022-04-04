#include <Arduino.h>
#include <Servo.h>

const bool SWAP_MOTORS = false;
const bool SWAP_LEFT_POLARITY = false;
const bool SWAP_RIGHT_POLARITY = false;
const int IO_PIN = 49;
Servo servo = Servo();
int commands[] = {
    1500, 
    1200,
    1500
};

void setup()
{
    Serial.begin(9600);
    Serial.print("Initialise");
    pinMode(IO_PIN, OUTPUT);
    digitalWrite(IO_PIN, 1);
    servo.attach(4);

    delay(1000);
    int length = sizeof(commands) / sizeof(int);
    for (int i = 500; i <= 2500; i += 500) {
        servo.writeMicroseconds(i);
        Serial.write(i);
        delay(1000);
    }
}

void loop()
{

}
