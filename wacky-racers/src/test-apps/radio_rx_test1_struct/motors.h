#ifndef MOTORS_H
#define MOTORS_H

#include "stdint.h"
#include "stdbool.h"


typedef struct {
    uint8_t motor_left;
    uint8_t motor_right;
} payload_h2r_t;



#ifdef __cplusplus
extern "C" {
#endif
    void motors_initialise();
    void set_motor_pwm(payload_h2r_t payload);
#ifdef __cplusplus
}
#endif
#endif