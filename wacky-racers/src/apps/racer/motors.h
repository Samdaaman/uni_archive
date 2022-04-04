#ifndef MOTORS_H
#define MOTORS_H

#include "comms.h"

#ifdef __cplusplus
extern "C" {
#endif
    void motors_initialise(void);
    void motors_poll(payload_h2r_t payload_h2r);
    void motors_stop(void);
    void motors_start(void);
    void servo_toggle(void);
    bool servo_get_canOn(void);
#ifdef __cplusplus
}
#endif    
#endif