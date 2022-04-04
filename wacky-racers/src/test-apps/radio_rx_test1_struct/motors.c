#include "motors.h"
#include "pwm.h"


#define PWM_FREQ_HZ 8e2 

//Set up the PWM for A Motor
static const pwm_cfg_t left_pwm_cfg =
{
    .pio = U2_MOTOR_LEFT_PWM_PIO,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 100),
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


void motors_initialise()
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
    pio_config_set (U2_MOTOR_SLEEP_PIO, PIO_OUTPUT_HIGH);

    //Set mode to be high, idk which way this will make the motor spin
    pio_config_set (U2_MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (U2_MOTOR_RIGHT_MODE_PIO, PIO_OUTPUT_HIGH);
}



void set_motor_pwm(payload_h2r_t payload)
{
    //Set the left motor first
    //Condition for forward movement
    if (payload.motor_left >= 128){
        pio_config_set (U2_MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_HIGH);
        pwm_duty_ppt_set (left_pwm, ((payload.motor_left - 126) * 2));
    }

    //Condition for backwards movement
    if (payload.motor_left < 128){
        pio_config_set (U2_MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_LOW);
        pwm_duty_ppt_set (left_pwm, ((payload.motor_left) * 2));
    }


    //Next , set the right motor first
    //Condition for forward movement
    if (payload.motor_right >= 128){
        pio_config_set (U2_MOTOR_RIGHT_MODE_PIO, PIO_OUTPUT_HIGH);
        pwm_duty_ppt_set (right_pwm, ((payload.motor_right - 126) * 2));
    }

    //Condition for backwards movement
    if (payload.motor_right < 128){
        pio_config_set (U2_MOTOR_RIGHT_MODE_PIO, PIO_OUTPUT_LOW);
        pwm_duty_ppt_set (right_pwm, ((payload.motor_right) * 2));
    }

}