/**
 * @file debug.h
 * @author Group 16
 * @brief header file for the module that interfaces with the Tiva board's serial interface
 */


#ifndef DEBUG_H
#define DEBUG_H


#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <driverlib/pin_map.h>
#include <utils/ustdlib.h>

#include <FreeRTOS.h>
#include <queue.h>


#define MAX_SERIAL_LINE_LENGTH 50
#define SERIAL_QUEUE_LENGTH 6


/**
 * Strings written to this queue should be no longer than MAX_SERIAL_LINE_LENGTH long
 * and should be null terminated.
 */
extern QueueHandle_t debug_messages;


/**
 * @brief Initializes the serial interface module. Must be called before the serial task is created.
 */
void debug_init();


/**
 * @brief Prints to the serial interface in a way that is NOT "threadsafe"
 * @param str A null terminated string, best to also be /r/n terminated
 */
void debug_uart_puts_unsafe(char *str);


/**
 * @brief Task that reads strings from the queue to the serial interface
 */
void debug_poll(void* args);


/**
 * @brief a "task/thread-safe" printf like function that sends to the serial interface queue
 * @param fmt null terminated string, to be formatted with the remaining arguments.
 */
void debug_printf(const char *format, ...);


#endif
