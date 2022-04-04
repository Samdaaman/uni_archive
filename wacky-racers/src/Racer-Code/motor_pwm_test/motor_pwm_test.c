/* File:   pwm_test2.c
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


#define PWM_FREQ_HZ 8e2 


//Set up blinky
enum {LOOP_POLL_RATE = 200};

/* Define LED flash rate in Hz.  */
enum {LED_FLASH_RATE = 1};



//Setup Serial
static usb_serial_cfg_t usb_serial_cfg =
{
    .read_timeout_us = 1,
    .write_timeout_us = 1,
};



//Set up the PWM for A Motor
static const pwm_cfg_t left_pwm_cfg =
{
    .pio = MOTOR_LEFT_PWM_PIO,
    .period = PWM_PERIOD_DIVISOR (50),
    .duty = PWM_DUTY_DIVISOR (50, 100),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_LOW,
    .stop_state = PIO_OUTPUT_LOW
};

//Set up the PWM for B Motor
static const pwm_cfg_t right_pwm_cfg =
{
    .pio = U2_MOTOR_RIGHT_PWM_PIO,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 100),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_LOW,
    .stop_state = PIO_OUTPUT_LOW
};



int
main (void)
{
    //Setup and start the pwm A 
    pwm_t left_pwm;
    left_pwm = pwm_init (&left_pwm_cfg);
    pwm_channels_start (pwm_channel_mask (left_pwm));
    
    //Setup and start the pwm B
    pwm_t right_pwm;
    right_pwm = pwm_init (&right_pwm_cfg);
    pwm_channels_start (pwm_channel_mask (right_pwm));


    //Configure the sleep pin to be high to enable the driver
    pio_config_set (MOTOR_SLEEP_PIO, PIO_OUTPUT_HIGH);

    //Set mode to be high, idk which way this will make the motor spin
    pio_config_set (MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (U2_MOTOR_RIGHT_MODE_PIO, PIO_OUTPUT_HIGH);



    //Setup usb
    usb_cdc_t usb_cdc;
    
    // Create non-blocking tty device for USB CDC connection.
    usb_serial_init (&usb_serial_cfg, "/dev/usb_tty");

    freopen ("/dev/usb_tty", "a", stdout);
    freopen ("/dev/usb_tty", "r", stdin);


    //USB input variables
    int inPwmVal = 0;
    int motor_select = 2;
    int old_motor_select = 2;
    int oldPwmVal = 0;


    //Set up blinky 
    uint8_t flash_ticks;

    /* Configure LED PIO as output.  */
    pio_config_set (LED1_PIO, PIO_OUTPUT_LOW);
    pio_config_set (LED2_PIO, PIO_OUTPUT_HIGH);

    pacer_init (LOOP_POLL_RATE);
    flash_ticks = 0;

    while (1)
    {

    //PWM Control
    scanf("%d,%d", &motor_select, &inPwmVal);
    if ((inPwmVal != oldPwmVal) || (motor_select != old_motor_select)){
        if (motor_select == 0){
            if (inPwmVal >= 50){
                pio_config_set (U2_MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_HIGH);
                pwm_duty_ppt_set (left_pwm, (100 - inPwmVal)*20);
            }
            else if (inPwmVal < 50){
                pio_config_set (U2_MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_LOW);
                pwm_duty_ppt_set (left_pwm, (inPwmVal)*20);
            }
        }
        if (motor_select == 1){
            if (inPwmVal >= 50){
                pio_config_set (U2_MOTOR_RIGHT_MODE_PIO, PIO_OUTPUT_HIGH);
                pwm_duty_ppt_set (right_pwm, (100 - inPwmVal)*20);
            }
            else if (inPwmVal < 50){
                pio_config_set (U2_MOTOR_RIGHT_MODE_PIO, PIO_OUTPUT_LOW);
                pwm_duty_ppt_set (right_pwm, (inPwmVal)*20);
            }
        }

        oldPwmVal = inPwmVal;
        old_motor_select = motor_select;
        printf ("Motor %d PWM set to: %d\n", motor_select, inPwmVal);
        fflush(stdout);
        }
        fflush(stdout);

        //Blinky
        /* Wait until next clock tick.  */
        pacer_wait ();

        flash_ticks++;

        if (flash_ticks >= LOOP_POLL_RATE / (LED_FLASH_RATE * 2))
        {
            flash_ticks = 0;

            /* Toggle LED.  */
            pio_output_toggle (LED1_PIO);
            pio_output_toggle (LED2_PIO);
            fflush(stdout);
        }

    }

return 0;
}