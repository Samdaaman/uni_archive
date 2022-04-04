/* File:   usb_vbus_test1.c
   Author: M. P. Hayes, UCECE
   Date:   15 May 2007
   Descr:  Turn on LED if USB connected
*/
#include <pio.h>
#include "target.h"
#include "pacer.h"

/* Define how fast ticks occur.  This must be faster than
   TICK_RATE_MIN.  */
enum {LOOP_POLL_RATE = 200};

/* Define LED flash rate in Hz.  */
enum {LED_FLASH_RATE = 2};

#ifndef UDP_VBUS_PIO
#define UDP_VBUS_PIO BUTTON_PIO
#endif

int
main (void)
{
    uint8_t flash_ticks;

    /* Configure LED PIO as output.  */
    pio_config_set (LED1_PIO, PIO_OUTPUT_LOW);
    pio_config_set (LED2_PIO, PIO_OUTPUT_LOW);
    pio_config_set (UDP_VBUS_PIO, PIO_INPUT);
    
    pacer_init (LOOP_POLL_RATE);
    flash_ticks = 0;

    while (1)
    {
	pacer_wait ();

        // Turn on LED if VBUS detected
        pio_output_set (LED2_PIO, pio_input_get (UDP_VBUS_PIO));

        // Flash other LED
	flash_ticks++;
	if (flash_ticks >= LOOP_POLL_RATE / (LED_FLASH_RATE * 2))
	{
	    flash_ticks = 0;
	    pio_output_toggle (LED1_PIO);
	}
    }
}
