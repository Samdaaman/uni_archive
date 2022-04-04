/**
 * @file calibration.h
 * @author Group 16
 * @brief Module that calibrates the height and yaw sensors for the helirig project 
 */


#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdbool.h>
#include <stdint.h>

#include <semphr.h>


/**
 * When the calibration_height is completed this semaphore is given (height controller starts)
 */
extern SemaphoreHandle_t calibration_completed_height_semaphore;


/**
 * When the calibration_yaw is completed this is given (yaw controller starts)
 */
extern SemaphoreHandle_t calibration_completed_yaw_semaphore;


/**
 * The lower [height based] bound for the adc in raw units (about 3000)
 */
extern int32_t calibration_adc_lower;


/**
 * The upper [height based] bound for the adc in raw units (about 1700)
 */
extern int32_t calibration_adc_upper;


/**
 * @brief Task for polling the calibration routine
 * @param args not used
 */
void calibration_poll(void* args);


#endif