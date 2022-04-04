#include <stdint.h>
#include <stdbool.h>

#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <driverlib/sysctl.h>
#include <driverlib/qei.h>
#include <driverlib/adc.h>
#include <driverlib/comp.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <driverlib/pin_map.h>
#include <inc/hw_ints.h>
#include <driverlib/interrupt.h>
#include <utils/ustdlib.h>
#include "inc/hw_gpio.h"

#include <FreeRTOS.h>
#include <task.h>


#define LED_PERIPH  SYSCTL_PERIPH_GPIOF
#define LED_PORT    GPIO_PORTF_BASE
#define LED_R       GPIO_PIN_1
#define LED_G       GPIO_PIN_3
#define LED_B       GPIO_PIN_2
#define LED_RGB     (LED_R | LED_G | LED_B)


void uart_init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinConfigure (GPIO_PA0_U0RX);
    GPIOPinConfigure (GPIO_PA1_U0TX);

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600,
    		UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
			UART_CONFIG_PAR_NONE);
    UARTFIFOEnable(UART0_BASE);
    UARTEnable(UART0_BASE);
}


void uart_puts(char* str)
{
    while (*str) {
        UARTCharPut(UART0_BASE, *str++);
    }
}

#define ENCODER_COUNTS 448 // TODO tune or measure this
static volatile int16_t quad_postition = 0;

#define PHASE_A GPIO_PIN_1
#define PHASE_B GPIO_PIN_0

void quad_interrupt(void)
{
    static bool a_dash = false;
    static bool b_dash = false;
    const uint32_t gpio = GPIOPinRead(GPIO_PORTB_BASE, PHASE_A | PHASE_B);
    const bool a = gpio & PHASE_A;
    const bool b = gpio & PHASE_B;
    if (b_dash ^ a)
    {
        if (++quad_postition >= ENCODER_COUNTS) {
            quad_postition = quad_postition - ENCODER_COUNTS;
        }
    }
    else if (a_dash ^ b)
    {
        if (--quad_postition < 0)
        {
            quad_postition = quad_postition + ENCODER_COUNTS;
        };
    }
    a_dash = a;
    b_dash = b;

    GPIOIntClear(GPIO_PORTB_BASE, PHASE_A | PHASE_B);
}

void quad_init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    uint32_t pins = PHASE_A | PHASE_B;

    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, pins);

    // // Enable pullups
    // GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    // GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    GPIOIntDisable(GPIO_PORTB_BASE, pins);
    GPIOIntRegister(GPIO_PORTB_BASE, quad_interrupt);

    GPIOIntTypeSet(GPIO_PORTB_BASE, pins, GPIO_BOTH_EDGES);
    GPIOIntEnable(GPIO_PORTB_BASE, pins);
}

void adc_init(void)
{
    //
    // The ADC0 peripheral must be enabled for use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    //
    // For this example ADC0 is used with AIN0 on port E7.
    // The actual port and pins used may be different on your part, consult
    // the data sheet for more information.  GPIO port E needs to be enabled
    // so these pins can be used.
    // TODO: change this to whichever GPIO port you are using.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    //
    // Select the analog ADC function for these pins.
    // Consult the data sheet to see which functions are allocated per pin.
    // TODO: change this to select the port/pin you are using.
    //
    // https://microcontrollerslab.com/adc-tm4c123g-tiva-c-launchpad-measure-analog-voltage-signal/
    // GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3); // AIN0
    // GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2); // AIN1
    // GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1); // AIN2
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_4); // AIN9

    //
    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.  Each ADC module has 4 programmable sequences, sequence 0
    // to sequence 3.  This example is arbitrarily using sequence 3.
    //
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    //
    // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
    // single-ended mode (default) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.  For more
    // information on the ADC sequences and steps, reference the datasheet.
    //
    // ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END); // use AIN0
    // ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH1 | ADC_CTL_IE | ADC_CTL_END); // use AIN1
    // ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH2 | ADC_CTL_IE | ADC_CTL_END); // use AIN2
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH9 | ADC_CTL_IE | ADC_CTL_END); // use AIN9

    //
    // Since sample sequence 3 is now configured, it must be enabled.
    //
    ADCSequenceEnable(ADC0_BASE, 3);

    //
    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    //
    ADCIntClear(ADC0_BASE, 3);
}

int adc_get_height(void)
{
    //
    // This array is used for storing the data read from the ADC FIFO. It
    // must be as large as the FIFO for the sequencer in use.  This example
    // uses sequence 3 which has a FIFO depth of 1.  If another sequence
    // was used with a deeper FIFO, then the array size must be changed.
    //
    uint32_t pui32ADC0Value[1];

    //
    // Trigger the ADC conversion.
    //
    ADCProcessorTrigger(ADC0_BASE, 3);

    //
    // Wait for conversion to be completed.
    //
    while(!ADCIntStatus(ADC0_BASE, 3, false))       // ---> program stuck HERE
    {
    }

    //
    // Clear the ADC interrupt flag.
    //
    ADCIntClear(ADC0_BASE, 3);

    //
    // Read ADC Value.
    //
    ADCSequenceDataGet(ADC0_BASE, 3, pui32ADC0Value);


    // convert to height percentage
    uint32_t height = pui32ADC0Value[0];
    height = height > 2300 ? 2300 : height;
    height = height < 1800 ? 1800 : height;
    return (2300 - height) / 5;
}

void blink(void* args) {
    (void)args; // unused

    TickType_t wake_time = xTaskGetTickCount();

    while (true) {
        GPIOPinWrite(GPIO_PORTF_BASE, LED_RGB, GPIOPinRead(GPIO_PORTF_BASE, LED_G) ^ LED_G);
        vTaskDelayUntil( &wake_time, 200 );
    }
}


void greet(void* args)
{
    (void)args; // unused

    TickType_t wake_time = xTaskGetTickCount();
    uint8_t count = 0;

    while (true) {
        int height = adc_get_height();
        uint16_t deg = (double)quad_postition * 360000 / (ENCODER_COUNTS * 1000);
        // uint16_t deg = 445.0;

        char greeting[100];

        usnprintf(greeting, sizeof(greeting), "Hello %i,  quad=%i  deg=%i  height=%i%%\r\n", count++, quad_postition, deg, height);
        uart_puts(greeting);

        vTaskDelayUntil( &wake_time, 500 );
    }
}

void read(void* args)
{
    (void)args; // unused

    TickType_t wake_time = xTaskGetTickCount();

    while (true) {
        // GPIOPinWrite(GPIO_PORTF_BASE, LED_B, !(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_6));
        bool switch_state = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_6) & GPIO_PIN_6;
        GPIOPinWrite(GPIO_PORTF_BASE, LED_B, switch_state ? LED_B : 0);
        vTaskDelayUntil( &wake_time, 200 );
    }
}

int main(void)
{
    SysCtlClockSet (SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(LED_PERIPH);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(LED_PORT, LED_RGB);
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_6);

    uart_init();
    quad_init();
    adc_init();

    // xTaskCreate(&blink, "blink", 256, NULL, 0, NULL);
    xTaskCreate(&greet, "greet", 256, NULL, 0, NULL);
    xTaskCreate(&read, "read", 256, NULL, 0, NULL);

    vTaskStartScheduler();

    return 0;
}

void vAssertCalled( const char * pcFile, unsigned long ulLine )
{
    (void)pcFile; // unused
    (void)ulLine; // unused
    while (true) ;
}