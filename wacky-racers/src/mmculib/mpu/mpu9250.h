/** @file   mpu9250.h
    @author Andre Renaud
    @date   February 2020
    @brief  Interface routines for the MPU 9250 IMU chip
*/

#ifndef MPU9250_H
#define MPU9250_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "config.h"
#include "twi.h"

/** Define datatype for handle to MPU functions.  */
typedef struct {
	twi_t twi; /* TWI bus */
	twi_slave_addr_t imu_addr; /* Address of the MPU9250 - one of 0x68 or 0x69 */
} mpu_t;

/**
 * Initialise the MPU object, attached on a TWI/I2C bus
 *
 * @param twi TWI bus on which the MPU device exists
 * @parma slave_addre TWI address of the MPU device
 * @return MPU object, or NULL on failure
 */
mpu_t *mpu9250_create(twi_t twi, twi_slave_addr_t slave_addr);

/**
 * Return true if IMU has data ready.
 *
 * @param mpu MPU object pointer
 * @return true if the MPU has IMU data ready, false otherwise
 */
bool mpu9250_is_imu_ready(mpu_t *mpu);

/**
 * Return raw accelerometer data.
 *
 * @param mpu MPU object pointer
 * @param acceldata Array of 3 16-bit integers to store the accelerometer
 *        data into
 * @return true if the data was read successfully, false otherwise
 */
bool mpu9250_read_accel(mpu_t *mpu, int16_t acceldata[3]);

/**
 * Return raw gyroscope data.
 *
 * @param mpu MPU object pointer
 * @param gyrodata Array of 3 16-bit integers to store the gyro
 *        data into
 * @return true if the data was read successfully, false otherwise
 */
bool mpu9250_read_gyro(mpu_t *mpu, int16_t gyrodata[3]);

/**
 * Return true if magnetometer has data ready.
 *
 * @param mpu MPU object pointer
 * @return true if the MPU has magnetometer data ready, false otherwise
 */
bool mpu9250_is_mag_ready(mpu_t *mpu);

/**
 * Return raw magetometer data.
 *
 * @param mpu MPU object pointer
 * @param magdata Array of 3 16-bit integers to store the magnetometer
 *        data into.
 * @return true if the data was read successfully, false otherwise
 */
bool mpu9250_read_mag(mpu_t *mpu, int16_t magdata[3]);

#ifdef __cplusplus
}
#endif

#endif