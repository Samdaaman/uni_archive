#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

#define CONTROL_HEIGHT_TIMESTEP 10
#define CONTROL_YAW_TIMESTEP 10 // TODO

uint8_t control_yaw_poll(uint8_t actual_yaw, uint8_t desired_yaw);
uint8_t control_height_poll(uint8_t actual_height, uint8_t desired_yaw);

#endif