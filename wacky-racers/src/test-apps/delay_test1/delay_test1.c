/* File:   delay_test1.c
   Author: M. P. Hayes, UCECE
   Date:   1 May 2015
   Descr:  This flashes an LED at 2 Hz using 
     delay routines.  It is useful for testing that
     the MCU clock is correctly configured. 
*/
#include "delay.h"
#include "pio.h"

#define FLASH_RATE 2

int
main (void)
{
    pio_config_set (LED1_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (LED2_PIO, PIO_OUTPUT_HIGH);

    while (1)
    {
        delay_ms (1000 / (FLASH_RATE * 2));

        pio_output_toggle (LED1_PIO);
    }
}
