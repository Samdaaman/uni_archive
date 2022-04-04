/**
 * @file master_v1.c
 * @author Group 16
 * @brief App for the Tiva board to control the height and yaw of the HeliRig emulator
 */


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
#include <queue.h>
#include <semphr.h>
#include <task.h>

#include "buttons.h"
#include "calibration.h"
#include "control.h"
#include "debug.h"
#include "inputs.h"
#include "pwm.h"
#include "display.h"


#define LED_PERIPH  SYSCTL_PERIPH_GPIOF
#define LED_PORT    GPIO_PORTF_BASE
#define LED_R       GPIO_PIN_1
#define LED_G       GPIO_PIN_3
#define LED_B       GPIO_PIN_2
#define LED_RGB     (LED_R | LED_G | LED_B)


static void call_inits();
static void create_tasks();
static bool create_queues();
static bool create_semaphores();


/**
 * @brief Create the two FreeRTOS queues
 * @return true if a queue failed to be created
 */
static bool create_queues()
{
    debug_messages = xQueueCreate(SERIAL_QUEUE_LENGTH, sizeof(char) * MAX_SERIAL_LINE_LENGTH);
    display_messages = xQueueCreate(DISPLAY_QUEUE_LENGTH, sizeof(Message));

    return debug_messages == NULL || display_messages == NULL;
}


/**
 * @brief Create the FreeRTOS semaphores and mutex
 * @return true if a semaphore or mutex failed to be created
 */
static bool create_semaphores()
{
    buttons_mutex = xSemaphoreCreateMutex();
    buttons_enable_semaphore = xSemaphoreCreateBinary();
    calibration_completed_height_semaphore = xSemaphoreCreateBinary();
    calibration_completed_yaw_semaphore = xSemaphoreCreateBinary();
    inputs_quad_reference_reset_semaphore = xSemaphoreCreateBinary();

    return buttons_mutex == NULL
        || buttons_enable_semaphore == NULL
        || calibration_completed_height_semaphore == NULL
        || calibration_completed_yaw_semaphore == NULL
        || inputs_quad_reference_reset_semaphore == NULL;
}


/**
 * @brief calls all the modules' initialisation functions
 */
static void call_inits()
{
    buttons_init();
    inputs_adc_init();
    inputs_quad_init();
    debug_init();
    pwm_init();
    display_init();
    timer_init();
}


/**
 * @brief Creates the FreeRTOS tasks
 */
static void create_tasks()
{
    xTaskCreate(&buttons_poll, "button control", 256, NULL, 5, NULL);
    xTaskCreate(&control_height_poll, "control_height_poll", 256, NULL, 4, NULL);
    xTaskCreate(&control_yaw_poll, "control_yaw_poll", 256, NULL, 4, NULL);
    xTaskCreate(&calibration_poll, "calibration_poll", 256, NULL, 0, NULL);
    xTaskCreate(&debug_poll, "debug_poll", 256, NULL, 2, NULL);
    xTaskCreate(&display_poll, "display_poll", 256, NULL, 3, NULL);
}


int main(void)
{
    SysCtlClockSet (SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    if (create_queues()) {
        debug_uart_puts_unsafe("Oh no, one of the queues could't be created\r\n");
        return 1;
    }

    if (create_semaphores()) {
        debug_uart_puts_unsafe("Oh no, one of the semaphores/mutexs could't be created\r\n");
        return 1;
    }

    call_inits();
    create_tasks();
    debug_printf("Serial test 123=%i\r\n", 123);

    vTaskStartScheduler();
    return 0;
}


void vAssertCalled( const char * pcFile, unsigned long ulLine )
{
    (void)pcFile;
    (void)ulLine;
    #ifdef DEBUG // If in debugging mode print errors to the serial
    debug_uart_puts_unsafe("vAssertCalled (this is bad)\r\n");
    debug_uart_puts_unsafe(pcFile);
    debug_uart_puts_unsafe("-----\r\n");
    debug_printf("line: %i\r\n", ulLine);
    #endif
    while (true);
}
