/**
 * @file control.c
 * @author Group 16
 * @brief Methods for utilising height and yaw PI controllers for the HeliRig project
 */


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "buttons.h"
#include "calibration.h"
#include "control.h"
#include "debug.h"
#include "inputs.h"
#include "pwm.h"
#include "display.h"


#define CONTROL_TIMESTEP 100 // 100ms corresponds to 10Hz frequency
#define GAIN_MULTIPLIER 100 // factor to scale the gains by to allow fine tuning
#define CONTROL_HOLD_PWM 50 // approximate hold value for the main rotor (used to preload the integral term)


/**
 * Struct which holds the PI controller configuration for the height and yaw controllers
 * Integral carry is has the units units*ms and timestep has the units ms. 
 * To allow fine tuning of the gains, all of the gains are divided by gain multiplier when applied.
 */
typedef struct {
    const int16_t p_gain;
    const int16_t i_gain;
    const int16_t timestep;
    const int8_t min_value;
    const int8_t max_value;
    int32_t integral_carry_over;
} pi_controller_config_t;


/**
 * @brief Generic PI function used by the height and yaw controllers
 * @param actual_value the measured input value (must be signed int)
 * @param desired_value the desired input value (must be signed int)
 * @param controller_config the controller configuration which contains the gains, integral carry over and min/max output values
 * @return uint8_t the output value from the controller after gains are applied (is also min/max checked aggainst the controller config)
 */
static uint8_t control_generic_poll(int16_t actual_value, int16_t desired_value, pi_controller_config_t *controller_config);


static pi_controller_config_t height_controller_config = {
    .p_gain = 50,
    .i_gain = 20,
    .timestep = CONTROL_TIMESTEP,
    .min_value = 2,
    .max_value = 98,
    .integral_carry_over = 0
};


static pi_controller_config_t yaw_controller_config = {
    .p_gain = 250,
    .i_gain = 50,
    .timestep = CONTROL_TIMESTEP,
    .min_value = 2,
    .max_value = 98,
    .integral_carry_over = 0
};


/**
 * @brief Generic PI function used by the height and yaw controllers
 * @param actual_value the measured input value (must be signed int)
 * @param desired_value the desired input value (must be signed int)
 * @param controller_config the controller configuration which contains the gains, integral carry over and min/max output values
 * @return uint8_t the output value from the controller after gains are applied (is also min/max checked aggainst the controller config)
 */
static uint8_t control_generic_poll(int16_t actual_value, int16_t desired_value, pi_controller_config_t *controller_config)
{
    int16_t error = desired_value - actual_value;
    controller_config->integral_carry_over = controller_config->integral_carry_over + error * (int32_t)controller_config->timestep;
    int32_t result = (error * controller_config->p_gain + controller_config->integral_carry_over * controller_config->i_gain / 1000) / GAIN_MULTIPLIER;
    
    if (result < controller_config->min_value)
    {
        return controller_config->min_value;
    }
    else if (result > controller_config->max_value)
    {
        return controller_config->max_value;
    }
    else
    {
        return result;
    }
}


/**
 * Task for polling the height PI controller
 * First waits for the height calibration to finished, then starts the control loop which: 
 * - Converts the height to a percentage
 * - Uses control_generic_poll to get the main_pwm output
 * - Waits CONTROL_TIMESTEP time so it runs at a constant frequency
 * @param args not used
 */
void control_height_poll(void *args)
{
    (void)args; // unused

    TickType_t wake_time = xTaskGetTickCount();

    // Enable the main rotor's pwm
    int16_t start_main_pwm = enable_pwm_main_output();
    display_send_message(TopDuty, start_main_pwm, 0);

    // Wait for height calibration to be completed
    xSemaphoreTake(calibration_completed_height_semaphore, portMAX_DELAY);

    // Preload the integral term with an approximate hold value
    height_controller_config.integral_carry_over = CONTROL_HOLD_PWM * 1000 * GAIN_MULTIPLIER / height_controller_config.i_gain;
    
    while (true)
    {
        int16_t raw_height = inputs_adc_get_raw();
        int16_t actual_height = (calibration_adc_lower - raw_height) * 100 / (calibration_adc_lower - calibration_adc_upper); // calulate the actual height from raw height
        int16_t desired_height = buttons_get_desired_height();
        int16_t main_pwm = control_generic_poll(actual_height, desired_height, &height_controller_config);
        if (height_controller_config.integral_carry_over < 0) {
            height_controller_config.integral_carry_over = 0; // prevent negative integral wind up
        }
        pwm_set_main_rotor_dutycycle(main_pwm);

        // Update the display
        display_send_message(TopDuty, main_pwm, 0);
        display_send_message(Height, actual_height, desired_height);

        vTaskDelayUntil(&wake_time, CONTROL_TIMESTEP);
    }
}


/**
 * Task for polling the yaw PI controller
 * First waits for the yaw calibration to finished, then starts the control loop which: 
 * - Modifies the raw_yaw value to be within 180 deg of the target yaw (closest path to target)
 * - Uses control_generic_poll to get the tail_pwm output
 * - Waits CONTROL_TIMESTEP time so it runs at a constant frequency
 * @param args not used
 */
void control_yaw_poll(void *args)
{
    (void)args; // unused

    TickType_t wake_time = xTaskGetTickCount();

    // Enable the tail rotor's pwm
    int16_t start_tail_pwm = enable_pwm_tail_output();
    display_send_message(TailDuty, start_tail_pwm, 0);

    // Wait for yaw calibration to be completed
    xSemaphoreTake(calibration_completed_yaw_semaphore, portMAX_DELAY);

    while (true)
    {
        int16_t raw_yaw = inputs_quad_get();
        int16_t desired_yaw = buttons_get_desired_yaw();

        // Make sure the raw_yaw is within 180 deg of the target yaw
        if (raw_yaw - desired_yaw > 180)
        {
            raw_yaw -= 360;
        }
        else if (raw_yaw - desired_yaw < -180)
        {
            raw_yaw += 360;
        }

        int16_t tail_pwm = control_generic_poll(raw_yaw, desired_yaw, &yaw_controller_config);
        pwm_set_tail_rotor_dutycycle(tail_pwm);

        // Update the display
        display_send_message(TailDuty, tail_pwm, 0);
        int16_t display_yaw = raw_yaw < 0 ? raw_yaw + 360 : raw_yaw; // make yaw display always positive
        display_send_message(Yaw, display_yaw, desired_yaw);

        vTaskDelayUntil(&wake_time, CONTROL_TIMESTEP);
    }
}
