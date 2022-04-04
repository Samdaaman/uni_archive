#ifndef SWTICH_H
#define SWITCH_H

#include <Arduino.h>

#define SWITCH_PIN 26

extern void waitForSwitch();

void waitForSwitch() {
    static bool initialised = false;
    if (!initialised) {
        pinMode(SWITCH_PIN, INPUT);
        initialised = true;
    }

    while (digitalRead(SWITCH_PIN) == HIGH) {}
    while (digitalRead(SWITCH_PIN) == LOW) {}
}

#endif