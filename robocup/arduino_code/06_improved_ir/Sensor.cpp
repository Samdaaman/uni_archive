#include <Arduino.h>
#include "Sensor.h"

int getAverageOfBuffer(int rawValueBuffer[], int len)
{
    long sum = 0;
    int numValues = 0;
    for (int i = 0; i < len; i++)
    {
        int rawValue = rawValueBuffer[i];
        if (rawValue != -1)
        {
            sum += rawValue;
            numValues ++;
        }
    }
    return sum / numValues;
}

Sensor::Sensor(uint8_t pinNumber, Type type)
{
    this->pinNumber = pinNumber;
    this->type = type;
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        rawValueBuffer[i] = -1;
    }
}

Reading Sensor::getReading(void)
{
    int rawValue;
    rawValue = analogRead(pinNumber);
    rawValueBuffer[bufferIndex] = rawValue;
    bufferIndex ++;
    if (bufferIndex == BUFFER_SIZE)
    {
        bufferIndex = 0;
    }
    int processedValue = getAverageOfBuffer(rawValueBuffer, BUFFER_SIZE);
    Reading reading = {false, 0, rawValue, processedValue};

    switch (type)
    {
    case IR_SMALL_TOP:
        reading.foundObject = (processedValue > 80); // && (rawValue < 550);
        break;
    case IR_SMALL_BOTTOM:
        reading.foundObject = (processedValue > 80);
        break;
    
    default:
        break;
    }
    return reading;
}