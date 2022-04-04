/* File:   ledtape-test2.c
   Author: B Mitchell, UCECE
   Date:   14 April 2021
   Descr:  Test ledtape
*/

#include <pio.h>
#include "target.h"
#include "pacer.h"
#include "ledbuffer.h"

#define NUM_LEDS 27

/*
    This is an alternative method for driving the LED tape using the ledbuffer
    module that is included in the ledtape driver.

    The buffer acts like a small framebuffer upon which you can set RGB values
    at specific positions (not GRB, it handles the translation automatically).
    It also makes it easy to make patterns, shuffle them allow the strip, and
    clear it later. See ledbuffer.h for more details (CTRL-Click it in VS Code).
*/

int main (void)
{
    int count = 0;

    ledbuffer_t* leds = ledbuffer_init(LEDTAPE_PIO, NUM_LEDS);

    pacer_init(30);


    while (1)
    {
        pacer_wait();

        ledbuffer_clear(leds);
        
        for (int i = 0; i < NUM_LEDS; i++)
        {
            ledbuffer_set(leds, count, 255, 0, 255);
            ledbuffer_set(leds, (count + 17) % 27, 255, 0, 0);
            // ledbuffer_set(leds, (count + 18) % 27, 0, 255, 0);
            ledbuffer_set(leds, 26 - count, 0, 0, 255);
            // ledbuffer_set(leds, NUM_LEDS - 1 -(count + 9) % 27, 255, 0, 0);
            ledbuffer_set(leds, 26 - (count + 12) % 27, 0, 255, 0);
        }

        

        ledbuffer_write (leds);
        // ledbuffer_advance (leds, 1);

        count++;
        if (count == NUM_LEDS) {
            count = 0;
        }
    }
}
