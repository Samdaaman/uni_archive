#include "joystick.h"
#include "pio.h"
#include "adc.h"
#include "core.h"



#define ADC_CLOCK_FREQ 24000000


static const adc_cfg_t adc_x_cfg =
{
    .bits = 12,
    .channel = ADC_CHANNEL_5,
    .trigger = ADC_TRIGGER_SW,
    .clock_speed_kHz = ADC_CLOCK_FREQ / 1000
};

static const adc_cfg_t adc_y_cfg =
{
    .bits = 12,
    .channel = ADC_CHANNEL_6,
    .trigger = ADC_TRIGGER_SW,
    .clock_speed_kHz = ADC_CLOCK_FREQ / 1000
};




static adc_t adc_x;
static adc_t adc_y;


void joystick_initialise(void)
{
    adc_x = adc_init(&adc_x_cfg);
    adc_y = adc_init(&adc_y_cfg);
    pio_config_set(JOYSTICK_SW, PIO_PULLUP);
}

void joystick_shutdown(void)
{
    adc_shutdown(&adc_x_cfg);
    adc_shutdown(&adc_y_cfg);
}

static uint8_t scale_value(uint16_t raw)
{
    return (limit_int(400, 3600, raw) - 400) * 50 / 1600;
}

payload_h2r_t joystick_read(void)
{
    uint16_t x_raw;
    uint16_t y_raw;
    adc_read (adc_x, &x_raw, sizeof (x_raw));
    adc_read (adc_y, &y_raw, sizeof (y_raw));
    int x = scale_value(x_raw); // 0-100
    int y = scale_value(y_raw); // 0-100

    return (payload_h2r_t){
        .motor_left = limit_int(0, 100, 50 + (y - 50) + (x - 50)),
        .motor_right = limit_int(0, 100, 50 + (y - 50) - (x - 50)),
        .should_deploy_string = false,
    };
}

bool joystick_is_pressed(void)
{
    return !pio_input_get(JOYSTICK_SW);
}

