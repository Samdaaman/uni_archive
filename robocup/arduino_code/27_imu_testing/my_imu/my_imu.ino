#include "Arduino.h"
#include "Accelerometer.h"

void setup() {
    Serial.begin(9600);
}

void loop() {
    Accelerations accelerations;
    bool success = Accelerometer.getAccelerations(&accelerations);

    if (success) {
        Serial.print("Accel ");
        Serial.print("X: "); Serial.print(accelerations.mx    ); Serial.print("  ");
        Serial.print("Y: "); Serial.print(accelerations.my    ); Serial.print("  ");
        Serial.print("Z: "); Serial.print(accelerations.mz    ); Serial.print("  ");
        Serial.println("m/s^2");
    } else {
        Serial.println("Failed");
    }

    Serial.println("");

    delay(500);
}