//*****************************************************************************
//
// pulseInrpt.c - Program to monitor the pulse width in a periodic digital
// waveform on Tiva Pin J1-03.
// Operates in the range 5 usec to 250 ms (i.e. for a square wave signal,
// the valid range is 2 Hz to 100 kHz).
//
// Author:  P.J. Bones	UCECE
// Last modified:	9.3.2017
//
//*****************************************************************************
// Lab 3 sheet for ENCE361 2012 using the Stellaris Cortex M3 processor:
// Step 6.    Write an interrupt service routine to read the SysTick
// register operating as a decrementing wrap-on-zero counter, using
// SysTickValueGet()  for every 0->1  or 1->0  transition on an EVK header
// pin.  Maintain a circular buffer of size NUM_TICKS
// with NUM_TICKS = 20, say. Produce a continually updating display 
// of the mean pulse duration in msec over the contents of the buffer.  
// Remember that the 24-bit counter value will wrap once it reaches zero.
//
// In 2017: the program has been adapted for the Tiva TM4C123GH6PM and
// Orbit booster pack.  Tiva pin J1-03 (PB0) is monitored.
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "stdio.h"
#include "stdlib.h"
#include "OrbitOLEDInterface.h"
#include "circBufT.h"

//*****************************************************************************
// Constants
//*****************************************************************************
#define MAX_24BIT_VAL 0X0FFFFFF
#define BUF_SIZE 20

//*****************************************************************************
// Global variables
//*****************************************************************************
static circBuf_t g_inBuffer;		// uint32_t buffer of size BUF_SIZE (intervals)
volatile static uint32_t g_u32IntCnt;	// Monitors interrupts

//*****************************************************************************
// Local prototypes
//*****************************************************************************
void PinChangeIntHandler (void);
void initClock (void);
void initPin (void);
void displayMeanValUs (int32_t i32MeanValUs);
void displayInrptCnt (void);

//*****************************************************************************
//
// The interrupt handler for the for the pin change interrupt. Note that
//  the SysTick counter is decrementing.
//
//*****************************************************************************
void
PinChangeIntHandler (void)
{
	static uint32_t u32LastCnt;	// Retains previous value of SysTick counter
	uint32_t u32SysTickCnt;
	uint32_t u32PortVal;
	int32_t i32Diff;
	
	// 
	// Clear the interrupt (documentation recommends doing this early)
	GPIOIntClear (GPIO_PORTB_BASE, GPIO_PIN_0);
	//
	// Read the pin
	u32PortVal = GPIOPinRead (GPIO_PORTB_BASE, GPIO_PIN_0);
	//
	// Read the SysTick counter value 
	u32SysTickCnt = SysTickValueGet ();
	//
	// Calculate pulse width (at trailing edge only)
	if (!u32PortVal)
	{		// end of pulse, so place calculated interval in buffer
		i32Diff = u32LastCnt - u32SysTickCnt;
		if (i32Diff < 0)	// i.e. if the wrap-around has occured between edges
		   i32Diff += MAX_24BIT_VAL + 1;
		writeCircBuf (&g_inBuffer, (uint32_t) i32Diff);
	}
	//
	// Prepare for next interrupt
	u32LastCnt = u32SysTickCnt;
	//
	// Count interrupts
	g_u32IntCnt++;
}

//*****************************************************************************
// Initialisation functions: clock, GPIO pin, display, buffer
//*****************************************************************************
void
initClock (void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
  //
  // Set up the period for the SysTick timer to get the maximum span.  
  SysTickPeriodSet (MAX_24BIT_VAL);
  //
  // Enable SysTick device (no interrupt)
  SysTickEnable ();
}

//********************************************************
void
initPin (void)
{
    // Enable the port used for input = Port B
    SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOB);
    //
    // Register the handler for Port B into the vector table
    GPIOIntRegister (GPIO_PORTB_BASE, PinChangeIntHandler);
    //
    // Configure the pin used:  input on PB0 (J1-03)
    GPIOPinTypeGPIOInput (GPIO_PORTB_BASE, GPIO_PIN_0);
    GPIOPadConfigSet (GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPD);
    //
    // Set up the pin change interrupt (both edges)
    GPIOIntTypeSet (GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_BOTH_EDGES);
    // 
    // Enable the pin change interrupt 
    GPIOIntEnable (GPIO_PORTB_BASE, GPIO_PIN_0);
    IntEnable (INT_GPIOB);	// NB: INT_GPIOF is defined in inc/hw_ints.h
}

// *******************************************************
void
initDisplay (void)
{
  // intialise the Orbit OLED display
	OLEDInitialise ();
}


//*****************************************************************************
// Function to display the mean interval in usec
//*****************************************************************************
void
displayMeanValUs (int32_t i32MeanValUs)
{
    char string[17];	// Orbit OLED handles up to 16 characters across
	
    OLEDStringDraw ("Montr Pin J1-03", 0, 0);
    OLEDStringDraw ("Mean interval = ", 0, 1);
    sprintf (string, "%8d usec", i32MeanValUs);
    OLEDStringDraw (string, 2, 2);
}

//*****************************************************************************
// Function to display the interrupt count 
//*****************************************************************************
void
displayInrptCnt (void)
{
   char string[17];
	
   sprintf (string, "Count = %d", g_u32IntCnt);
   OLEDStringDraw (string, 0, 3);
}
//*****************************************************************************

int
main(void)
{
	uint32_t u32sum, u32mean;
	int i;
    uint32_t u32ClksPerUsec;	// Used to convert clock pulses to usec

	
	initClock ();
	initPin ();
	initDisplay ();
	initCircBuf (&g_inBuffer, BUF_SIZE);

	u32ClksPerUsec = SysCtlClockGet() / 1000000L;
	    // NB: this assumes the clock rate is an integer number of MHz

	// Enable interrupts to the processor.
	IntMasterEnable ();
    
	while (1)
	{
		//
		// Background task: calculate the mean (rounded value) of the intervals in the
		//  circular buffer and display it

		for (i = 0, u32sum = 0; i < BUF_SIZE; i++)
			u32sum += readCircBuf(&g_inBuffer);
		// Note that this accesses all BUFSIZE elements, irrespective of
		// when it is executed.
		u32mean = (2 * u32sum + BUF_SIZE) / BUF_SIZE / 2;	  // = round(sum/BUF_SIZE)
		displayMeanValUs ((2 * u32mean + u32ClksPerUsec) / u32ClksPerUsec / 2);
		displayInrptCnt ();
	}
}

