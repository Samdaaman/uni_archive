#include <Servo.h>

Servo myservoA;      // create servo object to control a servo
Servo myservoB;      // create servo object to control a servo

int x = 0;
int y = 0;
int speedPercent = 0;
int directionPercent = 0;
int leftValue = 0;
int rightValue = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(49, OUTPUT);                 //Pin 49 is used to enable IO power
  digitalWrite(49, 1);                 //Enable IO power on main CPU board
  
  myservoA.attach(3);  // attaches the servo pin 3 to the servo object
  myservoB.attach(2);  // attaches the servo pin 2 to the servo object
}

void loop() {
  // put your main code here, to run repeatedly:
  x = analogRead(A1);
  y = analogRead(A0);
//  Serial.print("x=");
//  Serial.print(y);
//  Serial.println();
//  Serial.print("y=");
//  Serial.print(y);
//  Serial.println();

  speedPercent = (y - 500) / 4;
  directionPercent = (x - 500) / 4;
  
  leftValue = max(min((speedPercent * 5 + 1500) - directionPercent * 5, 2000), 1000);
  rightValue = max(min((speedPercent * 5 + 1500) + directionPercent * 5, 2000), 1000);
  
  myservoA.writeMicroseconds(leftValue);
  myservoB.writeMicroseconds(rightValue);
  
  delay(100);
}
