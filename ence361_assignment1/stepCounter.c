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

/**********************************************************
 * Includes
 *********************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "acc.h"
#include "i2c_driver.h"
#include "circBufT.h"
#include "stepCounter.h"


/**********************************************************
 * Constants
 *********************************************************/
#define BUFFER_SIZE 5 // buffer size for the circular buffers
#define THRESHHOLD_ACCERATION 384 // represents 1.5G in raw units (256 * 1.5)


/**********************************************************
 * Local (Private) Prototypes
 *********************************************************/
static uint32_t getAccNormSq(void);
static uint32_t getAverageOfCircBuf(circBuf_t *circBuf);
static void initAccelerometer(void);


/**********************************************************
 * Private Globals
 *********************************************************/
static circBuf_t circBufNormSq;
volatile static int32_t stepCount;


/**********************************************************
 * Local (private) Declarations
 *********************************************************/
// Updates the circular buffers (doesn't average)
static uint32_t getAccNormSq(void)
{
    char    fromAccl[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t bytesToRead = 6; // TODO magic number

    fromAccl[0] = ACCL_DATA_X0;
    I2CGenTransmit(fromAccl, bytesToRead, READ, ACCL_ADDR);

    int32_t xVal = (int16_t)((fromAccl[2] << 8) | fromAccl[1]);
    int32_t yVal = (int16_t)((fromAccl[4] << 8) | fromAccl[3]);
    int32_t zVal = (int16_t)((fromAccl[6] << 8) | fromAccl[5]);

    return xVal * xVal + yVal * yVal + zVal * zVal;
}


// Gets the mean value of a circular buffer
static uint32_t getAverageOfCircBuf(circBuf_t *circBuf)
{
    int32_t sum = 0;
    int32_t temp = 0;
    uint32_t i;
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        temp = readCircBuf(circBuf);
        sum = sum + temp;
    }
    return ((2 * sum + BUFFER_SIZE) / 2 / BUFFER_SIZE); // math based off lab4 to compute the correct rounding
}

// ******************************************************
// initAccelerometer: initializes the accelerometer
static void initAccelerometer(void)
{
    char toAccl[] = {0, 0};  // parameter, value

    // Enable I2C Peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    // Set I2C GPIO pins
    GPIOPinTypeI2C(I2CSDAPort, I2CSDA_PIN);
    GPIOPinTypeI2CSCL(I2CSCLPort, I2CSCL_PIN);
    GPIOPinConfigure(I2CSCL);
    GPIOPinConfigure(I2CSDA);

    // Setup I2C
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), true);
    GPIOPinTypeGPIOInput(ACCL_INT2Port, ACCL_INT2);

    //Initialise ADXL345 Accelerometer
    // set +-2g, 10 bit resolution, active low interrupts
    toAccl[0] = ACCL_DATA_FORMAT;
    toAccl[1] = (ACCL_RANGE_2G | ACCL_FULL_RES);
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_PWR_CTL;
    toAccl[1] = ACCL_MEASURE;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);


    toAccl[0] = ACCL_BW_RATE;
    toAccl[1] = ACCL_RATE_100HZ;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_INT;
    toAccl[1] = 0x00;       // Disable interrupts from accelerometer.
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_OFFSET_X;
    toAccl[1] = 0x00;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_OFFSET_Y;
    toAccl[1] = 0x00;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);

    toAccl[0] = ACCL_OFFSET_Z;
    toAccl[1] = 0x00;
    I2CGenTransmit(toAccl, 1, WRITE, ACCL_ADDR);
}


/**********************************************************
 * Public Declarations (defined in .h file)
 *********************************************************/

// ******************************************************
// checkForStep: called by ISR and checks to see if the
// user has made a step, gets data from accelerometer
void checkForStep(void)
{
    static bool previousWasBelowThreshold = true;
    
    uint32_t normSq = getAccNormSq();
    writeCircBuf(&circBufNormSq, normSq);
    uint32_t averageNormSq = getAverageOfCircBuf(&circBufNormSq);

    if (averageNormSq > THRESHHOLD_ACCERATION * THRESHHOLD_ACCERATION) {
        if (previousWasBelowThreshold) {
            stepCount ++;
            previousWasBelowThreshold = false;
        }
    } else {
        previousWasBelowThreshold = true;
    }
}


// *******************************************************
// changeStepCount: Function which safely adds or
// subtracts steps stored in the private step count value
// The amount of steps can be a negative to subtract
// Using this function prevents the step count going
// below zero
void changeStepCount (int32_t amountOfSteps)
{
    bool intsWereDisabled = IntMasterDisable();
    // Critical section so disable interrupts
    if (stepCount < -amountOfSteps) {
        stepCount = 0;
    } else {
        stepCount += amountOfSteps;
    }
    if (!intsWereDisabled)
    {
        IntMasterEnable();
    }

}

// *******************************************************
// getStepCount: Function which safely gets and returns
// the current step count recorded
int32_t getStepCount(void)
{
    return stepCount; // Interrupt protection not needed as will execute in one clock cycle
}

// *******************************************************
// resetStepCount: Function which safely resets the
// current step count recorded
void resetStepCount(void)
{
    stepCount = 0;  // Interrupt protection not needed as will execute in one clock cycle
}

// ******************************************************
// initStepCounter: initializes the step counter module
// and circular buffers etc
void initStepCounter(void)
{
    initAccelerometer();
    initCircBuf(&circBufNormSq, BUFFER_SIZE);
}
