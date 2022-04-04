#ifndef BUMPER_H
#define BUMPER_H

#include "motors.h"

#ifdef __cplusplus
extern "C" {
#endif
    void bumper_initialise(void);
    void bumper_poll(void);

#ifdef __cplusplus
}
#endif    
#endif