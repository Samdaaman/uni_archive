#include <Arduino.h>
#include "Sensor.h"

Sensor::Sensor(uint8_t pinNumber, Type type)
{
    this->pinNumber = pinNumber;
    this->type = type;
}

Reading Sensor::getReading(void)
{
    int rawValue;
    rawValue = analogRead(pinNumber);
    Reading reading = {false, -1, rawValue};

    switch (type)
    {
    case IR_SMALL:
        reading.foundObject = (rawValue > 100); // && (rawValue < 550);
        break;
    
    default:
        break;
    }
    return reading;
}