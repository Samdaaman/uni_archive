#include <Arduino.h>
#include "Sensor.h"

int getAverageOfBuffer(int rawValueBuffer[])
{
    long sum = 0;
    int numValues = 0;
    for (int i = 0; i < BUFFER_SIZE; i++)
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

int getMedianOfBuffer(int rawValueBuffer[])
{
    int sortedBuffer[BUFFER_SIZE] = {};
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (i == 0) {
            sortedBuffer[i] = rawValueBuffer[i];
        } else {
            for (int j = 0; j <= i; j++) {
                if (j == i || rawValueBuffer[j] > sortedBuffer[j]) {
                    sortedBuffer[j] = rawValueBuffer[j];
                    break;
                }
            }
        }
    }
    return sortedBuffer[BUFFER_SIZE / 2];
}

Sensor::Sensor(uint8_t pinNumber, Type type)
{
    this->pinNumber = pinNumber;
    this->type = type;
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        // rawValueBuffer[i] = -1;
        rawValueBuffer[i] = 0;
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
    int processedValue = getAverageOfBuffer(rawValueBuffer);
    Reading reading = {false, 0, rawValue, processedValue};

    switch (type)
    {
    case IR_MEDIUM:
        if (80 <= reading.processedValue && reading.processedValue <= 600) {
            reading.foundObject = true;
            // Max value is 800
            reading.distanceMM = (int)(48000L / (reading.processedValue - 20));
        }
        break;
    case IR_LONG:
        if (80 <= reading.processedValue && reading.processedValue <= 550) {
            reading.foundObject = true;
            reading.distanceMM = (int)(946200L / (reading.processedValue * 10 - 169));
        }
        break;
    
    default:
        break;
    }
    return reading;
}