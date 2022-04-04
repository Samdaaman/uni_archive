#include <stdint.h>
#include "control.h"

typedef struct {
    const uint32_t p_gain;
    const uint32_t i_gain;
    const uint32_t timestep; // in milliseconds
    const uint8_t min_value;
    const uint8_t max_value;
    double integral_carry_over;
} pi_controller_config_t;

static uint8_t control_generic_poll(uint8_t actual_value, uint8_t desired_value, pi_controller_config_t *controller_config);

static pi_controller_config_t height_controller_config = {
    .p_gain = 5, // TODO
    .i_gain = 3, // TODO
    .timestep = CONTROL_HEIGHT_TIMESTEP,
    .min_value = 0, // TODO think this has to be 2% for PWM
    .max_value = 100, // TODO think this has to be 98% for PWM
    .integral_carry_over = 0.0
};

static pi_controller_config_t yaw_controller_config = {
    .p_gain = 0, // TODO
    .i_gain = 0, // TODO
    .timestep = CONTROL_YAW_TIMESTEP,
    .min_value = 0, // TODO think this has to be 2% for PWM
    .max_value = 100, // TODO think this has to be 98% for PWM
    .integral_carry_over = 0.0
};

uint8_t control_yaw_poll(uint8_t actual_yaw, uint8_t desired_yaw)
{
    return control_generic_poll(actual_yaw, desired_yaw, &yaw_controller_config);
}

uint8_t control_height_poll(uint8_t actual_height, uint8_t desired_height)
{
    return control_generic_poll(actual_height, desired_height, &height_controller_config);
}

static uint8_t control_generic_poll(uint8_t actual_value, uint8_t desired_value, pi_controller_config_t *controller_config)
{
    int32_t error = desired_value - actual_value;
    controller_config->integral_carry_over = controller_config->integral_carry_over + error * (int32_t)controller_config->timestep / 1000.0;
    int32_t result = error * controller_config->p_gain + controller_config->integral_carry_over * controller_config->i_gain;
    
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
