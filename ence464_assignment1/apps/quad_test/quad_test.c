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
        int qeiPosition = QEIPositionGet(QEI0_BASE);

        char greeting[100];

        usnprintf(greeting, sizeof(greeting), "Hello %i, quad=%i\r\n", count++, qeiPosition);
        uart_puts(greeting);

        vTaskDelayUntil( &wake_time, 500 );
    }
}

void read(void* args) {
    (void)args; // unused

    TickType_t wake_time = xTaskGetTickCount();

    while (true) {
        // GPIOPinWrite(GPIO_PORTF_BASE, LED_B, !(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_6));
        bool switch_state = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_6) & GPIO_PIN_6;
        GPIOPinWrite(GPIO_PORTF_BASE, LED_B, switch_state ? LED_B : 0);
        vTaskDelayUntil( &wake_time, 200 );
    }
}

void quad_init(void)
{
    // Enable QEI Peripherals
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);

	//Unlock GPIOD7 - Like PF0 its used for NMI - Without this step it doesn't work
	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY; //In Tiva include this is the same as "_DD" in older versions (0x4C4F434B)
	HWREG(GPIO_PORTD_BASE + GPIO_O_CR) |= 0x80;
	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = 0;

	//Set Pins to be PHA0 and PHB0
	GPIOPinConfigure(GPIO_PD6_PHA0);
	GPIOPinConfigure(GPIO_PD7_PHB0);

	//Set GPIO pins for QEI. PhA0 -> PD6, PhB0 ->PD7. I believe this sets the pull up and makes them inputs
	GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_6 |  GPIO_PIN_7);

	//DISable peripheral and int before configuration
	QEIDisable(QEI0_BASE);
	QEIIntDisable(QEI0_BASE,QEI_INTERROR | QEI_INTDIR | QEI_INTTIMER | QEI_INTINDEX);

	// Configure quadrature encoder, use an arbitrary top limit of 1000
	QEIConfigure(QEI0_BASE, (QEI_CONFIG_CAPTURE_A_B  | QEI_CONFIG_NO_RESET 	| QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP), 1000);

	// Enable the quadrature encoder.
	QEIEnable(QEI0_BASE);

	//Set position to a middle value so we can see if things are working
	QEIPositionSet(QEI0_BASE, 500);
}


int main(void) {
    SysCtlClockSet (SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(LED_PERIPH);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(LED_PORT, LED_RGB);
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_6);

    uart_init();
    quad_init();

    // xTaskCreate(&blink, "blink", 256, NULL, 0, NULL);
    xTaskCreate(&greet, "greet", 256, NULL, 0, NULL);
    xTaskCreate(&read, "read", 256, NULL, 0, NULL);

    vTaskStartScheduler();

    return 0;
}

void vAssertCalled( const char * pcFile, unsigned long ulLine ) {
    (void)pcFile; // unused
    (void)ulLine; // unused
    while (true) ;
}