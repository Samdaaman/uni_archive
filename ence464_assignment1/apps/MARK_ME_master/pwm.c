#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"

#include <FreeRTOS.h>
#include <task.h>

#include "pwm.h"

// PWM configuration
#define PWM_DIVIDER_CODE        SYSCTL_PWMDIV_4
#define PWM_DIVIDER             4
#define PWM_CONST_FREQ          200
#define DUTY_CYCLE_START        0

//Main Rotor PWM Config: PC5 
#define PWM_MAIN_BASE           PWM0_BASE
#define PWM_MAIN_GEN            PWM_GEN_3
#define PWM_MAIN_OUTNUM         PWM_OUT_7
#define PWM_MAIN_OUTBIT         PWM_OUT_7_BIT
#define PWM_MAIN_PERIPH_PWM     SYSCTL_PERIPH_PWM0
#define PWM_MAIN_PERIPH_GPIO    SYSCTL_PERIPH_GPIOC
#define PWM_MAIN_GPIO_BASE      GPIO_PORTC_BASE
#define PWM_MAIN_GPIO_CONFIG    GPIO_PC5_M0PWM7
#define PWM_MAIN_GPIO_PIN       GPIO_PIN_5


// PWM Hardware Details M1PWM5
// --Tail Rotor PWM: PF1 
#define PWM_TAIL_BASE           PWM1_BASE
#define PWM_TAIL_GEN            PWM_GEN_2
#define PWM_TAIL_OUTNUM         PWM_OUT_5
#define PWM_TAIL_OUTBIT         PWM_OUT_5_BIT
#define PWM_TAIL_PERIPH_PWM     SYSCTL_PERIPH_PWM1
#define PWM_TAIL_PERIPH_GPIO    SYSCTL_PERIPH_GPIOF
#define PWM_TAIL_GPIO_BASE      GPIO_PORTF_BASE
#define PWM_TAIL_GPIO_CONFIG    GPIO_PF1_M1PWM5
#define PWM_TAIL_GPIO_PIN       GPIO_PIN_1

static uint8_t last_main_duty = 0;
static uint8_t last_tail_duty = 0;


/**
 * @brief Initialization function that sets up the PWM
 * 
 */

void pwm_init(void)
{
    // Init Main Rotor 
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_GPIO);

    GPIOPinConfigure(PWM_MAIN_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_MAIN_GPIO_BASE, PWM_MAIN_GPIO_PIN);

    PWMGenConfigure(PWM_MAIN_BASE, PWM_MAIN_GEN, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    // Set the initial PWM parameters
    pwm_set_main_rotor_dutycycle (DUTY_CYCLE_START);


    //Init Tail Rotor
    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_GPIO);

    GPIOPinConfigure(PWM_TAIL_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_TAIL_GPIO_BASE, PWM_TAIL_GPIO_PIN);

    PWMGenConfigure(PWM_TAIL_BASE, PWM_TAIL_GEN, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    // Set the initial PWM parameters
    pwm_set_tail_rotor_dutycycle (DUTY_CYCLE_START);


}


/**
 * @brief Changes the PWM dutycycle of the main rotor. Can only be called after the PWM peripherals have been set up
 * 
 * @param speed The desired PWM value as precentage of the maximum duty cycle (100)
 */
void pwm_set_main_rotor_dutycycle(uint8_t speed)
{
    // Calculate the PWM period corresponding to the freq.
    uint32_t period = SysCtlClockGet() / PWM_DIVIDER / PWM_CONST_FREQ;
    PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, period);
    PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM, period * speed / 100);
    last_main_duty = speed;
}


/**
 * @brief Changes the PWM dutycycle of the tail rotor. Can only be called after the PWM peripherals have been set up
 * 
 * @param speed The desired PWM value as precentage of the maximum duty cycle (100)
 */
void pwm_set_tail_rotor_dutycycle(uint8_t speed)
{
    // Calculate the PWM period corresponding to the freq.
    uint32_t period = SysCtlClockGet() / PWM_DIVIDER / PWM_CONST_FREQ;

    PWMGenPeriodSet(PWM_TAIL_BASE, PWM_TAIL_GEN, period);
    PWMPulseWidthSet(PWM_TAIL_BASE, PWM_TAIL_OUTNUM, period * speed / 100);
    last_tail_duty = speed;
}


/**
 * @brief Function that's called after pwm_init to start the main rotor PWM output
 * This function is separate to pwm_init to allow the PWM peripherals to be setup at the 
 * same time as all other step up routines. But enables the PWM output to only be enabled just before 
 * it is needed. 
 * This helps to reduce the chance of eratic outputs incase the PWM is enabled before the system is fully initalized 
 */

uint8_t enable_pwm_main_output(){

    //Enable the PWM timer 
    PWMGenEnable(PWM_MAIN_BASE, PWM_MAIN_GEN);

    // Switch the PWM states to on
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, true);

    // Set the PWM Duty Cycle of both outputs to zero
    pwm_set_main_rotor_dutycycle(DUTY_CYCLE_START);

    return DUTY_CYCLE_START;

}


/**
 * @brief Function that's called after pwm_init to start the tail rotor PWM output
 * This function is separate to pwm_init to allow the PWM peripherals to be setup at the 
 * same time as all other step up routines. But enables the PWM output to only be enabled just before 
 * it is needed. 
 * This helps to reduce the chance of eratic outputs incase the PWM is enabled before the system is fully initalized 
 */

uint8_t enable_pwm_tail_output(){

    //Enable the PWM timer 
    PWMGenEnable(PWM_TAIL_BASE, PWM_TAIL_GEN);

    // Switch the PWM states to on
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, true);

    // Set the PWM Duty Cycle of both outputs to zero
    pwm_set_tail_rotor_dutycycle(DUTY_CYCLE_START);

    return DUTY_CYCLE_START;

}