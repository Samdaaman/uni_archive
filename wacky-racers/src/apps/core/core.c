#include "core.h"
#include <pio.h>
#include <stdarg.h>
#include "usb_serial.h"
#include "delay.h"
#include "adc.h"
#include "mcu_sleep.h"

int limit_int(int min, int max, int value)
{
    return value < min ? min : value > max ? max : value;
}

void core_printf(const char * format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}

void core_panic(void)
{
    while (1) {
        pio_output_toggle(LED1_PIO);
        pio_output_toggle(LED2_PIO);
        delay_ms(400);
    }
}

void core_initialise_usb_serial(void)
{
    usb_serial_cfg_t usb_serial_cfg =
    {
        .read_timeout_us = 1,
        .write_timeout_us = 1,
    };
    // Create non-blocking tty device for USB CDC connection.
    usb_serial_init (&usb_serial_cfg, "/dev/usb_tty");

    freopen ("/dev/usb_tty", "a", stdout);
    freopen ("/dev/usb_tty", "r", stdin);

    delay_ms(100);
}

static void wait_for_sleep_button_release(void)
{
    int counter = 0;
    while (counter < 10)
    {
        if (pio_input_get(SLEEP_BUTTON_PIO))
        {
            counter ++;
        }
        else
        {
            counter = 0;
        }

        delay_ms(10);
    }
}

void core_initialise(void)
{
    pio_config_set(DIP_0_PIO, PIO_PULLUP);
    pio_config_set(DIP_1_PIO, PIO_PULLUP);
    pio_config_set(DIP_2_PIO, PIO_PULLUP);
    pio_config_set(DIP_3_PIO, PIO_PULLUP);

    pio_config_set(LED1_PIO, PIO_OUTPUT_HIGH);
    pio_config_set(LED2_PIO, PIO_OUTPUT_HIGH);

    pio_config_set(SLEEP_BUTTON_PIO, PIO_PULLUP);

    wait_for_sleep_button_release();
}


int core_read_dip_value(void)
{
    return (pio_input_get(DIP_0_PIO) << 3) + (pio_input_get(DIP_1_PIO) << 2) + (pio_input_get(DIP_2_PIO) << 1) + pio_input_get(DIP_3_PIO);
}

bool core_sleep_poll(void)
{
    return !pio_input_get(SLEEP_BUTTON_PIO);
}

void core_go_to_sleep(void)
{
    wait_for_sleep_button_release();

    for (int i = 0; i < 20; i++) {
        pio_output_toggle(LED1_PIO);
        delay_ms(50);
    }

    // mcu_sleep_wakeup_cfg_t sleep_wakeup_cfg = {
    //     .active_high = false,
    //     .pio = PA2_PIO
    // };
    // mcu_sleep_wakeup_set(&sleep_wakeup_cfg);

    pio_output_set(LED1_PIO, 1);
    pio_output_set(LED2_PIO, 1);
    
    mcu_sleep_cfg_t sleep_mode_cfg = {
        .mode = MCU_SLEEP_MODE_WAIT
    };

    mcu_sleep(&sleep_mode_cfg);
}


