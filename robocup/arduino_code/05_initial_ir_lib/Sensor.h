#include <stdint.h>

#ifndef Sensor_h
#define Sensor_h

struct Reading
{
    bool foundObject;
    int distanceMM;
    int rawValue;
};

enum Type {IR_SMALL};

class Sensor
{
public:
    Sensor(uint8_t pinNumber, Type type);
    Reading getReading(void);
    
private:
    int pinNumber;
    Type type;
};

#endif