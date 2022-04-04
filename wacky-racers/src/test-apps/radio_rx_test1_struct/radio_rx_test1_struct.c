/* File:   radio_rx_test1.c
   Author: M. P. Hayes, UCECE
   Date:   24 Feb 2018
   Descr:
*/
#include "nrf24.h"
#include "usb_serial.h"
#include "pio.h"
#include "delay.h"
#include "pwm.h"


nrf24_t *nrf;

typedef struct {
    uint8_t motor_left;
    uint8_t motor_right;
} payload_h2r_t;


#define PWM_FREQ_HZ 20e2 

//Set up the PWM for A Motor
static const pwm_cfg_t left_pwm_cfg =
{
    .pio = U2_MOTOR_LEFT_PWM_PIO,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 100),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_LOW,
    .stop_state = PIO_OUTPUT_LOW
};

//Set up the PWM for B Motor
static const pwm_cfg_t right_pwm_cfg =
{
    .pio = U2_MOTOR_RIGHT_PWM_PIO,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 100),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_LOW,
    .stop_state = PIO_OUTPUT_LOW
};


void set_motor_pwm(pwm_t left_pwm, pwm_t right_pwm, payload_h2r_t payload)
{
    const int upper = 135;
    const int lower = 125;

    int left;
    int right;
    //Set the left motor first
    //Condition for forward movement

    if ((payload.motor_left < upper) && (payload.motor_left > lower))
    {
        pio_config_set (U2_MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_LOW);
        pwm_duty_ppt_set (right_pwm, 0);
    }

    else if (payload.motor_left >= upper){
        pio_config_set (U2_MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_HIGH);
        left = (1000 - (payload.motor_left - 126)*10);
        pwm_duty_ppt_set (right_pwm, left);
    }

    //Condition for backwards movement
    else if (payload.motor_left < lower){
        pio_config_set (U2_MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_LOW);
        left = 1000 - ((payload.motor_left- 30)*10);
        pwm_duty_ppt_set (right_pwm, left);
    }


    //Next , set the right motor first
    //Condition for forward movement
    if ((payload.motor_right < upper) && (payload.motor_right > lower))
    {
        pio_config_set (U2_MOTOR_RIGHT_MODE_PIO, PIO_OUTPUT_LOW);
        pwm_duty_ppt_set (left_pwm, 0);
    }


    else if (payload.motor_right >= upper){
        pio_config_set (U2_MOTOR_RIGHT_MODE_PIO, PIO_OUTPUT_HIGH);
        right = 1000 - ((payload.motor_right - 126)*10);
        pwm_duty_ppt_set (left_pwm, right);
    }

    //Condition for backwards movement
    else if (payload.motor_right < lower){
        pio_config_set (U2_MOTOR_RIGHT_MODE_PIO, PIO_OUTPUT_LOW);
        right = 1000 - ((payload.motor_right - 30)*10);
        pwm_duty_ppt_set (left_pwm, right);
    }

    printf("%d, %d\n", left, right);
    printf("Raw: %d, %d\n",payload.motor_left, payload.motor_right);
    fflush(stdout);
}



static void panic(void)
{
    while (1) {
        pio_output_toggle(LED1_PIO);
        pio_output_toggle(LED2_PIO);
        delay_ms(400);
    }
}

//Receive functions stollen from comms.c

static payload_h2r_t decode_payload_h2r(uint8_t buffer[])
{
    return (payload_h2r_t){
        .motor_left = buffer[0],
        .motor_right = buffer[1]
    };
}

bool comms_receive_h2r_payload(payload_h2r_t *payload_h2r_p)
{
    char buffer[32] = {0};

    if (nrf24_read(nrf, buffer, sizeof(buffer)))
    {
        *payload_h2r_p = decode_payload_h2r(buffer);
        return true;
    }
    return false;
}



int main (void)
{
    spi_cfg_t nrf_spi = {
        .channel = 0,
        .clock_speed_kHz = 1000,
        .cs = RADIO_CS_PIO,
        .mode = SPI_MODE_0,
        .cs_mode = SPI_CS_MODE_FRAME,
        .bits = 8,
    };

    spi_t spi;
    usb_cdc_t usb_cdc;

     //Setup and start the pwm A 
    pwm_t left_pwm;
    left_pwm = pwm_init (&left_pwm_cfg);
    pwm_channels_start (pwm_channel_mask (left_pwm));
    
    //Setup and start the pwm B
    pwm_t right_pwm;
    right_pwm = pwm_init (&right_pwm_cfg);
    pwm_channels_start (pwm_channel_mask (right_pwm));

    //Configure the sleep pin to be high to enable the driver
    pio_config_set (U2_MOTOR_SLEEP_PIO, PIO_OUTPUT_HIGH);

    //Set mode to be high, idk which way this will make the motor spin
    pio_config_set (U2_MOTOR_LEFT_MODE_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (U2_MOTOR_RIGHT_MODE_PIO, PIO_OUTPUT_HIGH);


    /* Configure LED PIO as output.  */
    pio_config_set(LED1_PIO, PIO_OUTPUT_LOW);
    pio_config_set(LED2_PIO, PIO_OUTPUT_LOW);

    // Create non-blocking tty device for USB CDC connection.
    usb_serial_init(NULL, "/dev/usb_tty");

    freopen("/dev/usb_tty", "a", stdout);
    freopen("/dev/usb_tty", "r", stdin);

    spi = spi_init(&nrf_spi);
    nrf = nrf24_create(spi, RADIO_CE_PIO, RADIO_IRQ_PIO);
    if (!nrf)
        panic();

    // initialize the NRF24 radio with its unique 5 byte address
    if (!nrf24_begin(nrf, 15, 0x0101010203, 32))
        panic();
    if (!nrf24_listen(nrf))
        panic();


    //Motors 
    while (1)
    {
        payload_h2r_t payload_h2r;

        if (comms_receive_h2r_payload(&payload_h2r)) {
            //printf("%d, %d\n", payload_h2r.motor_left, payload_h2r.motor_right);
            set_motor_pwm(left_pwm, right_pwm, payload_h2r);
            //fflush(stdout);
            pio_output_toggle(LED2_PIO);
            pio_output_toggle(LED1_PIO);
        }
    }
}
