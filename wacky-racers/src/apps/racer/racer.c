#include "target.h"
#include "core.h"
#include "delay.h"
#include "nrf24.h"
#include "pio.h"
#include "comms.h"


/* File:   pwm_test2.cd
   Author: M. P. Hayes, UCECE
   Date:   15 April 2013
   Descr:  This example starts two channels simultaneously; one inverted
           with respect to the other.
*/
#include "pwm.h"
#include "pio.h"
#include "target.h"
#include "pacer.h"
#include <stdio.h>
#include "sys.h"
#include "delay.h"
#include "usb_serial.h"
#include <fcntl.h>
#include "button.h"
#include "adc.h"
#include "motors.h"
#include "bumper.h"
#include "low_voltage.h"
#include "led.h"
#include "led_tape.h"


payload_h2r_t payload_h2r;


enum {LOOP_POLL_RATE = 200};
enum {LED_FLASH_RATE = 10};

//Button Testing
#define BUTTON_POLL_RATE 100
static const button_cfg_t button2_cfg =
{
    .pio = BUTTON_PIO
};

/* Define LED configuration.  */
static const led_cfg_t led2_cfg =
{
    .pio = LED2_PIO,
    .active = 1
};


//Setup Serial
static usb_serial_cfg_t usb_serial_cfg =
{
    .read_timeout_us = 1,
    .write_timeout_us = 1,
};



int main (void)
{
    //Testing Buttons
    button_t button2;
    led_t led2;
        /* Initialise LED.  */
    led2 = led_init (&led2_cfg);

    /* Turn on LED.  */
    led_set (led2, 0);
    button2 = button_init (&button2_cfg);
    button_poll_count_set (BUTTON_POLL_COUNT (BUTTON_POLL_RATE));
    
    bumper_initialise();
    core_initialise();
    core_initialise_usb_serial();

    delay_ms(200); // It's good to wait for usb serial
    comms_initialise();
    adc_voltage_setup();
    motors_initialise();
    led_tape_initialise();
    pio_config_set (LED1_PIO, PIO_OUTPUT_LOW);
    pacer_init (LOOP_POLL_RATE);
    int flash_ticks = 0;
    unsigned int i;
    payload_h2r.motor_right = 50;
    payload_h2r.motor_left = 50;
    while (1)
    {
        i++;
        if ((i%500) == 0){
            while (is_voltage_low() && (payload_h2r.motor_right <  60) && (payload_h2r.motor_right > 40) && (payload_h2r.motor_left <  60) && (payload_h2r.motor_left > 40)){
                pio_output_set(LED1_PIO, 0);
                motors_stop();
                led_tape_stop();
            }
        }
        bumper_poll();
        if (comms_receive_h2r_payload(&payload_h2r)) {
            motors_poll(payload_h2r);

            static int deploy_string_counter = 0;
            if (payload_h2r.should_deploy_string)
            {
                if (deploy_string_counter == 30)
                {
                    // deploy the string
                    servo_toggle();
                    deploy_string_counter = 0;
                }
                else
                {
                    deploy_string_counter ++;
                }
            }
            else
            {
                deploy_string_counter = 0;
                if (servo_get_canOn())
                {
                    servo_toggle();
                }
            }
         }

        //Test Buttons
        button_poll (button2);
        if (button_pushed_p (button2))
        {
            led_toggle (led2);
            servo_toggle();    
        }
        if (core_sleep_poll())
        {
            comms_shutdown();
            led_tape_stop();
            motors_stop();
            core_go_to_sleep();
            core_panic(); // this will never actually happen but good in case the sleep doesn't actually work
        }
        else if ((i%5) == 0){
            led_tape_poll();
        }
        delay_ms(10);
        pacer_wait();
        flash_ticks++;

        if (flash_ticks >= LOOP_POLL_RATE / (LED_FLASH_RATE * 2))
        {
            flash_ticks = 0;

            /* Toggle LED.  */
            pio_output_toggle (LED1_PIO);
        }

    }
}
