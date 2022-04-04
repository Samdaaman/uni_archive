// *******************************************************
// 
// buttons5.c
//
// Program for handling button presses and user peripherals
// Based off buttons4.c by P.J. Bones UCECE on 7.2.2018 but
// with added code for switch and long-press functionality
//
//
//
// By Joel, Sam and Mikael 21/05/2020
// 
// *******************************************************


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "inc/tm4c123gh6pm.h"  // Board specific defines (for PF0)
#include "buttons5.h"


// *******************************************************
// Constants to the module
// *******************************************************
// UP button
#define UP_BUT_PERIPH SYSCTL_PERIPH_GPIOE
#define UP_BUT_PORT_BASE GPIO_PORTE_BASE
#define UP_BUT_PIN GPIO_PIN_0
#define UP_BUT_NORMAL false
// DOWN button
#define DOWN_BUT_PERIPH SYSCTL_PERIPH_GPIOD
#define DOWN_BUT_PORT_BASE GPIO_PORTD_BASE
#define DOWN_BUT_PIN GPIO_PIN_2
#define DOWN_BUT_NORMAL false
// LEFT button
#define LEFT_BUT_PERIPH SYSCTL_PERIPH_GPIOF
#define LEFT_BUT_PORT_BASE GPIO_PORTF_BASE
#define LEFT_BUT_PIN GPIO_PIN_4
#define LEFT_BUT_NORMAL true
// RIGHT button
#define RIGHT_BUT_PERIPH SYSCTL_PERIPH_GPIOF
#define RIGHT_BUT_PORT_BASE GPIO_PORTF_BASE
#define RIGHT_BUT_PIN GPIO_PIN_0
#define RIGHT_BUT_NORMAL true
// SWITCH 1
#define SWITCH1_BUT_PERIPH SYSCTL_PERIPH_GPIOA
#define SWTICH1_BUT_PORT_BASE GPIO_PORTA_BASE
#define SWITCH1_BUT_PIN GPIO_PIN_7
#define SWITCH1_BUT_NORMAL false

#define NUM_BUT_POLLS_RELEASE 5
#define NUM_BUT_POLLS_SHORT 3
#define NUM_BUT_POLLS_LONG 50
#define NUM_BUTS 5

// *******************************************************
// Globals to module
// *******************************************************
static bool but_value[NUM_BUTS];
static bool but_down_short[NUM_BUTS];
static bool but_down_long[NUM_BUTS];
static uint8_t but_count_down[NUM_BUTS];
static uint8_t but_count_release[NUM_BUTS];
static bool but_flag_short[NUM_BUTS];
static bool but_flag_long[NUM_BUTS];
static bool but_normal[NUM_BUTS];   // Corresponds to the electrical state

// *******************************************************
// Initialize the variables associated with the set of buttons
// defined by the constants in the buttons5.h header file.
// *******************************************************
void initButtons(void)
{
    // UP button (active HIGH)
    SysCtlPeripheralEnable(UP_BUT_PERIPH);
    GPIOPinTypeGPIOInput(UP_BUT_PORT_BASE, UP_BUT_PIN);
    GPIOPadConfigSet(UP_BUT_PORT_BASE, UP_BUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
    but_normal[UP] = UP_BUT_NORMAL;
    // DOWN button (active HIGH)
    SysCtlPeripheralEnable(DOWN_BUT_PERIPH);
    GPIOPinTypeGPIOInput(DOWN_BUT_PORT_BASE, DOWN_BUT_PIN);
    GPIOPadConfigSet(DOWN_BUT_PORT_BASE, DOWN_BUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
    but_normal[DOWN] = DOWN_BUT_NORMAL;
    // LEFT button (active LOW)
    SysCtlPeripheralEnable(LEFT_BUT_PERIPH);
    GPIOPinTypeGPIOInput(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN);
    GPIOPadConfigSet(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    but_normal[LEFT] = LEFT_BUT_NORMAL;
    // RIGHT button (active LOW)
    // Note that PF0 is one of a handful of GPIO pins that need to be
    // "unlocked" before they can be reconfigured.  This also requires
    // #include "inc/tm4c123gh6pm.h"
    SysCtlPeripheralEnable(RIGHT_BUT_PERIPH);
    //---Unlock PF0 for the right button:
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
    GPIO_PORTF_CR_R |= GPIO_PIN_0; //PF0 unlocked
    GPIO_PORTF_LOCK_R = GPIO_LOCK_M;
    GPIOPinTypeGPIOInput(RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN);
    GPIOPadConfigSet(RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    but_normal[RIGHT] = RIGHT_BUT_NORMAL;
    // SWITCH1 switch (active HIGH)
    SysCtlPeripheralEnable(SWITCH1_BUT_PERIPH);
    GPIOPinTypeGPIOInput (SWTICH1_BUT_PORT_BASE, SWITCH1_BUT_PIN);
    GPIOPadConfigSet(SWTICH1_BUT_PORT_BASE, SWITCH1_BUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
    but_normal[SWITCH1] = SWITCH1_BUT_NORMAL;

    int i;
    for (i = 0; i < NUM_BUTS; i++)
    {
        but_value[i] = false;
        but_count_down[i] = 0;
        but_count_release[i] = 0;
        but_flag_short[i] = false;
        but_flag_long[i] = false;
    }
}

// *******************************************************
// updateButtons: Function designed to be called regularly. It polls all
// buttons once and updates variables associated with the buttons if
// necessary.  It is efficient enough to be part of an ISR, e.g. from
// a SysTick interrupt.
// De-bounce algorithm: A state machine is associated with each button.
// A state change occurs only after a series of consecutive button polls
// have occured with the new state.
//*********************************************************
void updateButtons (void)
{    
    // Read the pins; true means HIGH, false means LOW
    but_value[UP] = (GPIOPinRead(UP_BUT_PORT_BASE, UP_BUT_PIN) == UP_BUT_PIN);
    but_value[DOWN] = (GPIOPinRead(DOWN_BUT_PORT_BASE, DOWN_BUT_PIN) == DOWN_BUT_PIN);
    but_value[LEFT] = (GPIOPinRead(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN) == LEFT_BUT_PIN);
    but_value[RIGHT] = (GPIOPinRead(RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN) == RIGHT_BUT_PIN);
    but_value[SWITCH1] = (GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_7) == GPIO_PIN_7);

    // Iterate through the buttons, updating button variables as required
    int i;
    for (i = 0; i < NUM_BUTS; i++)
    {
        if (but_value[i] != but_normal[i])
        {
            but_count_down[i] ++;
            but_count_release[i] = 0;
            if (but_count_down[i] >= NUM_BUT_POLLS_LONG && !but_down_long[i])
            {
                but_down_long[i] = true;
                but_flag_long[i] = true;
            }
            if (but_count_down[i] >= NUM_BUT_POLLS_SHORT && !but_down_short[i])
            {
                but_down_short[i] = true;
                but_flag_short[i] = true;
            }
        }
        else
        {
            but_count_down[i] = 0;
            but_count_release[i] ++;
            if (but_count_release[i] >= NUM_BUT_POLLS_RELEASE)
            {
                but_down_short[i] = false;
                but_down_long[i] = false;
            }
        }
             
    }
}

// ********************************************
// Check if button was short pressed
// Clears the button short press flag
// ********************************************
bool wasButtonShortPress(uint8_t butName)
{
    if (but_flag_short[butName]) {
        but_flag_short[butName] = false;
        return true;
    }
    else
    {
        return false;
    }
}

// ********************************************
// Check if button was long pressed
// Clears the button long press flag
// ********************************************
bool wasButtonLongPress(uint8_t butName)
{
    if (but_flag_long[butName]) {
        but_flag_long[butName] = false;
        return true;
    }
    else
    {
        return false;
    }
    
}

// ********************************************
// Returns whether the button is currently down (pressed)
// Useful for determining switch states as it does not
// clear the flag
// ********************************************
bool isButtonDown(uint8_t butName)
{
    return but_value[butName];
}
