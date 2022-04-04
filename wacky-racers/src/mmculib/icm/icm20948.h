/** @file   icm20948.h
    @author Andre Renaud
    @date   July 2020
    @brief  Interface routines for the ICM 20948 IMU chip
*/

#ifndef ICM20948_H
#define ICM20948_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "config.h"
#include "twi.h"

/** Define datatype for handle to ICM20948 functions.  */
typedef struct {
	twi_t twi; /* TWI bus */
	twi_slave_addr_t imu_addr; /* Address of the ICM20948 - one of 0x68 or 0x69 */
	int bank;  /* Which bank in the ICM 20948 is currently active */
} icm_t;

/**
 * Initialise the ICM20948 object, attached on a TWI/I2C bus
 * NOTE: Only one of these can currently be created
 *
 * @param twi TWI bus on which the ICM20948 device exists
 * @parma slave_addre TWI address of the ICM20948 device
 * @return ICM20948 object, or NULL on failure
 */
icm_t *icm20948_create(twi_t twi, twi_slave_addr_t slave_addr);

/**
 * Return true if IMU has data ready.
 *
 * @param ICM20948 object pointer
 * @return true if the ICM20948 has IMU data ready, false otherwise
 */
bool icm20948_is_imu_ready(icm_t *icm);

/**
 * Return raw accelerometer data.
 *
 * @param icm ICM20948 object pointer
 * @param acceldata Array of 3 16-bit integers to store the accelerometer
 *        data into
 * @return true if the data was read successfully, false otherwise
 */
bool icm20948_read_accel(icm_t *icm, int16_t acceldata[3]);

/**
 * Return raw gyroscope data.
 *
 * @param icm ICM20948 object pointer
 * @param gyrodata Array of 3 16-bit integers to store the gyro
 *        data into
 * @return true if the data was read successfully, false otherwise
 */
bool icm20948_read_gyro(icm_t *icm, int16_t gyrodata[3]);

/**
 * Return true if magnetometer has data ready.
 *
 * @param icm ICM20948 object pointer
 * @return true if the ICM20948 has magnetometer data ready, false otherwise
 */
bool icm20948_is_mag_ready(icm_t *icm);

/**
 * Return raw magetometer data.
 *
 * @param icm ICM20948 object pointer
 * @param magdata Array of 3 16-bit integers to store the magnetometer
 *        data into.
 * @return true if the data was read successfully, false otherwise
 */
bool icm20948_read_mag(icm_t *icm, int16_t magdata[3]);

/**
 * Return the temperature of the ICM
 *
 * @param icm ICM20948 object pointer
 * @param temp Pointer to the integer to store the temperature,
 *             in 1/100th of a degree
 * @return true if the temperature was read successfully, false otherwise
 */

bool icm20948_read_temperature(icm_t *icm, int *temp);

#ifdef __cplusplus
}
#endif

#endif