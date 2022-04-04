#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "comms.h"

#ifdef __cplusplus
extern "C" {
#endif
    void joystick_initialise(void);
    void joystick_shutdown(void);
    payload_h2r_t joystick_read();
    bool joystick_is_pressed(void);
#ifdef __cplusplus
}
#endif    
#endif


