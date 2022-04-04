/** @file   target.h
    @author M. P. Hayes, UCECE
    @date   12 February 2018
    @brief
*/
#ifndef TARGET_H
#define TARGET_H

#include "mat91lib.h"

/* This is for the carhat (chart) board configured as a hat!  */

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
#define MPU_ADDRESS 0x68

/* USB  */
//#define USB_VBUS_PIO PA5_PIO
#define USB_CURRENT_MA 500

/* ADC  */
#define ADC_BATTERY PB3_PIO
#define ADC_JOYSTICK_X PB2_PIO
#define ADC_JOYSTICK_Y PB1_PIO

/* IMU  */
#define IMU_INT_PIO PA0_PIO

/* LEDs  */
#define LED1_PIO PA0_PIO
#define LED2_PIO PA1_PIO

/* General  */
#define APPENDAGE_PIO PA1_PIO
#define SERVO_PWM_PIO PA2_PIO
#define BUZZER_PIO PB0_PIO

/* Battery Voltage Detect*/
#define BATT_V_PIO PA22_PIO

/* Button  */
#define BUTTON_PIO PA16_PIO
#define BUTTON_JOYSTICK PA29_PIO

/* Radio  */
#define RADIO_CS_PIO PA11_PIO
#define RADIO_CE_PIO PA10_PIO
#define RADIO_IRQ_PIO PA9_PIO

/* DIP Switch */
#define DIP_0_PIO PA20_PIO
#define DIP_1_PIO PA23_PIO
#define DIP_2_PIO PA19_PIO
#define DIP_3_PIO PA21_PIO

#define BOARD_HAT 1

#define LEDTAPE_PIO PA3_PIO

#define BUZZER_BUTTON_PIO PA2_PIO

#define SLEEP_BUTTON_PIO PA15_PIO

#define _5V_VALUE 2500 //2585

#define JOYSTICK_SW PA29_PIO

#endif /* TARGET_H  */
