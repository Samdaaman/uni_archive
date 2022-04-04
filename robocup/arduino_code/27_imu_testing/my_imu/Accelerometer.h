#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <stdint.h>

typedef struct {
    int16_t mx;
    int16_t my;
    int16_t mz;
} Accelerations;

class AccelerometerClass {
    public:
        AccelerometerClass();
        bool AccelerometerClass::getAccelerations(Accelerations *accelerationsP);
    private:
        bool AccelerometerClass::readReg(uint8_t reg, uint8_t *pBuf, uint8_t len);
};

extern AccelerometerClass Accelerometer;

#endif