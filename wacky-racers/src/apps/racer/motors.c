#include "motors.h"
#include "pwm.h"
#include "pio.h"



//defines 

#define PWM_FREQ_HZ 8e2 
#define SERVO_PWM_FREQ_HZ 50 

//Servo Positions
#define CAN_ON 19
#define CAN_OFF 11

static pwm_t left_pwm;
static pwm_t right_pwm;
static pwm_t servo_pwm;


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

//Set up the PWM for Servo
static const pwm_cfg_t servo_pwm_cfg =
{ 
    .pio = MOTOR_LEFT_PWM_PIO,
    .period = PWM_PERIOD_DIVISOR (50),
    .duty = PWM_DUTY_DIVISOR (50, 14),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_LOW,
    .stop_state = PIO_OUTPUT_LOW
};


void motors_initialise(void)
{
    //Setup and start the pwm A 
    left_pwm = pwm_init (&left_pwm_cfg);
    pwm_channels_start (pwm_channel_mask (left_pwm));
    
    //Setup and start the pwm B
    right_pwm = pwm_init (&right_pwm_cfg);
    pwm_channels_start (pwm_channel_mask (right_pwm));

    //Setup and start the servo pwm 
    servo_pwm = pwm_init (&servo_pwm_cfg);
    pwm_channels_start (pwm_channel_mask (servo_pwm));


    //Configure the sleep pin to be high to enable the driver
    pio_config_set (U2_MOTOR_SLEEP_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (MOTOR_SLEEP_PIO, PIO_OUTPUT_HIGH);

    //Set mode to be high, idk which way this will make the motor spin
    pio_config_set (U2_MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (U2_MOTOR_RIGHT_MODE_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_HIGH);
    
}


uint8_t old_left_value = -1;
uint8_t old_right_value = -1;

void motors_poll(payload_h2r_t payload_h2r) {
    if (payload_h2r.motor_left != old_left_value){

        if (payload_h2r.motor_left >= 50){
            pio_config_set (U2_MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_HIGH);
            pwm_duty_ppt_set (left_pwm, (100 - payload_h2r.motor_left)*20);
        }
        else {
            pio_config_set (U2_MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_LOW);
            pwm_duty_ppt_set (left_pwm, (50 - payload_h2r.motor_left)*20);
        }

        old_left_value = payload_h2r.motor_left;
    }

    if (payload_h2r.motor_right != old_right_value){
        if (payload_h2r.motor_right >= 50){
            pio_config_set (U2_MOTOR_RIGHT_MODE_PIO, PIO_OUTPUT_HIGH);
            pwm_duty_ppt_set (right_pwm, (100 - payload_h2r.motor_right)*20);
        }
        else {
            pio_config_set (U2_MOTOR_RIGHT_MODE_PIO, PIO_OUTPUT_LOW);
            pwm_duty_ppt_set (right_pwm, (50 - payload_h2r.motor_right)*20);
        }
        old_right_value = payload_h2r.motor_right;
    }
}


void motors_stop(void){
    //Stop Left Motor
    pio_config_set (U2_MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_LOW);
    pwm_duty_ppt_set (left_pwm, 0);

    //Stop Right Motor
    pio_config_set (U2_MOTOR_RIGHT_MODE_PIO, PIO_OUTPUT_LOW);
    pwm_duty_ppt_set (right_pwm, 0);

    pwm_stop (left_pwm);
    pwm_stop (right_pwm);
}

void motors_start(void){
    old_left_value = -1;
    old_right_value = -1;
    pwm_start (left_pwm);
    pwm_start (right_pwm);
}

static bool isCanOn = false;

bool servo_get_canOn(void)
{
    return isCanOn;
}

void servo_toggle(void){
    if (isCanOn){
        pwm_duty_ppt_set (servo_pwm, (94.5*10));
        isCanOn = false;
    }else{
        pwm_duty_ppt_set (servo_pwm, (90.5 *10));
        isCanOn = true;
    }
}

