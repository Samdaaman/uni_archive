/* File:   adc_usb_cdc_test1.c
   Author: M. P. Hayes, UCECE
   Date:   3 May 2021
   Descr:  This reads from ADC channels 2 and 4.
           It triggers ADC conversions as each sample is read.
*/
#include <stdio.h>
#include "usb_serial.h"
#include "adc.h"
#include "pacer.h"
#include "delay.h"

#define PACER_RATE 2

#define ADC_CLOCK_FREQ 24000000

static const adc_cfg_t adc_cfg =
{
    .bits = 12,
    .channels = BIT (ADC_CHANNEL_2) | BIT (ADC_CHANNEL_4),
    .trigger = ADC_TRIGGER_SW,
    .clock_speed_kHz = ADC_CLOCK_FREQ / 1000
};

static usb_serial_cfg_t usb_serial_cfg =
{
    .read_timeout_us = 1,
    .write_timeout_us = 1,
};


int main (void)
{
    usb_cdc_t usb_cdc;
    adc_t adc;
    int count = 0;

    //Create non-blocking tty device for USB CDC connection.
    usb_serial_init (&usb_serial_cfg, "/dev/usb_tty");

    freopen ("/dev/usb_tty", "a", stdout);
    freopen ("/dev/usb_tty", "r", stdin);

    adc = adc_init (&adc_cfg);

    pacer_init (PACER_RATE);
    while (1)
    {
        uint16_t data[2];

        The lowest numbered channel is read first.
        adc_read (adc, data, sizeof (data));

        printf ("%3d: %d, %d\n", count, data[0], data[1]);
        printf ("Hello\n");
        fflush (stdout);
        delay_ms(500);
    }
}
