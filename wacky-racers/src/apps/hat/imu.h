#ifndef IMU_H
#define IMU_H

#include "comms.h"

#ifdef __cplusplus
extern "C" {
#endif
    void imu_initialise(void);
    payload_h2r_t imu_read(void);
#ifdef __cplusplus
}
#endif    
#endif


