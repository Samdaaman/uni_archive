/**********************************************************
 *
 * main.c
 *
 * Main code which serves as the brain for the step counter
 * project. This combines the stepCounter, display and 
 * buttons. It allows the a Tiva Board with an Orbit
 * Booster pack to count steps.
 *
 *    Mikael Ewans, Joel Adams, Sam Hogan
 *    11 May 2020
 *
 **********************************************************/


/**********************************************************
 * Includes
 *********************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "stepCounter.h"
#include "buttons5.h"
#include "display.h"


/**********************************************************
 * Constants
 *********************************************************/
#define SYSTICK_RATE_HZ 50
#define TESTING_INCREMENT_STEPS 100 // amount to change the step count by when in testing mode and up button is pressed
#define TESTING_DECREMENT_STEPS -500 // amount to change the step count by when in testing mode and the down button is pressed


/**********************************************************
 * Globals
 *********************************************************/
static volatile bool displayNeedsUpdateFlag = true; // Flag which indicates when the display needs to be updated


/**********************************************************
 * Local Prototype
 *********************************************************/
static void initClock(void);
static void sysTickIntHandler(void);
static void initInterrupts(void);
static void processButtonChanges(void);


/**********************************************************
 * Local Implementations
 *********************************************************/
// Initialise the clock
static void initClock(void)
{
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
}

// Initialise the interrupts (Sys tick only)
static void initInterrupts(void)
{
    SysTickPeriodSet(SysCtlClockGet() / SYSTICK_RATE_HZ);
    SysTickIntRegister(&sysTickIntHandler);
    SysTickIntEnable();
    SysTickEnable();
    IntMasterEnable();
}

// Function which is called why a systick occurs
static void sysTickIntHandler(void)
{
    checkForStep();
    processButtonChanges();
}

// Updates the buttons and handles any presses that occur
// This could be changing the display mode or units
static void processButtonChanges(void)
{
    updateButtons();

    if (wasButtonShortPress(LEFT) || wasButtonShortPress(RIGHT)) {
        displayNeedsUpdateFlag = true;
        displayModeToggle();
    }
    if (isButtonDown(SWITCH1)) {
        if (wasButtonShortPress(UP)) {
            changeStepCount(TESTING_INCREMENT_STEPS);
        }
        if (wasButtonShortPress(DOWN)) {
            changeStepCount(TESTING_DECREMENT_STEPS);
        }
        wasButtonLongPress(DOWN); // long presses of the down button are ignored (flag reset)
    }
    else {
        if (wasButtonShortPress(UP)) {
            bool unitsToggled = displayUnitsToggle();
            displayNeedsUpdateFlag |= unitsToggled;
        }
        if (wasButtonLongPress(DOWN)) {
            resetStepCount();
        }
        wasButtonShortPress(DOWN); // short presses for down button are ignored (flag reset)
    }

    static int32_t lastStepCount = 0;
    int32_t currentStepCount = getStepCount();
    if (currentStepCount != lastStepCount) {
        displayNeedsUpdateFlag = true;
        lastStepCount = currentStepCount;
    }
}

// Main method
int main(void) {
    initClock();
    initStepCounter();
    initDisplay();
    initButtons();
    initInterrupts();

    while (1) {
        // Only update the display when it needs to be update (flag is true)
        if (displayNeedsUpdateFlag) {
            displayNeedsUpdateFlag = false;
            displayUpdate(getStepCount());
        }
    }
}
