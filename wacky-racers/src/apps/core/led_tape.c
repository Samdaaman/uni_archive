#include "pio.h"
#include "target.h"
#include "pacer.h"
#include "ledbuffer.h"

#define NUM_LEDS 27

static ledbuffer_t* leds;

static bool leds_stopped = true;

void led_tape_initialise(void)
{
    leds = ledbuffer_init(LEDTAPE_PIO, NUM_LEDS);
    ledbuffer_write(leds);
}

void led_tape_poll(void)
{
    static int count = 0;
    leds_stopped = false;

    ledbuffer_clear(leds);
        
    for (int i = 0; i < NUM_LEDS; i++)
    {
        ledbuffer_set(leds, count, 0, 255, 0);
        ledbuffer_set(leds, (count + 17) % 27, 255, 0, 0);
        ledbuffer_set(leds, 26 - count, 0, 0, 255);
        // ledbuffer_set(leds, 26 - ((count + 12) % 27), 0, 255, 0);
    }

    

    ledbuffer_write(leds);

    count++;
    if (count == NUM_LEDS) {
        count = 0;
    }
}

void led_tape_stop(void)
{
    if (!leds_stopped)
    {
        ledbuffer_clear(leds);
        ledbuffer_write(leds);
        leds_stopped = true;
    }
}