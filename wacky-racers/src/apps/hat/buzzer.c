#include "stdint.h"
#include "buzzer.h"
#include "delay.h"

#define DUR 300
#define GAP 50

#define KEY 2

#define B4 493 * KEY
#define Cs5 554 * KEY
#define Ds5 622 * KEY
#define Fs5 740 * KEY
#define Gs5 830 * KEY

static void play_sound(int freq, int duration)
{
    uint16_t i;

    for (i = 0; i < duration * freq * 2 / 1000; i++)
    {    
        pio_output_toggle (BUZZER_PIO);   
        DELAY_US (500000 / freq);  
    } 
}

static void buzzer_play(void)
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

void buzzer_button_poll(void)
{
    if (!pio_input_get(BUZZER_BUTTON_PIO))
    {
        buzzer_play();
    }
}

void buzzer_initialise(void)
{
    pio_config_set(BUZZER_PIO, PIO_OUTPUT_LOW);
    pio_config_set(BUZZER_BUTTON_PIO, PIO_PULLUP);
}

void buzzer_die(void)
{
    play_sound(Gs5, DUR);
    delay_ms(GAP);
    play_sound(Ds5, DUR);
    delay_ms(GAP);
    play_sound(B4, DUR);
    delay_ms(GAP);
    pio_output_set(BUZZER_PIO, 0);
}