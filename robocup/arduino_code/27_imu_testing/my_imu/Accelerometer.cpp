#include "Accelerometer.h"
#include "Wire.h"

#define BMX160_MAG_DATA_ADDR 0x04
#define I2C_ADDRESS (uint8_t)0x68
// #define SCALE_FACTOR 0.000061035F
#define SCALE_FACTOR 0.59875335

AccelerometerClass::AccelerometerClass() {

}


bool AccelerometerClass::getAccelerations(Accelerations *accelerationsP) {
    uint8_t data[23] = {0};
    if (readReg(BMX160_MAG_DATA_ADDR, data, 23)) {
        int16_t rawX = (int16_t) ((data[15] << 8) | data[14]);
        int16_t rawY = (int16_t) ((data[17] << 8) | data[16]);
        int16_t rawZ = (int16_t) ((data[19] << 8) | data[18]);

        accelerationsP->mx = rawX * SCALE_FACTOR;
        accelerationsP->my = rawY * SCALE_FACTOR;
        accelerationsP->mz = rawZ * SCALE_FACTOR;
        return true;
    } else {
        return false;
    } 
}

bool AccelerometerClass::readReg(uint8_t reg, uint8_t *pBuf, uint8_t len)
{
    Wire.begin();
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(reg);
    if(Wire.endTransmission() != 0)
        return false;
    Wire.requestFrom(I2C_ADDRESS, len);
    for(uint8_t i = 0; i < len; i ++) {
        pBuf[i] = Wire.read();
    }
    Wire.endTransmission();
    return true;
}

AccelerometerClass Accelerometer;