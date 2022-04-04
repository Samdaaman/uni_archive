#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

enum LevelStatus {L_ON_LEAN, L_LEVEL, L_ERROR};

class AccelerometerClass {
    public:
        void AccelerometerClass::initialise();
        LevelStatus AccelerometerClass::getLevelStatus();
        bool initSuccess = false;
};

extern AccelerometerClass Accelerometer;

#endif