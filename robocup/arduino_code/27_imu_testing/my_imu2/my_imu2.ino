#include "Arduino.h"
#include "Accelerometer.h"

void setup() {
    Serial.begin(9600);
    Accelerometer.initialise();
    pinMode(13, OUTPUT);

    // // For timing purposes
    // delay(1000);
    // unsigned long start = millis();
    // for (int i = 0; i < 1000; i ++) {
    //     Accelerometer.getLevelStatus();
    // }
    // unsigned long time = millis() - start;
    // Serial.print("Time=");
    // Serial.println(time);
    // while(1);
}

bool checkOnLean() {
    static uint16_t counter = 0;
    static unsigned firstTimestamp = 0;

    if (counter < 100) {
        counter ++;
    } else {
        counter = 0;
        
        if (Accelerometer.getLevelStatus() == L_ON_LEAN) {
            if (firstTimestamp != 0) {
                if (millis() - firstTimestamp > 1000) {
                    firstTimestamp = 0;
                    return true;
                }
            } else {
                firstTimestamp = millis();
            }
        } else {
            firstTimestamp = 0;
        }
    }
    return false;
}

void loop() {
    if (checkOnLean()) {
        Serial.println("On lean");
        digitalWrite(13, HIGH);
        while (checkOnLean() == true) {
            delay(100);
        }
        digitalWrite(13, LOW);
    }

    delay(2);
}