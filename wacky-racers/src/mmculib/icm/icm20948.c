/** @file   icm20948.c
    @author Andre Renaud, UCECE
    @date   July 2020
*/

#include <stdio.h>
#include <string.h>

#include "delay.h"
#include "icm20948.h"
#include "twi.h"

#define ICM_REG(bank, reg) ((bank) << 8 | (reg))
#define ICM_BANK_ID(reg) ((((reg) & 0x300) >> 4))
#define ICM_REG_ID(reg) ((reg) & 0xff)

#define ICM_EXPECTED_ID   0xea
#define ICM_REG_BANK_SEL 0x7f

#define ICM_INT_STATUS ICM_REG(0, 0x19)
#define ICM_USER_CTRL ICM_REG(0, 0x03)
#define ICM_I2C_MST_CTRL ICM_REG(3, 0x01)
#define ICM_I2C_MST_DELAY_CTRL ICM_REG(3, 0x02)
#define ICM_INT_ENABLE ICM_REG(0, 0x10)
#define ICM_ACCEL_XOUT_H ICM_REG(0, 0x2d)
#define ICM_WHO_AM_I ICM_REG(0, 0x00)
#define ICM_PWR_MGMT_1 ICM_REG(0, 0x06)
#define ICM_PWR_MGMT_2 ICM_REG(0, 0x07)
#define ICM_GYRO_XOUT_H ICM_REG(0, 0x33)
#define ICM_INT_PIN_CFG ICM_REG(0, 0x0f)
#define ICM_SMPLRT_DIV ICM_REG(2, 0x0)
#define ICM_ACCEL_CONFIG ICM_REG(2, 0x14)
#define ICM_GYRO_CONFIG_1 ICM_REG(2, 0x01)
#define ICM_TEMP_OUT_H ICM_REG(0, 0x39)
#define ICM_ACCEL_SMPLRT_DIV_1 ICM_REG(2, 0x10)
#define ICM_ACCEL_SMPLRT_DIV_2 ICM_REG(2, 0x11)

#define ICM_I2C_SLV0_ADDR ICM_REG(3, 0x03)
#define ICM_I2C_SLV0_REG ICM_REG(3, 0x04)
#define ICM_I2C_SLV0_CTRL ICM_REG(3, 0x05)
#define ICM_I2C_SLV0_DO ICM_REG(3, 0x06)

#define ICM_EXT_SLV_SENS_DATA_00 ICM_REG(0, 0x3B)

#define ICM_MAG_WIA2 0x01
#define ICM_MAG_CNTL2 0x31
#define ICM_MAG_CNTL3 0x32
#define ICM_MAG_HXL 0x11

// We only support a single icm, so we will use a single
// static allocation to store it.
static icm_t _static_icm = {};

static bool icm_ensure_bank(icm_t *icm, const int addr)
{
    uint8_t new_bank = ICM_BANK_ID(addr);
    twi_ret_t status;

    if (new_bank == icm->bank)
        return true;
    status = twi_master_addr_write(icm->twi, icm->imu_addr, ICM_REG_BANK_SEL, 1, &new_bank, 1);
    if (status == 1)
        icm->bank = new_bank;
    return status == 1;
}

static uint8_t icm_imu_read(icm_t *icm, const int addr)
{
    uint8_t response = 0;
    twi_ret_t status;

    if (!icm_ensure_bank(icm, addr))
        return 0x0;

    status = twi_master_addr_read(icm->twi, icm->imu_addr, ICM_REG_ID(addr), 1, &response, 1);
    if (status != 1)
        return 0x0;

    return response;
}

static bool icm_imu_write(icm_t *icm, const int addr, const uint8_t value)
{
    twi_ret_t status;
    if (!icm_ensure_bank(icm, addr))
        return false;
    status = twi_master_addr_write(icm->twi, icm->imu_addr, ICM_REG_ID(addr), 1, &value, 1);

    // Some register writes appear to take some time to actuate during initialisation
    DELAY_US(1000);
    return (status == 1);
}

static bool icm_mag_write(icm_t *icm, const uint8_t addr, const uint8_t value)
{
  	if (!icm_imu_write(icm, ICM_I2C_SLV0_ADDR, 0x0C)) //mode: write
  		return false;

  	if (!icm_imu_write(icm, ICM_I2C_SLV0_REG, addr)) //set reg addr
  		return false;

	if (!icm_imu_write(icm, ICM_I2C_SLV0_DO, value)) //send value
		return false;

	return true;
}

static bool icm_mag_read_count(icm_t *icm, const uint8_t addr, const uint8_t count, uint8_t *data)
{
	twi_ret_t status;

  	if (!icm_imu_write(icm, ICM_I2C_SLV0_ADDR, 0x80 | 0x0c))
  		return false;
  	if (!icm_imu_write(icm, ICM_I2C_SLV0_REG, addr))
  		return false;
  	if (!icm_imu_write(icm, ICM_I2C_SLV0_CTRL, 0x80 | count))
  		return false;

    if (!icm_ensure_bank(icm, ICM_EXT_SLV_SENS_DATA_00))
        return false;

    status = twi_master_addr_read(icm->twi, icm->imu_addr, ICM_REG_ID(ICM_EXT_SLV_SENS_DATA_00), 1,
     	data, count);
    if (status != count)
        return false;

    return true;
}

// Reads 6 bytes, assumed to be 3x16-bit words
static bool icm20948_read_six(icm_t *icm, const int addr, int16_t data[3])
{
	twi_ret_t status;
    uint8_t rawdata[6];

    memset(data, 0, 6);

	if (!icm_ensure_bank(icm, addr))
		return false;
	status = twi_master_addr_read(icm->twi, icm->imu_addr, ICM_REG_ID(addr),
		1, rawdata, 6);
	if (status != 6)
		return false;

    data[0] = ((int16_t)rawdata[0] << 8) | rawdata[1];
    data[1] = ((int16_t)rawdata[2] << 8) | rawdata[3];  
    data[2] = ((int16_t)rawdata[4] << 8) | rawdata[5]; 
    
    return true;
}

bool icm20948_read_gyro(icm_t *icm, int16_t gyrodata[3])
{
	return icm20948_read_six(icm, ICM_GYRO_XOUT_H, gyrodata);
}

bool icm20948_read_accel(icm_t *icm, int16_t acceldata[3])
{
	return icm20948_read_six(icm, ICM_ACCEL_XOUT_H, acceldata);
}

bool icm20948_read_mag(icm_t *icm, int16_t magdata[3])
{
	uint8_t data[6];

	if (!icm_mag_read_count(icm, ICM_MAG_HXL, 6, data))
		return false;

	magdata[0] = ((int16_t)data[0] << 8) | data[1];
	magdata[1] = ((int16_t)data[2] << 8) | data[3];
	magdata[2] = ((int16_t)data[4] << 8) | data[5];

	return true;
}

bool icm20948_is_imu_ready(icm_t *icm)
{
    return (icm_imu_read(icm, ICM_INT_STATUS) & 0x01) != 0;
}

bool icm20948_read_temperature(icm_t *icm, int *temp)
{
	uint8_t rawdata[2];
	twi_ret_t status;

	*temp = 0;
	if (!icm_ensure_bank(icm, ICM_TEMP_OUT_H))
		return false;

	status = twi_master_addr_read(icm->twi, icm->imu_addr, ICM_REG_ID(ICM_TEMP_OUT_H),
		1, rawdata, 2);
	if (status != 2)
		return false;

	// TODO: What is the units? 0.05 degrees? Offset by 21 degrees?
	*temp = (rawdata[0] << 8) | (rawdata[1]);
	return true;
}

icm_t *icm20948_create(twi_t twi, twi_slave_addr_t slave_addr)
{
	icm_t *icm = &_static_icm;
	struct {
		int addr;
		uint8_t value;
	} init_sequence[] = {
		{ICM_PWR_MGMT_1, 1 << 7}, // Reset
		{ICM_PWR_MGMT_1, 1}, // Set auto clock source
		{ICM_PWR_MGMT_2, 0x38 | 0x07}, // Disable accelerometer & gryo
		{ICM_PWR_MGMT_2, 0}, // Enable accerometer & gryo
		{ICM_ACCEL_CONFIG, (3 << 1) | (1 << 0)},
		{ICM_GYRO_CONFIG_1, (3 << 1) | (1 << 0)}, // Gryo @ 2000dps, 197Hz
		{ICM_INT_ENABLE, (1 << 0)},
		{ICM_SMPLRT_DIV, 0},
		{ICM_ACCEL_SMPLRT_DIV_1, 0},
		{ICM_ACCEL_SMPLRT_DIV_2, 0x0a},
		{ICM_INT_PIN_CFG, 0x30}, // int pin / bypass enable config
		{ICM_USER_CTRL, 0x20}, // i2c_mst_en
		{ICM_I2C_MST_CTRL, 0x4d}, // i2c master & speed 400khz
		{ICM_I2C_MST_DELAY_CTRL, 0x01}, // i2c_slv0 _DLY_ enable
	};
	uint8_t wia;

	icm->twi = twi;
	icm->imu_addr = slave_addr;
	icm->bank = -1; // force it to set the bank on the first read

	if (icm_imu_read(icm, ICM_WHO_AM_I) != ICM_EXPECTED_ID)
		return NULL;

	// Initialise accelerometer/gryo
	for (unsigned i = 0; i < sizeof(init_sequence) / sizeof(init_sequence[0]); i++) {
		if (!icm_imu_write(icm, init_sequence[i].addr, init_sequence[i].value))
			return NULL;
	}

	// Reset magnetometer
	if (!icm_mag_write(icm, ICM_MAG_CNTL3, 0x01))
		return NULL;

	// Check that it appears properly
	if (!icm_mag_read_count(icm, ICM_MAG_WIA2, 1, &wia) || wia != 0x09)
		return NULL;

	// Put it into continuous measurement mode, 100Hz
	if (!icm_mag_write(icm, ICM_MAG_CNTL2, 0x01))
		return NULL;

	return icm;
}
