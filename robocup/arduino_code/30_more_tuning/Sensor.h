#include <stdint.h>

#ifndef Sensor_h
#define Sensor_h

class Reading
{
    public:
        Reading() {};
        Reading(int foundObject, int distanceMM, int rawValue, int processedValue);
        bool foundObject = false;
        int distanceMM = 0;
        int rawValue = 0;
        int processedValue = 0;
        bool foundObjectCloser(int cutOffDistance);
};

const int BUFFER_SIZE = 5;

enum Type {IR_MEDIUM, IR_LONG, IR_PROXIMITY};

class Sensor
{
public:
    Sensor(uint8_t pinNumber, Type type);
    Reading getReading(void);
    
private:
    int pinNumber;
    int rawValueBuffer[BUFFER_SIZE];
    int bufferIndex;
    Type type;
};

#endif