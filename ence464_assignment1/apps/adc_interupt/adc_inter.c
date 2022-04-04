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

//Timer Include 
#include "driverlib/timer.h"
#include <FreeRTOS.h>
#include <task.h>

//Temp Global
uint16_t HeightADCValueGet[1];
volatile uint16_t HeightADCValueRead;

void uart_init(void) {
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

void uart_puts(char* str) {
    while (*str) {
        UARTCharPut(UART0_BASE, *str++);
    }
}

void greet(void* args) {
    (void)args; // unused

    TickType_t wake_time = xTaskGetTickCount();
    
    while (true) {
        //int qeiPosition = QEIPositionGet(QEI0_BASE);

        char greeting[100];

        usnprintf(greeting, sizeof(greeting), " Raw ADC Value=%i\r\n", HeightADCValueRead);
        uart_puts(greeting);

        vTaskDelayUntil( &wake_time, 500 );
    }
}

void adc_done_isr(void){
    ADCIntClear(ADC0_BASE, 3);
    ADCSequenceDataGet(ADC0_BASE, 3, HeightADCValueGet);

    // This operation is atmoic as the variables being read/written to are 16 bit,
    // and thus the copy can be done within a single cpu cycle
    // To make this work, the vairables have to be 16 bit. This is fine, as the ADC being used 
    // is only 12 bit 
    HeightADCValueRead = HeightADCValueGet[0]; 
}

void inputs_adc_init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3); // AIN0
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2); // AIN1
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH1 | ADC_CTL_IE | ADC_CTL_END); // use AIN1
    //Code for enabling an interput callback 
    ADCIntRegister(ADC0_BASE, 3, adc_done_isr);
    ADCIntEnable(ADC0_BASE, 3);
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntClear(ADC0_BASE, 3);
}

void timer_isr(void){
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ADCProcessorTrigger(ADC0_BASE, 3);
}

void timer_init(){
   /**
    * @brief Setup functions for the interupt generating timer that triggers 
    * the ADC conversion process
    * 
    */

    //Set Timer Frequency
    static unsigned int Hz = 50;
    unsigned long ulPeriod;
    

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerIntRegister(TIMER0_BASE, TIMER_A, timer_isr);
    TimerEnable(TIMER0_BASE, TIMER_A); 
    IntEnable(INT_TIMER0A); 
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);  

    //TODO: Explain how this is converting from Hz to ***
    ulPeriod = (SysCtlClockGet() / Hz)/ 2;
    TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod-1);
}

int main(void) {
    SysCtlClockSet (SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    uart_init();
    inputs_adc_init();
    timer_init();

    xTaskCreate(&greet, "greet", 256, NULL, 0, NULL);

    vTaskStartScheduler();

    return 0;
}

void vAssertCalled( const char * pcFile, unsigned long ulLine ) {
    (void)pcFile; // unused
    (void)ulLine; // unused
    while (true) ;
}

//Comment