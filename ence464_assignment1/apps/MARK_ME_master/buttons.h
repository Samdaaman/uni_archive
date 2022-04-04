#ifndef BUTTONS_H_
#define BUTTONS_H_

/**
 * @file buttons.h
 * @author Group 16
 * @brief Methods for initialising and monitoring buttons on the TIVA board - adapted from Phil Bones buttons4.h module from ENCE361
 */

#include <stdint.h>
#include <stdbool.h>
#include <semphr.h>

enum butNames {UP = 0, DOWN, LEFT, RIGHT, NUM_BUTS};
enum butStates {RELEASED = 0, PUSHED, NO_CHANGE};

#define UP_BUT_PERIPH  SYSCTL_PERIPH_GPIOE
#define UP_BUT_PORT_BASE  GPIO_PORTE_BASE
#define UP_BUT_PIN  GPIO_PIN_0
#define UP_BUT_NORMAL  false

#define DOWN_BUT_PERIPH  SYSCTL_PERIPH_GPIOD
#define DOWN_BUT_PORT_BASE  GPIO_PORTD_BASE
#define DOWN_BUT_PIN  GPIO_PIN_2
#define DOWN_BUT_NORMAL  false

#define LEFT_BUT_PERIPH  SYSCTL_PERIPH_GPIOF
#define LEFT_BUT_PORT_BASE  GPIO_PORTF_BASE
#define LEFT_BUT_PIN  GPIO_PIN_4
#define LEFT_BUT_NORMAL  true

#define RIGHT_BUT_PERIPH  SYSCTL_PERIPH_GPIOF
#define RIGHT_BUT_PORT_BASE  GPIO_PORTF_BASE
#define RIGHT_BUT_PIN  GPIO_PIN_0
#define RIGHT_BUT_NORMAL  true

#define SW1_PERIPH       SYSCTL_PERIPH_GPIOA
#define SW1_PORT_BASE    GPIO_PORTA_BASE
#define SW1_PIN          GPIO_PIN_7

#define NUM_BUT_POLLS 3


/**
 * @brief variable to hold mutex for button polling and accessing height and yaw data
 */
extern SemaphoreHandle_t buttons_mutex;

/**
 * @brief variable to hold the state of the semaphore used for button polling and accessing height and yaw data
 */
extern SemaphoreHandle_t buttons_enable_semaphore;


/**
 * @brief Task to poll the buttons on the TIVA board (UP, DOWN, LEFT, RIGHT)
 * @param args not used 
 */
void buttons_poll(void *args);

/**
 * @brief Function to return the value of desired_height in a way that is thread-safe
 * @return int16_t output is the desired_height
 */
int16_t buttons_get_desired_height(void);

/**
 * @brief Fucntion to return the value of desired_yaw in a way that is thread-safe
 * @return int16_t output is the desired_yaw
 */
int16_t buttons_get_desired_yaw(void);

/**
 * @brief Initialises the buttons on the TIVA board for use in further functions
 */
void buttons_init (void);


#endif /*BUTTONS_H_*/