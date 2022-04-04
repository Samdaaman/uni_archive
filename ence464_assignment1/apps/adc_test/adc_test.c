#include <stdint.h>
#include <stdbool.h>

#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <driverlib/sysctl.h>
#include <driverlib/adc.h>
#include <driverlib/comp.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <driverlib/pin_map.h>
#include <inc/hw_ints.h>
#include <driverlib/interrupt.h>
#include <utils/ustdlib.h>

#include <FreeRTOS.h>
#include <task.h>

#define LED_PERIPH  SYSCTL_PERIPH_GPIOF
#define LED_PORT    GPIO_PORTF_BASE
#define LED_R       GPIO_PIN_1
#define LED_G       GPIO_PIN_3
#define LED_B       GPIO_PIN_2
#define LED_RGB     (LED_R | LED_G | LED_B)

#define  ADC0_SEQUENCER3   3

void uart_init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    // SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    // SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinConfigure (GPIO_PA0_U0RX);
    GPIOPinConfigure (GPIO_PA1_U0TX);

    // GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);
    // ComparatorConfigure(COMP_BASE, 1, (COMP_TRIG_FALL | COMP_ASRCP_PIN | COMP_OUTPUT_NORMAL));
    // ADCSequenceDisable(ADC0_BASE, ADC0_SEQUENCER3);
    // ADCReferenceSet(ADC0_BASE,ADC_REF_INT);
    // ADCSequenceConfigure(ADC0_BASE, ADC0_SEQUENCER3, ADC_TRIGGER_COMP1, 0 );
    // ADCSequenceStepConfigure(ADC0_BASE, ADC0_SEQUENCER3, 0, ADC_CTL_CH1 | ADC_CTL_IE | ADC_CTL_END );
    // ADCSequenceEnable(ADC0_BASE, ADC0_SEQUENCER3);
    // ADCIntClear(ADC0_BASE, ADC0_SEQUENCER3);
    // ADCIntEnable(ADC0_BASE, ADC0_SEQUENCER3);

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600,
    		UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
			UART_CONFIG_PAR_NONE);
    UARTFIFOEnable(UART0_BASE);
    UARTEnable(UART0_BASE);
}


void uart_puts(char* str) {
    while (*str) {
        UARTCharPut(UART0_BASE, *str++);
    }
}

static volatile unsigned int adcValue[1] = {0};

// isr()
// {
//     ADCIntClear(ADC0_BASE, ADC0_SEQUENCER3);
//     ADCSequenceDataGet(ADC0_BASE, ADC0_SEQUENCER3, adcValue);
// }


void blink(void* args) {
    (void)args; // unused

    TickType_t wake_time = xTaskGetTickCount();

    while (true) {
        GPIOPinWrite(GPIO_PORTF_BASE, LED_RGB, GPIOPinRead(GPIO_PORTF_BASE, LED_G) ^ LED_G);
        vTaskDelayUntil( &wake_time, 200 );
    }
}


void greet(void* args) {
    (void)args; // unused

    TickType_t wake_time = xTaskGetTickCount();
    uint8_t count = 0;

    while (true) {
        char greeting[100];

        usnprintf(greeting, sizeof(greeting), "Hello %i %i!\r\n", count++, adcValue[0]);
        uart_puts(greeting);

        vTaskDelayUntil( &wake_time, 500 );
    }
}


int main(void) {
    SysCtlClockSet (SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(LED_PERIPH);
    GPIOPinTypeGPIOOutput(LED_PORT, LED_RGB);

    uart_init();

    xTaskCreate(&blink, "blink", 256, NULL, 0, NULL);
    xTaskCreate(&greet, "greet", 256, NULL, 0, NULL);

    vTaskStartScheduler();

    return 0;
}

void vAssertCalled( const char * pcFile, unsigned long ulLine ) {
    (void)pcFile; // unused
    (void)ulLine; // unused
    while (true) ;
}