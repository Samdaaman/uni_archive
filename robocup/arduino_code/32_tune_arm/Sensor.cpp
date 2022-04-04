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

int sortDesc(const void *cmp1, const void *cmp2) {
    int a = *((int *)cmp1);
    int b = *((int *)cmp2);
    return a > b ? -1 : (a < b ? 1 : 0);
}

int getMedianOfBuffer(int rawValueBuffer[])
{
    int sortedBuffer[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        sortedBuffer[i] = rawValueBuffer[i];
    }
    qsort(sortedBuffer, BUFFER_SIZE, sizeof(int), sortDesc);
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

    if (type == IR_PROXIMITY) {
        pinMode(pinNumber, INPUT);
    }
}

Reading Sensor::getReading(void)
{
    if (type == IR_PROXIMITY) {
        bool foundObject = !digitalRead(pinNumber);
        Reading reading = Reading(foundObject, 0, 0, 0);
        return reading;

    } else {
        int rawValue;
        rawValue = analogRead(pinNumber);
        rawValueBuffer[bufferIndex] = rawValue;
        bufferIndex ++;
        if (bufferIndex == BUFFER_SIZE)
        {
            bufferIndex = 0;
        }
        int processedValue = getAverageOfBuffer(rawValueBuffer);
        Reading reading = Reading(false, 0, rawValue, processedValue);

        switch (type)
        {
        case IR_MEDIUM:
            if (80 <= reading.processedValue && reading.processedValue <= 600) {
                reading.foundObject = true;
                // Max value is 800
                // https://www.eztronics.nl/webshop2/catalog/Sensor/Distance-Range?product_id=598
                reading.distanceMM = (int)(48000L / (reading.processedValue - 20));
            }
            break;
        case IR_LONG:
            if (80 <= reading.processedValue && reading.processedValue <= 490 /* Adjusted from 550 (490 might be optimal too)*/) {
                reading.foundObject = true;
                // https://www.eztronics.nl/webshop2/catalog/Sensor/Distance-Range?product_id=489
                reading.distanceMM = (int)(946200L / (reading.processedValue * 10 - 169));
            }
            break;
        
        default:
            break;
        }

        return reading;
    }
}

Reading::Reading(int foundObject, int distanceMM, int rawValue, int processedValue) {
    this->foundObject = foundObject;
    this->distanceMM = distanceMM;
    this->rawValue = rawValue;
    this->processedValue = processedValue;
}

bool Reading::foundObjectCloser(int cutOffDistance) {
    return foundObject && distanceMM <= cutOffDistance;
}