#include "Watchdog.h"
#include "Arduino.h"

static bool enabled = false;

void Watchdog::enableAll()
{
    enabled = true;
}

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
        if (enabled) {
            (*callback)();
        }
    }
}

void Watchdog::reset()
{
    lastReset = millis();
}