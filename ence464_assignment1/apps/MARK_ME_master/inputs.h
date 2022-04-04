#ifndef INPUTS_H
#define INPUTS_H

#include <stdint.h>
#include <semphr.h>

extern SemaphoreHandle_t inputs_quad_reference_reset_semaphore;
extern bool inputs_quad_reference_reset;


/**
 * Copys the rotary encoder value from the ISR variable into a safe to read vairable
 */
int16_t inputs_quad_get(void);


/**
 * ISR that triggers on a the incremental encoders GPIO rising edge 
 */
void inputs_quad_interrupt(void);


/**
 * Initialization function to setup the GPIO pins and interupt for the incremental encoder counts 
 */
void inputs_quad_init(void);


/**
 *  ISR that triggers on a the index encoders GPIO rising edge 
 */
void reset_reference_interrupt(void);


/**
 * Initialization function to setup the GPIO pins and interupt for the index encoder count
 */
void inputs_quad_enable_reference_reset(void);


/**
 * Callback ISR for when the the ADC has finished processing a value 
 */
void adc_done_isr(void);


/**
 * Initializes the ADC peripherals and sets up the interrupt callback method 
 */
void inputs_adc_init(void);


/**
 * ISR for when the timer interupt triggers
 */
void timer_init(void);


/**
 * Initializes the Timer Peripheral
 */
void timer_init();


/**
 * Gets the lastest adc value from the safe to read adc variable
 */
uint16_t inputs_adc_get_raw(void);

#endif