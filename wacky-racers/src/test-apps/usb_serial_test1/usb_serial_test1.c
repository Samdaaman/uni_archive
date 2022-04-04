#include <stdio.h>
#include "usb_serial.h"
#include "pio.h"
#include "sys.h"
#include "pacer.h"
#include "delay.h"
#include <fcntl.h>

#define HELLO_DELAY 500

static usb_serial_cfg_t usb_serial_cfg =
{
    .read_timeout_us = 1,
    .write_timeout_us = 1,
};


int main (void)
{
    usb_cdc_t usb_cdc;
    int i = 0;

    pio_config_set (LED1_PIO, PIO_OUTPUT_LOW);
    pio_config_set (LED2_PIO, PIO_OUTPUT_HIGH);

    // Create non-blocking tty device for USB CDC connection.
    usb_serial_init (&usb_serial_cfg, "/dev/usb_tty");

    freopen ("/dev/usb_tty", "a", stdout);
    freopen ("/dev/usb_tty", "r", stdin);

    while (1)
    {
        printf ("Hello world %d\n", i++);
        fflush (stdout);

        pio_output_toggle(LED1_PIO);
        pio_output_toggle(LED2_PIO);

        delay_ms (HELLO_DELAY);
    }
}
