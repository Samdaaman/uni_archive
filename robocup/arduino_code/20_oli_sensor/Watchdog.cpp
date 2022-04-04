#include "Watchdog.h"
#include "Arduino.h"

Watchdog::Watchdog(unsigned long timeout, void (*callback)())
{
    this->timeout = timeout;
    this->callback = callback;
    this->lastReset = millis();
    Serial.print("Watchdog init");
}

void Watchdog::update()
{
    if (millis() - lastReset > timeout)
    {
        (*callback)();
    }
}

void Watchdog::reset()
{
    lastReset = millis();
}