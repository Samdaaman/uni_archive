/**
 * @file control.h
 * @author Group 16
 * @brief Methods for utilising height and yaw PI controllers for the HeliRig project
 */


#ifndef CONTROL_H
#define CONTROL_H


#include <stdint.h>


/**
 * @brief Task for polling the yaw PI controller
 * @param args not used
 */
void control_yaw_poll(void *args);


/**
 * @brief Task for polling the height PI controller
 * @param args not used
 */
void control_height_poll(void *args);


#endif