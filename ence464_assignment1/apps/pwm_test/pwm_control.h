#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H

#include <stdint.h>

//Setup Call
void pwm_init(void);

//PWM Output Toggle Functions (bool)
void toggle_main_PWM_output(bool main_output);
void toggle_tail_PWM_output(bool tail_output);

 
//PWM Duty Cycle Control (Int between 0 - 100)
void pwm_set_main_rotor_dutycycle(uint8_t speed);
void pwm_set_tail_rotor_dutycycle(uint8_t speed);



#endif