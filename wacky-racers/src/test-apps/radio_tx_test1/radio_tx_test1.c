/* File:   radio_tx_test1.c
   Author: M. P. Hayes, UCECE
   Date:   24 Feb 2018
   Descr:
*/
#include "nrf24.h"
#include "pio.h"
#include "pacer.h"
#include "stdio.h"
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
    uint8_t count = 0;
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

    /* Configure LED PIO as output.  */
    pio_config_set(LED1_PIO, PIO_OUTPUT_HIGH);
    pio_config_set(LED2_PIO, PIO_OUTPUT_LOW);
    pacer_init(10);

#ifdef RADIO_PWR_EN
    pio_config_set(RADIO_PWR_EN, PIO_OUTPUT_HIGH);
#endif

    spi = spi_init(&nrf_spi);
    nrf = nrf24_create(spi, RADIO_CE_PIO, RADIO_IRQ_PIO);
    if (!nrf)
        panic();

    // initialize the NRF24 radio with its unique 5 byte address
    if (!nrf24_begin(nrf, 4, 0x0123456789, 32))
        panic();

    while (1)
    {
        char buffer[32];

        pacer_wait();
        pio_output_toggle(LED2_PIO);
        pio_output_set(LED1_PIO, 1);

        sprintf (buffer, "YEET %d\r\n", count++);

        if (! nrf24_write(nrf, buffer, sizeof (buffer)))
            pio_output_set(LED1_PIO, 0);
        else
            pio_output_set(LED1_PIO, 1);
    }
}

