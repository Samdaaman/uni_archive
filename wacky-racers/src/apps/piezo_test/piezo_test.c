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

/* Define how fast ticks occur.  This must be faster than TICK_RATE_MIN.  */
enum {LOOP_POLL_RATE = 200};

/* Define LED flash rate in Hz.  */
enum {LED_FLASH_RATE = 1};

void play_sound(int freq, int duration)
{
    uint16_t i;

    for (i = 0; i < duration * freq * 2 / 1000; i++)
    {    
        pio_output_toggle (BUZZER_PIO);   
        DELAY_US (500000 / freq);  
    } 
}

void beep ()
{
    uint16_t i;    

    for (i = 0; i < PIEZO_BEEP_SHORT_PERIOD_MS * PIEZO_BEEP_FREQ_KHZ * 2; i++)
    {    
        pio_output_toggle (BUZZER_PIO);   
        DELAY_US (500 / PIEZO_BEEP_FREQ_KHZ);  
    }
    pio_output_set(BUZZER_PIO, 0);
}

int main (void)
{
    pio_config_set (BUZZER_PIO, PIO_OUTPUT_LOW);

    /* Configure LED PIO as output.  */
    pio_config_set (LED1_PIO, PIO_OUTPUT_LOW);
    pio_config_set (LED2_PIO, PIO_OUTPUT_HIGH);

    pacer_init (LOOP_POLL_RATE);

    // /** Generate a short beep,  */
    // void piezo_beep_short (piezo_t piezo)
    // {
    //     uint16_t i;    

    //     for (i = 0; i < PIEZO_BEEP_SHORT_PERIOD_MS * PIEZO_BEEP_FREQ_KHZ * 2; i++)
    //     {    
    //         pio_output_toggle (piezo->pio);   
    //         DELAY_US (500 / PIEZO_BEEP_FREQ_KHZ);  
    //     } 
    // }

    #define DUR 300
    #define GAP 50

    #define KEY 2

    #define B4 493 * KEY
    #define Cs5 554 * KEY
    #define Ds5 622 * KEY
    #define Fs5 740 * KEY
    #define Gs5 830 * KEY

    while (1)
    {
        play_sound(B4, DUR);
        delay_ms(GAP);
        play_sound(B4, DUR);
        delay_ms(GAP);

        play_sound(Cs5, DUR);
        delay_ms(GAP);

        play_sound(Ds5, DUR);
        delay_ms(GAP);
        play_sound(Ds5, DUR);
        delay_ms(GAP);
        play_sound(Ds5, DUR);
        delay_ms(GAP);
        play_sound(Ds5, DUR);
        delay_ms(GAP);
        play_sound(Ds5, DUR);
        delay_ms(GAP);

        play_sound(Fs5, DUR);
        delay_ms(GAP);
        play_sound(Fs5, DUR);
        delay_ms(GAP);
        play_sound(Fs5, DUR);
        delay_ms(GAP);
        play_sound(Fs5, DUR);
        delay_ms(GAP);

        play_sound(Gs5, DUR);
        delay_ms(GAP);
        play_sound(Gs5, DUR);
        delay_ms(GAP);
        play_sound(Gs5, DUR);
        delay_ms(GAP);

        delay_ms(DUR+GAP);
    }
}
