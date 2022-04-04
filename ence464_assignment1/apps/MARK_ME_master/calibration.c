/**
 * @file calibration.c
 * @author Group 16
 * @brief Module that calibrates the height and yaw sensors for the helirig project 
 */


#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "buttons.h"
#include "debug.h"
#include "inputs.h"


#define CALIBRATION_LOOP_DELAY 100 // run the calibration routine at 10Hz
#define CALIBRATION_DENOISE_ITERATIONS 5 // number of iterations to wait while the heli is at it's lowest value 
#define CALIBRATION_PWM_STEP 5 // used for calibrating the hold value
#define CALIBRATION_ADC_NOISE 30 // aproximate peak noise from the ADC (conservatively large)
#define CALIBRATION_ADC_RANGE 1310 // 12 bit ADC so a 0.8V range corresponds to 2^13 * 0.8/5


typedef enum {
    Calibration_State_Height,
    Calibration_State_Yaw,
    Calibration_State_Done,
} Calibration_State;


/**
 * When the calibration_height is completed this semaphore is given (height controller starts)
 */
SemaphoreHandle_t calibration_completed_height_semaphore;

/**
 * When the calibration_yaw is completed this is given (yaw controller starts)
 */
SemaphoreHandle_t calibration_completed_yaw_semaphore;

/**
 * The lower [height based] bound for the adc in raw units (about 3000)
 */
int32_t calibration_adc_lower = 0;

/**
 * The upper [height based] bound for the adc in raw units (about 1700)
 */
int32_t calibration_adc_upper = 0;


/**
 * @brief Task for polling the calibration
 * @param args not used
 */
void calibration_poll(void* args)
{
    (void)args; // unused

    TickType_t wake_time = xTaskGetTickCount();

    Calibration_State calibration_state = Calibration_State_Height;
    Calibration_State last_calibration_state = calibration_state - 1;

    while (true)
    {
        int32_t current_height_value = inputs_adc_get_raw();

        if (last_calibration_state != calibration_state)
        {
            if (calibration_state == Calibration_State_Done)
            {
                debug_printf("Calibration done\r\n");
                vTaskDelete(NULL);
            }

            last_calibration_state = calibration_state;
            debug_printf("Calibration step %i/%i\r\n", calibration_state + 1, Calibration_State_Done);
        }

        switch (calibration_state)
        {
        // Wait for the helicopter to arrive at it's lowest height and stay there (for CALIBRATION_DENOISE_ITERATIONS iterations)
        case Calibration_State_Height:
        {
            static int8_t counter = 0;
            static int32_t lowest_height = 0;

            if (lowest_height != 0 && current_height_value < lowest_height && current_height_value > lowest_height - CALIBRATION_ADC_NOISE)
            {
                if (counter++ > CALIBRATION_DENOISE_ITERATIONS)
                {
                    calibration_adc_lower = lowest_height + CALIBRATION_ADC_NOISE;
                    calibration_adc_upper = lowest_height - CALIBRATION_ADC_RANGE;
                    debug_printf("calibration_adc_lower=%i\r\n", calibration_adc_lower);
                    debug_printf("calibration_adc_upper=%i\r\n", calibration_adc_upper);

                    // Enable the height controller
                    xSemaphoreGive(calibration_completed_height_semaphore);
                    calibration_state ++;
                }
            }
            else
            {
                counter = 0;
                if (lowest_height == 0 || current_height_value > lowest_height)
                {
                    lowest_height = current_height_value;
                }
            }
            break;
        }
        // Enable the reference reset interrupt for the yaw and wait for it to trigger
        case Calibration_State_Yaw:
        {
            inputs_quad_enable_reference_reset(); 

            // Wait for the quadrature encode to be zeroed
            xSemaphoreTake(inputs_quad_reference_reset_semaphore, portMAX_DELAY);
            
            // Enable yaw controller and buttons
            xSemaphoreGive(calibration_completed_yaw_semaphore);
            xSemaphoreGive(buttons_enable_semaphore);
            calibration_state++;
            break;
        }
        case Calibration_State_Done:
        break; // this never occurs as we never get here as the task is deleted. Included to suppress compiler warning
        }

        vTaskDelayUntil(&wake_time, CALIBRATION_LOOP_DELAY);
    }
}
