#ifndef BUZZER_H
#define BUZZER_H

#include "pio.h"
#include "target.h"

#ifdef __cplusplus
extern "C" {
#endif
    void buzzer_initialise(void);
    void buzzer_die(void);
    void buzzer_button_poll(void);
#ifdef __cplusplus
}
#endif    
#endif