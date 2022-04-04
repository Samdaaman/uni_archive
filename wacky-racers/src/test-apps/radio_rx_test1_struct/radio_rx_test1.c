/* File:   radio_rx_test1.c
   Author: M. P. Hayes, UCECE
   Date:   24 Feb 2018
   Descr:
*/
#include "nrf24.h"
#include "usb_serial.h"
#include "pio.h"
#include "delay.h"

static void panic(void)
{
    while (1) {
        pio_output_toggle(LED1_PIO);
        pio_output_toggle(LED2_PIO);
        delay_ms(400);
    }
}

int main (void)
{
    spi_cfg_t nrf_spi = {
        .channel = 0,
        .clock_speed_kHz = 1000,
        .cs = RADIO_CS_PIO,
        .mode = SPI_MODE_0,
        .cs_mode = SPI_CS_MODE_FRAME,
        .bits = 8,
    };
    nrf24_t *nrf;
    spi_t spi;
    usb_cdc_t usb_cdc;

    /* Configure LED PIO as output.  */
    pio_config_set(LED1_PIO, PIO_OUTPUT_LOW);
    pio_config_set(LED2_PIO, PIO_OUTPUT_LOW);

    // Create non-blocking tty device for USB CDC connection.
    usb_serial_init(NULL, "/dev/usb_tty");

    freopen("/dev/usb_tty", "a", stdout);
    freopen("/dev/usb_tty", "r", stdin);

    spi = spi_init(&nrf_spi);
    nrf = nrf24_create(spi, RADIO_CE_PIO, RADIO_IRQ_PIO);
    if (!nrf)
        panic();

    // initialize the NRF24 radio with its unique 5 byte address
    if (!nrf24_begin(nrf, 15, 0x0101010203, 32))
        panic();
    if (!nrf24_listen(nrf))
        panic();

    while (1)
    {
        char buffer[32];

        if (nrf24_read(nrf, buffer, sizeof(buffer))) {
            printf("%s\n", buffer);
            fflush(stdout);
            pio_output_toggle(LED2_PIO);
            pio_output_toggle(LED1_PIO);
        }
    }
}
