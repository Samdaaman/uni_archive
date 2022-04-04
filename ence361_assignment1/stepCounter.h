#ifndef STEP_COUNTER_H_
#define STEP_COUNTER_H_

/**********************************************************
 *
 * stepCounter.c
 *
 * This is a module which tracks the users steps. It uses
 * a functions such as getStepCount() and checkForStep() 
 * which hide voltile varibles behind a layer of abstaction
 * This means that it is "threadsafe" and some methods can
 * be called from an ISR to improve performance and
 * reliablity
 *
 *    Sam Hogan
 *    21st May 2020
 *
 *********************************************************/


//*****************************************************************************
// Includes
//*****************************************************************************
#include <stdint.h>


//*****************************************************************************
// Functions
//*****************************************************************************

// *******************************************************
// changeStepCount: Function which safely adds or
// subtracts steps stored in the private step count value
// The amount of steps can be a negative to subtract
// Using this function prevents the step count going
// below zero
void changeStepCount (int32_t amountOfSteps);

// *******************************************************
// getStepCount: Function which safely gets and returns
// the current step count recorded
int32_t getStepCount (void);

// *******************************************************
// resetStepCount: Function which safely resets the
// current step count recorded
void resetStepCount (void);

// ******************************************************
// checkForStep: called by ISR and checks to see if the
// user has made a step, gets data from accelerometer
void checkForStep(void);

// ******************************************************
// initStepCounter: initializes the step counter module
// and circular buffers etc
void initStepCounter(void);

#endif /*STEP_COUNTER_H0_*/
