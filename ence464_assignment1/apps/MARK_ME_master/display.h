/**
 * @file display.h
 * @author Group 16
 * @brief Header file for the module that interfaces with the Tiva board's display
 */


#ifndef UI_H
#define UI_H


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_ints.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <driverlib/pin_map.h>
#include <utils/ustdlib.h>
#include <driverlib/debug.h>

#include "OrbitOLED/OrbitOLEDInterface.h"

#include <FreeRTOS.h>
#include <queue.h>


#define DISPLAY_QUEUE_LENGTH 10


typedef enum {Height, Yaw, TopDuty, TailDuty } MessageType;


/**
 * @brief The type for messages that the display function reads from the queue to the display
 * 
 * What value represents depends on the message type
 * Yaw: { actual, desired }
 * Height: { actual, desired }
 * TopDuty: { top rotor's duty cycle, unused }
 * TailDuty: { tail rotor's duty cycle, unused }
 */
typedef struct DisplayMessage {
    MessageType type;
    int16_t value[2];
} Message;


extern QueueHandle_t display_messages;


/**
 * @brief Initialisation function for the display. To be called before creating the display task.
 */
void display_init();


/**
 * @brief Task that reads message and writes to the display.
 */
void display_poll(void* args);


/**
 * @brief Forms a queue message from the given values and adds it to the queue.
 * 
 * @param type The message's type
 * @param value0 The message's first value
 * @param value1 The message's second value
 */
void display_send_message(MessageType type, int16_t value0, int16_t value1);


/**
 * @brief Task to run to test the display task.
 */
void display_dummy(void* args);

#endif
