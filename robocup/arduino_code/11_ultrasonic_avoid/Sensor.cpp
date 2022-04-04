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
    case IR_MEDIUM:
        if (80 <= reading.processedValue && reading.processedValue <= 600) {
            reading.foundObject = true;
            reading.distanceMM = (int)(48000L / (reading.processedValue - 20));
        }
        break;
    case IR_LONG:
        if (80 <= reading.processedValue && reading.processedValue <= 550) {
            reading.foundObject = true;
            reading.distanceMM = (int)(94620L / (reading.processedValue * 10 - 169));
        }
        break;

    case ULTRA_EXP:
        if (reading.processedValue < 60) {
            reading.foundObject = true;
        }
        break;
    
    default:
        break;
    }
    return reading;
}