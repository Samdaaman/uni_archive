#include "Accelerometer.h"
#include "My_BMX160.h"

#define THRESHOLD_Z 10200

DFRobot_BMX160 bmx160;

void AccelerometerClass::initialise() {
    if (bmx160.begin()) {
        initSuccess = true;
    } else {
        Serial.println("Failed to init accelerometer");
        initSuccess = false;
    }
    delay(100);
}

LevelStatus AccelerometerClass::getLevelStatus() {
    if (!initSuccess) {
        return L_ERROR;
    }
    
    bmx160SensorData accelerationData;
    if (!bmx160.getAllData(NULL, NULL, &accelerationData)) {
        return L_ERROR;
    }

    // Serial.println(accelerationData.z);
    if (accelerationData.z >= THRESHOLD_Z) {
        return L_LEVEL;
    } else {
        return L_ON_LEAN;
    }
}

AccelerometerClass Accelerometer;