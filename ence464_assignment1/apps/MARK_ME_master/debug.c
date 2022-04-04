/**
 * @file debug.c
 * @author Group 16
 * @brief Description of file
 */


#include "debug.h"


QueueHandle_t debug_messages;


/**
 * Initializes the serial interface module. Must be called before the serial task is created.
 */
void debug_init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinConfigure (GPIO_PA0_U0RX);
    GPIOPinConfigure (GPIO_PA1_U0TX);

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600, UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
    UARTFIFOEnable(UART0_BASE);
    UARTEnable(UART0_BASE);

    debug_uart_puts_unsafe("Serial initialised\r\n");
}


/**
 * Prints to the serial interface in a way that is NOT "threadsafe"
 * Should only be called in extreme circumstances or if something is wrong with tasks
 * @param str A null terminated string, best to also be /r/n terminated
 */
void debug_uart_puts_unsafe(char* str)
{
    while (*str) {
        UARTCharPut(UART0_BASE, *str++);
    }
}


/**
 * Reads strings from the queue to the serial interface.
 * @param args unused
 */
void debug_poll(void* args)
{
    (void)args; // unused
    while (true) {
        char line[MAX_SERIAL_LINE_LENGTH];
        if ( xQueueReceive(debug_messages, line, portMAX_DELAY) ) {
            debug_uart_puts_unsafe(line);
        }
    }
}


/**
 * A "task/thread-safe" printf like function that sends to the serial interface queue
 * @param format null terminated string, to be formatted with the remaining arguments.
 */
void debug_printf(const char *format, ...)
{
    char buffer[MAX_SERIAL_LINE_LENGTH] = {0};
    va_list extra_args;
    va_start(extra_args, format);
    uvsnprintf(buffer, MAX_SERIAL_LINE_LENGTH - 1, format, extra_args);
    xQueueSend(debug_messages, buffer, portMAX_DELAY);
    va_end(extra_args);
}
