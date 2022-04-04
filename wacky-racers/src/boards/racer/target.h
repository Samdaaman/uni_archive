/** @file   target.h
    @author M. P. Hayes, UCECE
    @date   12 February 2018
    @brief 
*/
#ifndef TARGET_H
#define TARGET_H

#include "mat91lib.h"

/* This is for the carhat (chart) board configured as a racer!  */

/* System clocks  */
#define F_XTAL 12.00e6
#define MCU_PLL_MUL 16
#define MCU_PLL_DIV 1

#define MCU_USB_DIV 2
/* 192 MHz  */
#define F_PLL (F_XTAL / MCU_PLL_DIV * MCU_PLL_MUL)
/* 96 MHz  */
#define F_CPU (F_PLL / 2)

/* TWI  */
#define TWI_TIMEOUT_US_DEFAULT 10000

/* USB  */
#define USB_VBUS_PIO PA5_PIO
#define USB_CURRENT_MA 500

/* LEDs  */
#define LED1_PIO PA0_PIO
#define LED2_PIO PA1_PIO

/* General  */
#define APPENDAGE_PIO PA1_PIO
#define SERVO_PWM_PIO PA2_PIO


/* Battery Voltage Detect*/
#define BATT_V_PIO ADC_CHANNEL_9
#define _5V_VALUE 2700 //2585

/* Button  */ 
#define BUTTON_PIO PA2_PIO  //Currently Set to the bumper

/*Bumper*/
#define BUMPER_PIO PA6_PIO  //Currently Set to the bumper

/* H-bridges U1  */  
#define MOTOR_LEFT_PWM_PIO PA30_PIO  //Setting the 1 pins (i.e L_IN1) to be PWM
#define MOTOR_LEFT_MODE_PIO PA29_PIO //Setting the 2 pins to be mode
#define MOTOR_RIGHT_PWM_PIO PA28_PIO
#define MOTOR_RIGHT_MODE_PIO PA27_PIO
#define MOTOR_SLEEP_PIO PA31_PIO


/* H-bridges U2  */
#define U2_MOTOR_LEFT_PWM_PIO PA19_PIO  //AIN2
#define U2_MOTOR_LEFT_MODE_PIO PA23_PIO //AIN1
#define U2_MOTOR_RIGHT_PWM_PIO PA20_PIO //BIN2
#define U2_MOTOR_RIGHT_MODE_PIO PA24_PIO //BIN2
#define U2_MOTOR_SLEEP_PIO PA25_PIO


//Singled these defines out as they don't look like they are used
#define MOTOR_LEFT_PHASE_PIO PB4_PIO
#define MOTOR_RIGHT_PHASE_PIO PA6_PIO

/* Radio  */
#define RADIO_CS_PIO PA11_PIO
#define RADIO_CE_PIO PA10_PIO
#define RADIO_IRQ_PIO PA9_PIO

/* DIP Switch */
#define DIP_0_PIO PA21_PIO
#define DIP_1_PIO PA18_PIO
#define DIP_2_PIO PA17_PIO
#define DIP_3_PIO PA16_PIO

// #define DIP_0_PIO PA16_PIO
// #define DIP_1_PIO PA17_PIO
// #define DIP_2_PIO PA18_PIO
// #define DIP_3_PIO PA21_PIO

/*LED Tap*/
#define LEDTAPE_PIO PA3_PIO



#define BOARD_RACER 1

#define SLEEP_BUTTON_PIO PA15_PIO

#endif /* TARGET_H  */
