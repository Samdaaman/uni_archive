#include "Watchdog.h"
#include "Arduino.h"

Watchdog::Watchdog(unsigned long timeout, void (*callback)())
{
    this->timeout = timeout;
    this->callback = callback;
    this->lastUpdated = millis();
}

void Watchdog::allUpdate(Watchdog watchdogs[])
{
    for (int i = 0; i < sizeof(watchdogs) / sizeof(Watchdog); i ++) {
        watchdogs[i].update();
    }
}

void Watchdog::update()
{
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdated > timeout)
    {
        //(*callback)();
    }
    lastUpdated = currentMillis;
}

void Watchdog::reset()
{
    lastUpdated = millis();
}