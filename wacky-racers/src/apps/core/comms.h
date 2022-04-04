#ifndef COMMS_H
#define COMMS_H

#include "stdint.h"
#include "stdbool.h"

typedef struct {
    uint8_t motor_left;
    uint8_t motor_right;
    bool should_deploy_string;
} payload_h2r_t;

#ifdef __cplusplus
extern "C" {
#endif
    void comms_initialise(void);
    void comms_shutdown(void);
    bool comms_send_h2r_payload(payload_h2r_t payload);
    bool comms_receive_h2r_payload(payload_h2r_t* payload_p);
#ifdef __cplusplus
}
#endif
#endif

