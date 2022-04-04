/**
 * @file inputs.c
 * @author Group 16
 * @brief Module that calibrates the height and yaw sensors for the helirig project 
 */


#include <stdbool.h>
#include <stdint.h>

#include <driverlib/adc.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <driverlib/sysctl.h>
#include <inc/hw_gpio.h>
#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>

#include <FreeRTOS.h>
#include <semphr.h>

#include "inputs.h"

#include "driverlib/timer.h"

#define ENCODER_COUNTS 448
#define PHASE_A GPIO_PIN_1
#define PHASE_B GPIO_PIN_0
#define ADC_FREQUENCY 500 // must be at least 5 times higher than the PID controllers frequency
#define ADC_SEQUENCE_NUM 3 // use ADC sequence 3 which has a buffer size of 1 (ADC_FIFO_SIZE)
#define ADC_FIFO_SIZE 1

static void timer_isr(void);

volatile uint16_t adc_raw_value;
static volatile int16_t quad_position = ENCODER_COUNTS / 4; // start at 90deg
bool inputs_quad_reference_reset = false;
SemaphoreHandle_t inputs_quad_reference_reset_semaphore = NULL;


/**
 * @brief Copy the rotary encoder value from the ISR variable to a safe to read vairable
 * 
 * @return int16_t The rotational position of the heli in degrees.
 */
int16_t inputs_quad_get(void)
{
    int16_t quad_position_safe = quad_position; // read of a 16bit value is atomic and happens in one instruction (interrupts can't occur in between)
    return quad_position_safe * 360 / ENCODER_COUNTS;
}


/**
 * @brief ISR that triggers on a the incremental encoders GPIO rising edge 
 * Determines if the quad position should be incremented or decremented
 */
void inputs_quad_interrupt(void)
{
    static bool a_dash = false;
    static bool b_dash = false;
    
    const uint32_t gpio = GPIOPinRead(GPIO_PORTB_BASE, PHASE_A | PHASE_B);
    GPIOIntClear(GPIO_PORTB_BASE, PHASE_A | PHASE_B);
    
    const bool a = gpio & PHASE_A;
    const bool b = gpio & PHASE_B;
    
    if (b_dash ^ a)
    {
        quad_position++;
        if (quad_position >= ENCODER_COUNTS) {
            quad_position -= ENCODER_COUNTS;
        }
    }
    else if (a_dash ^ b)
    {
        quad_position--;
        if (quad_position < 0)
        {
            quad_position += ENCODER_COUNTS;
        };
    }
    
    a_dash = a;
    b_dash = b;
}


/**
 * @brief Initialization function to setup the GPIO pins and interupt for the incremental encoder counts 
 */
void inputs_quad_init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, PHASE_A | PHASE_B);
    GPIOIntDisable(GPIO_PORTB_BASE, PHASE_A | PHASE_B);
    GPIOIntRegister(GPIO_PORTB_BASE, inputs_quad_interrupt);

    GPIOIntTypeSet(GPIO_PORTB_BASE, PHASE_A | PHASE_B, GPIO_BOTH_EDGES);
    GPIOIntEnable(GPIO_PORTB_BASE, PHASE_A | PHASE_B);
}


/**
 * @brief ISR that triggers on a the index encoders GPIO rising edge 
 * When triggered, will reset the incremental encoder. Also sets a semaphore 
 * that allows the calibration roution to continue 
 */
void reset_reference_interrupt(void)
{
    quad_position = 0; // this is a 16-bit (atomic) instruction
    GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_4);
    GPIOIntDisable(GPIO_PORTC_BASE, GPIO_PIN_4);
    static BaseType_t higher_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(inputs_quad_reference_reset_semaphore, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}



/**
 * @brief Initialization function to setup the GPIO pins and interupt for the index encoder count
 */
void inputs_quad_enable_reference_reset(void)
{
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4);
    GPIOIntDisable(GPIO_PORTC_BASE, GPIO_PIN_4);
    GPIOIntRegister(GPIO_PORTC_BASE, reset_reference_interrupt);
    GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
    IntPrioritySet(INT_GPIOC, configMAX_API_CALL_INTERRUPT_PRIORITY + 1);
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_PIN_4);
}


/**
 * @brief Callback ISR for when the the ADC has finished processing a value 
 */
void adc_done_isr(void){
    ADCIntClear(ADC0_BASE, ADC_SEQUENCE_NUM);
    uint32_t temp_adc_value_32_bit[1];
    ADCSequenceDataGet(ADC0_BASE, ADC_SEQUENCE_NUM, temp_adc_value_32_bit);
    uint16_t temp_adc_value_16_bit = temp_adc_value_32_bit[0];
    
    // This operation is atmoic as the variables being read/written to are 16 bit,
    // and thus the copy can be done within a single cpu cycle
    // To make this work, the vairables have to be 16 bit.
    adc_raw_value = temp_adc_value_16_bit; 
}

/**
 * @brief Initializes the ADC peripherals and sets up the interrupt callback method 
 */
void inputs_adc_init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_4); // AIN9
    ADCSequenceConfigure(ADC0_BASE, ADC_SEQUENCE_NUM, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, ADC_SEQUENCE_NUM, 0, ADC_CTL_CH9 | ADC_CTL_IE | ADC_CTL_END); // use AIN9
    
    ADCIntRegister(ADC0_BASE, ADC_SEQUENCE_NUM, adc_done_isr);
    ADCIntEnable(ADC0_BASE, ADC_SEQUENCE_NUM);
    ADCSequenceEnable(ADC0_BASE, ADC_SEQUENCE_NUM);
    ADCIntClear(ADC0_BASE, ADC_SEQUENCE_NUM);
}


/**
 * @brief ISR for when the timer interupt triggers
 */
static void timer_isr(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ADCProcessorTrigger(ADC0_BASE, ADC_SEQUENCE_NUM);
}


/**
 * @brief Initializes the Timer Peripheral
 * Sets up the timer_isr to trigger when the the Timer overflows 
 */
void timer_init()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerIntRegister(TIMER0_BASE, TIMER_A, timer_isr);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / ADC_FREQUENCY);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);
}


/**
 * @brief Function called from control module to get the current lastest adc value 
 * 
 * @return uint16_t the 12 bit raw adc value stored in the adc_raw_value variable
 */
uint16_t inputs_adc_get_raw(void)
{
    return adc_raw_value;
}
