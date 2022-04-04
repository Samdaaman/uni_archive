/**
 * @file display.c
 * @author Group 16
 * @brief Module that interfaces with the Tiva board's display.
 */


#include "display.h"


QueueHandle_t display_messages;


/**
 * @brief Initialises the Orbit OLED display
 */
void display_init()
{
    OLEDInitialise();
}


/**
 * Starts by showing that the yaw and height are being calibrated on the first and second line.
 * This will be overwritten by the first readings for each.
 * 
 * The function reads messages from the display queue and displays them on the Tiva board's OLED screen.
 * 
 * Four lines on the display:
 *  First line is for yaw
 *  Second line is for height
 *  Third line is for the top rotor's duty cycle
 *  Fourth line is for the tail rotor's duty cycle
 */
void display_poll(void* args)
{
    (void)args; // unused

    Message message;
    char string[17] = {0}; // Display fits 16 characters wide.

    usnprintf(string, sizeof(string), "Calibrating H...");
    OLEDStringDraw(string, 0, 0);

    usnprintf(string, sizeof(string), "Calibrating Y...");
    OLEDStringDraw(string, 0, 1);

    while (true) {

        xQueueReceive(display_messages, &message, portMAX_DELAY);

        switch(message.type) {
            case Height:
                usnprintf(string, sizeof(string), "H: %4d%%, %4d%%", message.value[0], message.value[1]);
                OLEDStringDraw(string, 0, 0);
                break;
            case Yaw:
                usnprintf(string, sizeof(string), "Y:%4d,%4d deg", message.value[0], message.value[1]);
                OLEDStringDraw(string, 0, 1);
                break;
            case TopDuty:
                usnprintf(string, sizeof(string), "Top  DC: %4d%%", message.value[0]);
                OLEDStringDraw(string, 0, 2);
                break;
            case TailDuty:
                usnprintf(string, sizeof(string), "Tail DC: %4d%%", message.value[0]);
                OLEDStringDraw(string, 0, 3);
                break;
        }
    }
}


/**
 * @brief 
 * 
 * @param type 
 * @param value0 
 * @param value1 
 */
void display_send_message(MessageType type, int16_t value0, int16_t value1) {
    Message message = {
            .type = type,
            .value = {
                value0,
                value1,
            }
        };
        xQueueSend(display_messages, &message, portMAX_DELAY);
}


/**
 * Tests the display task by writing messages to the display queue.
 */
void display_dummy(void* args)
{
    (void)args; // unused
    TickType_t wake_time = xTaskGetTickCount();
    int index = 0;

    while (true) {
        Message message;
        // debug_printf("Display index: %d\r\n", index); // Need to include debug.h for this
        switch (index % 4) {
            case 0:
                message.type = Yaw;
                message.value[0] = index;
                message.value[1] = index + 1;
                xQueueSend(display_messages, &message, portMAX_DELAY);
                break;
            case 1:
                message.type = Height;
                message.value[0] = index;
                message.value[1] = index + 1;
                xQueueSend(display_messages, &message, portMAX_DELAY);
                break;
            case 2:
                message.type = TopDuty;
                message.value[0] = index;
                message.value[1] = 0; // unused
                xQueueSend(display_messages, &message, portMAX_DELAY);
                break;
            case 3:
                message.type = TailDuty;
                message.value[0] = index;
                message.value[1] = 0; // unused
                xQueueSend(display_messages, &message, portMAX_DELAY);
                break;
        }
        index++;
        vTaskDelayUntil( &wake_time, 250);
    }
}
