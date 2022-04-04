#ifndef BUTTONS_5_H_
#define BUTTONS_5_H_

// *******************************************************
//
// buttons5.h
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

//*****************************************************************************
// Includes
//*****************************************************************************
#include <stdbool.h>

//*****************************************************************************
// Constants
//*****************************************************************************
enum butNames {UP = 0, DOWN, LEFT, RIGHT, SWITCH1};

// *******************************************************
// Initialize the variables associated with the set of buttons
// defined by the constants in the buttons5.h header file.
// *******************************************************
void initButtons (void);

// *******************************************************
// updateButtons: Function designed to be called regularly. It polls all
// buttons once and updates variables associated with the buttons if
// necessary.  It is efficient enough to be part of an ISR, e.g. from
// a SysTick interrupt.
// De-bounce algorithm: A state machine is associated with each button.
// A state change occurs only after a series of consecutive button polls
// have occured with the new state.
//*********************************************************
void updateButtons (void);

// ********************************************
// Check if button was short pressed
// Clears the button short press flag
// ********************************************
bool wasButtonShortPress(uint8_t butName);

// ********************************************
// Check if button was long pressed
// Clears the button long press flag
// ********************************************
bool wasButtonLongPress(uint8_t butName);

// ********************************************
// Returns whether the button is currently down (pressed)
// Useful for determining switch states as it does not
// clear the flag
// ********************************************
bool isButtonDown(uint8_t butName);

#endif /*BUTTONS_5_H_*/



