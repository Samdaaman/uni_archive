#include "pit.h"
#include "bits.h"

/* SysTick is a 24 bit down counter that resets to the preloaded value
   in the SYST_RVR register.
   
   We trigger an IRQ whenever the timer reaches 0, and then increment
   a counter on each interrupt, giving us a 1ms counter
   */

volatile uint32_t pit_ticks = 0;

void _systick_handler (void) {
    pit_ticks++;
}

void
pit_start (void)
{
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk; 
}


void
pit_stop (void)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; 
}


static uint32_t pit_period_set (uint32_t period)
{
    SysTick->LOAD = period;

    return period;
}


pit_tick_t pit_get (void)
{
    return pit_ticks;
}


/** Wait until specified time:
    @param when time to sleep until
    @return current time.  */
pit_tick_t pit_wait_until (pit_tick_t when)
{
    while (1)
    {
        pit_tick_t now = pit_get();
        int32_t diff = (when - now);
        if (diff < 0)
            return now;
    }
}


/** Wait for specified period:
    @param period how long to wait
    @return current time.  */
pit_tick_t pit_wait (pit_tick_t period)
{
    return pit_wait_until (pit_get () + period);
}


int
pit_init (void)
{
    /* Set maximum period.  */
    pit_period_set (PIT_RATE);
    pit_start ();
    return 1;
}
