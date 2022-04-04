/* File:   ledflash1.c
   Author: M. P. Hayes, UCECE
   Date:   15 May 2007
   Descr:  Flash an LED
*/
#include <pio.h>
#include "target.h"
#include "pacer.h"
#include "piezo.h"
#include "piezo_beep.h"

/* Define how fast ticks occur.  This must be faster than
   TICK_RATE_MIN.  */
enum {LOOP_POLL_RATE = 200};

/* Define LED flash rate in Hz.  */
enum {LED_FLASH_RATE = 100};


/*
    This test app is the faithful blinky program. It works as follows:

    1. set the LED pin as an output low (turns on the LED)

    2. initialize a pacer for 200 Hz

    3. wait for the next pacer tick.

    4. if this is the 50th pacer tick then toggle the LED.

    5. go to step 3.

    Suggestions:
    * Add more LEDs
    * Blink interesting patterns (S-O-S for example)
    * Make two LEDs blink at two separate frequencies.
*/


int
main (void)
{
    uint8_t flash_ticks;

    piezo_cfg_t piezo_cfg = { BUZZER_PIO };
    piezo_t piezo = piezo_init(&piezo_cfg);
    bool flag = true;

    /* Configure LED PIO as output.  */
    pio_config_set (LED1_PIO, PIO_OUTPUT_LOW);
    pio_config_set (LED2_PIO, PIO_OUTPUT_HIGH);

    pacer_init (LOOP_POLL_RATE);
    flash_ticks = 0;

    while (1)
    {
        /* Wait until next clock tick.  */
        pacer_wait ();

        flash_ticks++;
        if (flash_ticks >= LOOP_POLL_RATE / (LED_FLASH_RATE * 2))
        {
            flash_ticks = 0;

            /* Toggle LED.  */
            pio_output_toggle (LED1_PIO);
            pio_output_toggle (LED2_PIO);
            if (flag) {
                piezo_set(piezo, 1);
            } else {
                piezo_set(piezo, 0);
            }
            flag = !flag;

        }
    }

}
