#include <stdint.h>

#ifndef Sensor_h
#define Sensor_h

struct Reading
{
    bool foundObject;
    int distanceMM;
    int rawValue;
    int processedValue;
};

const int BUFFER_SIZE = 20;

enum Type {IR_SMALL_TOP, IR_SMALL_BOTTOM};

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