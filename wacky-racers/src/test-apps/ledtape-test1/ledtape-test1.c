/* File:   ledtape-test11.c
   Author: M. P. Hayes, UCECE
   Date:   30 Jan 2020
   Descr:  Test ledtape
*/

#include <pio.h>
#include "target.h"
#include "pacer.h"
#include "ledtape.h"

#define NUM_LEDS 20

/*
    This test app shows how to program the WS2318B LED tape.
    In this example the ledtape_write() function is used to send the appropriate
    GRB color information down the LED tape. It takes care of all the tricky
    timings, you just need to feed it an array of color values.

    ledtape_write(
        pin - This is the pin you want to send the data on. Should be connected
              to your level shifter.
        data - This is the array of GRB data (8 bit values, 24 bits per pixel).
        size - This is the size of the array in bytes.
    )

    You will also see use of the pacer module. This isn't really needed, but
    makes it easier to see the data packets being sent if you put the data
    signal on an oscilloscope.

    Suggestions:
    * Change the number of LEDs to match yours (or move the NUM_LEDS definition
      to your target file)
    * Change the color being set on all the LEDs.
    * Make an interesting pattern.
    * Make the pattern scroll down the LEDs (see ledtape-test2 for an option).
*/


int
main (void)
{
    uint8_t leds[NUM_LEDS * 3];
    int i;

    for (i = 0; i < NUM_LEDS; i++)
    {
        // Set full green  GRB order
        leds[i * 3] = 255;
        leds[i * 3 + 1] = 0;
        leds[i * 3 + 2] = 0;
    }

    pacer_init(10);

    while (1)
    {
        pacer_wait();

        ledtape_write (LEDTAPE_PIO, leds, NUM_LEDS * 3);
    }
}
