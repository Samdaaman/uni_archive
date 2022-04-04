/**
 * @file buttons.c
 * @author Group 16
 * @brief Code handles button initialisation and polling. Note: initalisation code adapted from Phil Bones Buttons4.c module from ENCE361
 */

#include <stdint.h>
#include <stdbool.h>

#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/debug.h>

#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/tm4c123gh6pm.h>  // Board specific defines (for PF0)

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "buttons.h"


#define STARTING_HEIGHT 20


SemaphoreHandle_t buttons_mutex;    // define mutex for button polling
SemaphoreHandle_t buttons_enable_semaphore;
int16_t desired_height = STARTING_HEIGHT;
int16_t desired_yaw = 0;

static bool but_state[NUM_BUTS];	// Corresponds to the electrical state
static uint8_t but_count[NUM_BUTS];
static bool but_flag[NUM_BUTS];
static bool but_normal[NUM_BUTS];   // Corresponds to the electrical state

static void buttons_update(void);
static uint8_t buttons_check (uint8_t butName);


/**
 * @brief safely returns the desired height of the helicopter (shared data is protected via mutex)
 * @return int16_t the output value of desired height
 */
int16_t buttons_get_desired_height(void)
{
    xSemaphoreTake(buttons_mutex, portMAX_DELAY);
    int16_t desired_height_safe = desired_height;
    xSemaphoreGive(buttons_mutex);
    return desired_height_safe;
}

/**
 * @brief safely returns the desired yaw of the helicopter (shared data is protected via mutex)
 * @return int16_t the output value of desired yaw
 */
int16_t buttons_get_desired_yaw(void)
{
    xSemaphoreTake(buttons_mutex, portMAX_DELAY);
    int16_t desired_yaw_safe = desired_yaw;
    xSemaphoreGive(buttons_mutex);
    return desired_yaw_safe;
}


/**
 * @brief Task used to increment/decrement the values of desired_height and desired_yaw when one of their respective buttons
 * are pressed: 
 * - Left and Right buttons toggle the value of desired_yaw by +- 15 degrees
 * - Up and Down buttons toggle the value of desired_height by +- 10
 * @param args not used
 */
void buttons_poll(void *args) 
{
    (void)args;

    TickType_t wake_time = xTaskGetTickCount();

    // Wait until calibration is done
    xSemaphoreTake(buttons_enable_semaphore, portMAX_DELAY);

    while (true) {
        buttons_update();
        xSemaphoreTake(buttons_mutex, portMAX_DELAY);

        if (buttons_check(UP) == PUSHED)
        {
            desired_height += 10;

            // setting the maximum height of the helicopter to be 100
            if (desired_height > 100)
            {
                desired_height = 100;
            }
        }

        if (buttons_check(DOWN) == PUSHED)
        {
            desired_height -= 10;

            // In this context, the helicopter cannot have a height of less than zero as it's starting point is the ground
            if (desired_height < 0)
            {
                desired_height = 0;
            }
        }

        if (buttons_check(LEFT) == PUSHED)
        {
            desired_yaw -= 15;

            // conditional statement used to keep the yaw positive
            if (desired_yaw < 0)
            {
                desired_yaw += 360;
            }
        }

        if (buttons_check(RIGHT) == PUSHED)
        {
            desired_yaw += 15;

            // convert desired_yaw into degrees
            if (desired_yaw >= 360)
            {
                desired_yaw -= 360;
            }
        }

        xSemaphoreGive(buttons_mutex);
        vTaskDelayUntil( &wake_time, 50);
    }

}


/**
 * @brief function by Phil Bones taken from Buttons4.c - Used to initialise the buttons on the TIVA board
 */
void buttons_init(void)
{
    SysCtlPeripheralEnable (UP_BUT_PERIPH);
    GPIOPinTypeGPIOInput (UP_BUT_PORT_BASE, UP_BUT_PIN);
    GPIOPadConfigSet (UP_BUT_PORT_BASE, UP_BUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
    but_normal[UP] = UP_BUT_NORMAL;

    SysCtlPeripheralEnable (DOWN_BUT_PERIPH);
    GPIOPinTypeGPIOInput (DOWN_BUT_PORT_BASE, DOWN_BUT_PIN);
    GPIOPadConfigSet (DOWN_BUT_PORT_BASE, DOWN_BUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
    but_normal[DOWN] = DOWN_BUT_NORMAL;

    SysCtlPeripheralEnable (LEFT_BUT_PERIPH);
    GPIOPinTypeGPIOInput (LEFT_BUT_PORT_BASE, LEFT_BUT_PIN);
    GPIOPadConfigSet (LEFT_BUT_PORT_BASE, LEFT_BUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    but_normal[LEFT] = LEFT_BUT_NORMAL;

    SysCtlPeripheralEnable (RIGHT_BUT_PERIPH);
    // Note that PF0 is one of a handful of GPIO pins that need to be
    // "unlocked" before they can be reconfigured.  This also requires #include "inc/tm4c123gh6pm.h"
    //---Unlock PF0 for the right button:
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
    GPIO_PORTF_CR_R |= GPIO_PIN_0; //PF0 unlocked
    GPIO_PORTF_LOCK_R = GPIO_LOCK_M;
    
    GPIOPinTypeGPIOInput (RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN);
    GPIOPadConfigSet (RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    but_normal[RIGHT] = RIGHT_BUT_NORMAL;

	for (int i = 0; i < NUM_BUTS; i++)
	{
		but_state[i] = but_normal[i];
		but_count[i] = 0;
		but_flag[i] = false;
	}
}


/**
 * @brief function by Phil Bones taken from Buttons4.c - used for updating the state of each button
 */
static void buttons_update(void)
{
	bool but_value[NUM_BUTS];
	int i;

	// Read the pins; true means HIGH, false means LOW
	but_value[UP] = (GPIOPinRead (UP_BUT_PORT_BASE, UP_BUT_PIN) == UP_BUT_PIN);
	but_value[DOWN] = (GPIOPinRead (DOWN_BUT_PORT_BASE, DOWN_BUT_PIN) == DOWN_BUT_PIN);
    but_value[LEFT] = (GPIOPinRead (LEFT_BUT_PORT_BASE, LEFT_BUT_PIN) == LEFT_BUT_PIN);
    but_value[RIGHT] = (GPIOPinRead (RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN) == RIGHT_BUT_PIN);

	// Iterate through the buttons, updating button variables as required
	for (i = 0; i < NUM_BUTS; i++)
	{
        if (but_value[i] != but_state[i])
        {
        	but_count[i]++;
        	if (but_count[i] >= NUM_BUT_POLLS)
        	{
        		but_state[i] = but_value[i];
        		but_flag[i] = true;	   // Reset by call to buttons_check()
        		but_count[i] = 0;
        	}
        }
        else
        	but_count[i] = 0;
	}
}

/**
 * @brief function by Phil Bones taken from Buttons4.c - used for checking the state of a button
 * @param butName 
 * @return uint8_t outputs the state of the button - PUSHED, RELEASED, NO_CHANGE
 */
static uint8_t buttons_check (uint8_t butName)
{
	if (but_flag[butName])
	{
		but_flag[butName] = false;
		if (but_state[butName] == but_normal[butName])
			return RELEASED;
		else
			return PUSHED;
	}
	return NO_CHANGE;
}
