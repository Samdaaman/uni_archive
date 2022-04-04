/* File:   button_test2.c
   Author: M. P. Hayes, UCECE
   Date:   3 May 2021
   Descr:  Simple button test demo
*/
#include "mcu.h"
#include "led.h"
#include "pio.h"
#include "pacer.h"
#include "button.h"

#define BUTTON_POLL_RATE 100


/* Define LED configuration.  */
static const led_cfg_t led1_cfg =
{
    .pio = LED1_PIO,
    .active = 1
};

static const button_cfg_t button1_cfg =
{
    .pio = BUTTON_PIO
};

int
main (void)
{
    led_t led1;
    button_t button1;

    /* Initialise LED.  */
    led1 = led_init (&led1_cfg);

    /* Turn on LED.  */
    led_set (led1, 1);

    /* Initialise button.  */
    button1 = button_init (&button1_cfg);

    button_poll_count_set (BUTTON_POLL_COUNT (BUTTON_POLL_RATE));

    pacer_init (BUTTON_POLL_RATE);

    while (1)
    {
        pacer_wait ();

        button_poll (button1);

        if (button_pushed_p (button1))
        {
            led_toggle (led1);        
        }
    }
    return 0;
}

