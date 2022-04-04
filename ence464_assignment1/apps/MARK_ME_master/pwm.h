#ifndef PWM_H
#define PWM_H

#include <stdbool.h>

/**
 * Initialization function that sets up the PWM
 */
void pwm_init(void);


/**
 * Changes the PWM dutycycle of the main rotor. Can only be called after the PWM peripherals have been set up
 */
void pwm_set_main_rotor_dutycycle(uint8_t speed);


/**
 * Changes the PWM dutycycle of the tail rotor. Can only be called after the PWM peripherals have been set up
 */
void pwm_set_tail_rotor_dutycycle(uint8_t speed);


/**
 * Function that's called after pwm_init to start the main rotor PWM output
 */
uint8_t enable_pwm_main_output();


/**
 * Function that's called after pwm_init to start the tail rotor PWM output
 */
uint8_t enable_pwm_tail_output();

#endif