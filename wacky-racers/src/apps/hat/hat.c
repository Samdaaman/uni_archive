#include "target.h"
#include "core.h"
#include "delay.h"
#include "nrf24.h"
#include "pio.h"
#include "pacer.h"
#include "comms.h"
#include "adc.h"
#include "joystick.h"
#include "led_tape.h"
#include "buzzer.h"
#include "imu.h"
#include "button.h"

#define ADC_CLOCK_FREQ 24000000

enum {LOOP_POLL_RATE = 2000};
enum {LED_FLASH_RATE = 100};


static adc_t low_power_adc;

static payload_h2r_t payload_h2r = {
    .motor_left = 128,
    .motor_right = 128
};

static adc_t adc_x;
static adc_t adc_y;

//Set up ADC
static const adc_cfg_t adc_cfg =
{
    .bits = 12,
    .channel = ADC_CHANNEL_9,
    .trigger = ADC_TRIGGER_SW,
    .clock_speed_kHz = ADC_CLOCK_FREQ / 1000
};

static bool voltage_low(void)
{
    uint16_t data[1];
    adc_read(low_power_adc, data, sizeof (data));
    if (data[0] < _5V_VALUE){
        return true;
    }else{
        return false;
    }
}

int main(void)
{
    core_initialise();
    core_initialise_usb_serial();

    delay_ms(100);

    comms_initialise();
    imu_initialise();
    joystick_initialise();
    led_tape_initialise();
    buzzer_initialise();
    low_power_adc = adc_init (&adc_cfg);
    pacer_init(30);
    unsigned long int i;
    uint8_t flash_ticks = 0;
    pio_config_set (LED2_PIO, PIO_OUTPUT_HIGH);
    while (1)
    {
        i++;
        pacer_init (LOOP_POLL_RATE);

        if (core_sleep_poll())
        {
            comms_shutdown();
            led_tape_stop();
            buzzer_die();
            core_go_to_sleep();
            core_panic(); // this will never actually happen but good in case the sleep doesn't actually work
        }

        bool is_voltage_low = voltage_low();
        bool useIMU = core_read_dip_value() >= 8;

        if (useIMU){
            payload_h2r = imu_read();
        }else if (!useIMU){
            payload_h2r = joystick_read();
        }
        core_printf("left=%d :: right=%d\n", payload_h2r.motor_left, payload_h2r.motor_right);
        
        static int joystick_debouncing = 0;
        if (joystick_is_pressed())
        {
            if (joystick_debouncing < 10)
            {
                joystick_debouncing += 1;
            }
            else
            {
                payload_h2r.should_deploy_string = true;
                pio_output_toggle(BUZZER_PIO);
            }
        }
        else
        {
            joystick_debouncing = 0;
        }

        if (comms_send_h2r_payload(payload_h2r))
        {
            //pio_output_set(LED2_PIO, 1);
        }
        else
        {
            if (!is_voltage_low)
            {
                pio_output_set(LED2_PIO, 0);
            }
        }

        buzzer_button_poll();
        // if (pio_input_get(BUTTON_JOYSTICK)){
        //     if (useIMU){
        //         useIMU = false;
        //         pio_output_toggle(LED1_PIO);
        //     }else{
        //         useIMU = true;
        //     }
        // }

        if (is_voltage_low)
        {
            led_tape_stop();
            pio_output_toggle(LED1_PIO);
        }
        else if ((i%2) == 0)
        {
            led_tape_poll();
        }

        pacer_wait ();
        flash_ticks++;

        if (flash_ticks >= LOOP_POLL_RATE / (LED_FLASH_RATE * 2))
        {
            flash_ticks = 0;

            /* Toggle LED.  */
            pio_output_toggle (LED2_PIO);
        }

    }
}



