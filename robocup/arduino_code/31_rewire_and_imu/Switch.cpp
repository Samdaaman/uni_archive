#include "Switch.h"

void waitForSwitch() {
    static bool initialised = false;
    if (!initialised) {
        pinMode(SWITCH_PIN, INPUT);
        initialised = true;
    }

    while (digitalRead(SWITCH_PIN) == HIGH) {}
    while (digitalRead(SWITCH_PIN) == LOW) {}
}