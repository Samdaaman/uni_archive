#include "low_voltage.h"
#include "adc.h"
#include "pio.h"

#define ADC_CLOCK_FREQ 24000000
adc_t adc_voltage;

int voltage_read_smooth = 0;

//Set up ADC
static const adc_cfg_t adc_cfg =
{
    .bits = 12,
    .channel = BATT_V_PIO,
    .trigger = ADC_TRIGGER_SW,
    .clock_speed_kHz = ADC_CLOCK_FREQ / 1000
};

//ADC Voltage Functions
void adc_voltage_setup(void){
    adc_voltage = adc_init (&adc_cfg);
}

bool is_voltage_low (void){
    uint16_t data[1];
    adc_read (adc_voltage, data, sizeof (data));
    if (data[0] < _5V_VALUE){
        voltage_read_smooth++;
        if (voltage_read_smooth >= 255){
            return true;
        }
    }else{
        voltage_read_smooth = 0;
        return false;
    core_printf("%d", voltage_read_smooth);
    }
}


    // if (data[0] < 2585){
    //     pwm_stop(left_pwm);
    //     pwm_stop(right_pwm);
    //     pio_output_set(LED1_PIO, 0);
    //     pio_output_set(LED2_PIO, 0);
    // }else{
    //     pwm_start(left_pwm);
    //     pwm_start(right_pwm);
    // }