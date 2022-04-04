#include "delay.h"
#include "twi.h"
#include "mpu9250.h"

// See https://github.com/Snowda/MPU9250
// https://github.com/kriswiner/MPU9250

/* Max 400 kHz.  */
#ifndef TWI_CLOCK_SPEED_KHZ
#define TWI_CLOCK_SPEED_KHZ 100
#endif


#define MPU9250_MAG_SLAVE_ADDR 0x0C

#define MPU9250_MAG_DEVICE_ID 0x00
#define MPU9250_MAG_INFO      0x01
#define MPU9250_MAG_ST1       0x02
#define MPU9250_MAG_HXL       0x03
#define MPU9250_MAG_HXH       0x04
#define MPU9250_MAG_HYL       0x05
#define MPU9250_MAG_HYH       0x06
#define MPU9250_MAG_HZL       0x07
#define MPU9250_MAG_HZH       0x08
#define MPU9250_MAG_ST2       0x09
#define MPU9250_MAG_CNTL1     0x0A
#define MPU9250_MAG_CNTL2     0x0B
#define MPU9250_MAG_ASTC      0x0C
#define MPU9250_MAG_I2CDIS    0x0F
#define MPU9250_MAG_ASAX      0x10
#define MPU9250_MAG_ASAY      0x11
#define MPU9250_MAG_ASAZ      0x12

/* This can be 0x69 if the AD0 pin is high.  */
#define MPU9250_IMU_SLAVE_ADDR 0x68

#define MPU9250_IMU_SELF_TEST_X_GYRO 0x00                  
#define MPU9250_IMU_SELF_TEST_Y_GYRO 0x01                                                                          
#define MPU9250_IMU_SELF_TEST_Z_GYRO 0x02

#define MPU9250_IMU_SELF_TEST_X_ACCEL 0x0D
#define MPU9250_IMU_SELF_TEST_Y_ACCEL 0x0E    
#define MPU9250_IMU_SELF_TEST_Z_ACCEL 0x0F

#define MPU9250_IMU_SELF_TEST_A      0x10

#define MPU9250_IMU_XG_OFFSET_H      0x13
#define MPU9250_IMU_XG_OFFSET_L      0x14
#define MPU9250_IMU_YG_OFFSET_H      0x15
#define MPU9250_IMU_YG_OFFSET_L      0x16
#define MPU9250_IMU_ZG_OFFSET_H      0x17
#define MPU9250_IMU_ZG_OFFSET_L      0x18
#define MPU9250_IMU_SMPLRT_DIV       0x19
#define MPU9250_IMU_CONFIG           0x1A
#define MPU9250_IMU_GYRO_CONFIG      0x1B
#define MPU9250_IMU_ACCEL_CONFIG     0x1C
#define MPU9250_IMU_ACCEL_CONFIG2    0x1D
#define MPU9250_IMU_LP_ACCEL_ODR     0x1E   
#define MPU9250_IMU_WOM_THR          0x1F   

#define MPU9250_IMU_MOT_DUR          0x20
#define MPU9250_IMU_ZMOT_THR         0x21
#define MPU9250_IMU_ZRMOT_DUR        0x22

#define MPU9250_IMU_FIFO_EN          0x23
#define MPU9250_IMU_I2C_MST_CTRL     0x24   
#define MPU9250_IMU_I2C_SLV0_ADDR    0x25
#define MPU9250_IMU_I2C_SLV0_REG     0x26
#define MPU9250_IMU_I2C_SLV0_CTRL    0x27
#define MPU9250_IMU_I2C_SLV1_ADDR    0x28
#define MPU9250_IMU_I2C_SLV1_REG     0x29
#define MPU9250_IMU_I2C_SLV1_CTRL    0x2A
#define MPU9250_IMU_I2C_SLV2_ADDR    0x2B
#define MPU9250_IMU_I2C_SLV2_REG     0x2C
#define MPU9250_IMU_I2C_SLV2_CTRL    0x2D
#define MPU9250_IMU_I2C_SLV3_ADDR    0x2E
#define MPU9250_IMU_I2C_SLV3_REG     0x2F
#define MPU9250_IMU_I2C_SLV3_CTRL    0x30
#define MPU9250_IMU_I2C_SLV4_ADDR    0x31
#define MPU9250_IMU_I2C_SLV4_REG     0x32
#define MPU9250_IMU_I2C_SLV4_DO      0x33
#define MPU9250_IMU_I2C_SLV4_CTRL    0x34
#define MPU9250_IMU_I2C_SLV4_DI      0x35
#define MPU9250_IMU_I2C_MST_STATUS   0x36
#define MPU9250_IMU_INT_PIN_CFG      0x37
#define MPU9250_IMU_INT_ENABLE       0x38
#define MPU9250_IMU_DMP_INT_STATUS   0x39
#define MPU9250_IMU_INT_STATUS       0x3A
#define MPU9250_IMU_ACCEL_XOUT_H     0x3B
#define MPU9250_IMU_ACCEL_XOUT_L     0x3C
#define MPU9250_IMU_ACCEL_YOUT_H     0x3D
#define MPU9250_IMU_ACCEL_YOUT_L     0x3E
#define MPU9250_IMU_ACCEL_ZOUT_H     0x3F
#define MPU9250_IMU_ACCEL_ZOUT_L     0x40
#define MPU9250_IMU_TEMP_OUT_H       0x41
#define MPU9250_IMU_TEMP_OUT_L       0x42
#define MPU9250_IMU_GYRO_XOUT_H      0x43
#define MPU9250_IMU_GYRO_XOUT_L      0x44
#define MPU9250_IMU_GYRO_YOUT_H      0x45
#define MPU9250_IMU_GYRO_YOUT_L      0x46
#define MPU9250_IMU_GYRO_ZOUT_H      0x47
#define MPU9250_IMU_GYRO_ZOUT_L      0x48
#define MPU9250_IMU_EXT_SENS_DATA_00 0x49
#define MPU9250_IMU_EXT_SENS_DATA_01 0x4A
#define MPU9250_IMU_EXT_SENS_DATA_02 0x4B
#define MPU9250_IMU_EXT_SENS_DATA_03 0x4C
#define MPU9250_IMU_EXT_SENS_DATA_04 0x4D
#define MPU9250_IMU_EXT_SENS_DATA_05 0x4E
#define MPU9250_IMU_EXT_SENS_DATA_06 0x4F
#define MPU9250_IMU_EXT_SENS_DATA_07 0x50
#define MPU9250_IMU_EXT_SENS_DATA_08 0x51
#define MPU9250_IMU_EXT_SENS_DATA_09 0x52
#define MPU9250_IMU_EXT_SENS_DATA_10 0x53
#define MPU9250_IMU_EXT_SENS_DATA_11 0x54
#define MPU9250_IMU_EXT_SENS_DATA_12 0x55
#define MPU9250_IMU_EXT_SENS_DATA_13 0x56
#define MPU9250_IMU_EXT_SENS_DATA_14 0x57
#define MPU9250_IMU_EXT_SENS_DATA_15 0x58
#define MPU9250_IMU_EXT_SENS_DATA_16 0x59
#define MPU9250_IMU_EXT_SENS_DATA_17 0x5A
#define MPU9250_IMU_EXT_SENS_DATA_18 0x5B
#define MPU9250_IMU_EXT_SENS_DATA_19 0x5C
#define MPU9250_IMU_EXT_SENS_DATA_20 0x5D
#define MPU9250_IMU_EXT_SENS_DATA_21 0x5E
#define MPU9250_IMU_EXT_SENS_DATA_22 0x5F
#define MPU9250_IMU_EXT_SENS_DATA_23 0x60
#define MPU9250_IMU_MOT_DETECT_STATUS 0x61
#define MPU9250_IMU_I2C_SLV0_DO      0x63
#define MPU9250_IMU_I2C_SLV1_DO      0x64
#define MPU9250_IMU_I2C_SLV2_DO      0x65
#define MPU9250_IMU_I2C_SLV3_DO      0x66
#define MPU9250_IMU_I2C_MST_DELAY_CTRL 0x67
#define MPU9250_IMU_SIGNAL_PATH_RESET  0x68
#define MPU9250_IMU_MOT_DETECT_CTRL  0x69
#define MPU9250_IMU_USER_CTRL        0x6A
#define MPU9250_IMU_PWR_MGMT_1       0x6B
#define MPU9250_IMU_PWR_MGMT_2       0x6C
#define MPU9250_IMU_DMP_BANK         0x6D
#define MPU9250_IMU_DMP_RW_PNT       0x6E
#define MPU9250_IMU_DMP_REG          0x6F
#define MPU9250_IMU_DMP_REG_1        0x70
#define MPU9250_IMU_DMP_REG_2        0x71 
#define MPU9250_IMU_FIFO_COUNTH      0x72
#define MPU9250_IMU_FIFO_COUNTL      0x73
#define MPU9250_IMU_FIFO_R_W         0x74
#define MPU9250_IMU_WHO_AM_I         0x75
#define MPU9250_IMU_XA_OFFSET_H      0x77
#define MPU9250_IMU_XA_OFFSET_L      0x78
#define MPU9250_IMU_YA_OFFSET_H      0x7A
#define MPU9250_IMU_YA_OFFSET_L      0x7B
#define MPU9250_IMU_ZA_OFFSET_H      0x7D
#define MPU9250_IMU_ZA_OFFSET_L      0x7E

// We only support a single MPU, so we will use a single
// static allocation to store it.
static mpu_t _static_mpu = {};

static uint8_t mpu9250_imu_read(mpu_t *mpu, const uint8_t addr)
{
    uint8_t response = 0;
    twi_ret_t status;

    status = twi_master_addr_read(mpu->twi, mpu->imu_addr, addr, 1, &response, 1);
    if (status != 1)
        return 0x0;

    return response;
}

static bool mpu9250_imu_write(mpu_t *mpu, const uint8_t addr,
                                 const uint8_t value)
{
    twi_ret_t status = twi_master_addr_write(mpu->twi, mpu->imu_addr, addr, 1, &value, 1);
    return (status == 1);
}

static uint8_t mpu9250_mag_read(mpu_t *mpu, const uint8_t addr)
{
    uint8_t response = 0;
    twi_ret_t status;

    status = twi_master_addr_read(mpu->twi, MPU9250_MAG_SLAVE_ADDR, addr, 1, &response, 1);
    if (status != 1)
        return 0x0;

    return response;
}

static bool mpu9250_mag_write(mpu_t *mpu, const uint8_t addr, const uint8_t value)
{
    uint8_t response = 0;
    twi_ret_t status;

    status = twi_master_addr_write(mpu->twi, MPU9250_MAG_SLAVE_ADDR, addr, 1, &value, 1);
    return status == 1;
}

bool mpu9250_is_imu_ready(mpu_t *mpu)
{
    return (mpu9250_imu_read(mpu, MPU9250_IMU_INT_STATUS) & 0x01) != 0;
}

static bool mpu9250_read_imu_data(mpu_t *mpu, const uint8_t addr, int16_t data[3])
{
    uint8_t rawdata[6];
    twi_ret_t status;

    status = twi_master_addr_read(mpu->twi, mpu->imu_addr, addr, 1, rawdata, 6);
    if (status != 6) {
        data[0] = data[1] = data[2] = 0;
        return false;
    }

    data[0] = ((int16_t)rawdata[0] << 8) | rawdata[1];
    data[1] = ((int16_t)rawdata[2] << 8) | rawdata[3];  
    data[2] = ((int16_t)rawdata[4] << 8) | rawdata[5]; 
    
    return true;
}

bool mpu9250_read_accel(mpu_t *mpu, int16_t acceldata[3])
{
    return mpu9250_read_imu_data(mpu, MPU9250_IMU_ACCEL_XOUT_H, acceldata);
}


bool mpu9250_read_gyro(mpu_t *mpu, int16_t gyrodata[3])
{
    return mpu9250_read_imu_data(mpu, MPU9250_IMU_GYRO_XOUT_H, gyrodata);
}

bool mpu9250_is_mag_ready(mpu_t *mpu)
{
    return (mpu9250_mag_read(mpu, MPU9250_MAG_ST1) & 0x1) != 0;
}

bool mpu9250_read_mag(mpu_t *mpu, int16_t magdata[3])
{
    uint8_t rawdata[7];
    twi_ret_t status;

    status = twi_master_addr_read(mpu->twi, MPU9250_MAG_SLAVE_ADDR, MPU9250_MAG_HXL, 1, rawdata, 7);
    if (status != 7 || (rawdata[6] & (1 << 3))) {
        magdata[0] = magdata[1] = magdata[2] = 0;
        return false;
    }

    /* Note, magnetometer registers are LS byte first.  */
    magdata[0] = ((int16_t)rawdata[1] << 8) | rawdata[0];
    magdata[1] = ((int16_t)rawdata[3] << 8) | rawdata[2];  
    magdata[2] = ((int16_t)rawdata[5] << 8) | rawdata[4]; 

    return true;
}

static bool mpu9250_init_imu(mpu_t *mpu)
{
    uint8_t response;

    /* Make sure we can see the IMU ID */
    response = mpu9250_imu_read(mpu, MPU9250_IMU_WHO_AM_I);
    if (response != 0x71)
        return false;

    mpu9250_imu_write(mpu, MPU9250_IMU_PWR_MGMT_1, 0x00);
    DELAY_US (100);

    /* Choose best clock source.  */
    mpu9250_imu_write(mpu, MPU9250_IMU_PWR_MGMT_1, 0x01);
    DELAY_US (200);

    mpu9250_imu_write(mpu, MPU9250_IMU_CONFIG, 0x03);

    mpu9250_imu_write(mpu, MPU9250_IMU_SMPLRT_DIV, 0x04);

    /* Enable I2C bypass so commands can be sent to magnetometer.  */
    mpu9250_imu_write(mpu, MPU9250_IMU_INT_PIN_CFG, 0x02);    

    return true;
}

static bool mpu9250_init_mag(mpu_t *mpu)
{
    uint8_t response;

    /* This should read 0x48.  */
    response = mpu9250_mag_read(mpu, MPU9250_MAG_DEVICE_ID);
    if (response != 0x48)
        return false;

    /* Reset the device */
    if (!mpu9250_mag_write(mpu, MPU9250_MAG_CNTL2, 0x1))
        return false;

    /* Set to continuous measurement 2 mode and 16-bit output */
    if (!mpu9250_mag_write(mpu, MPU9250_MAG_CNTL1, (1 << 4) | (6 << 0)))
        return false;

    return true;
}

mpu_t *mpu9250_create(twi_t twi, twi_slave_addr_t imu_addr)
{
    mpu_t *mpu = &_static_mpu;
    mpu->imu_addr = imu_addr;
    mpu->twi = twi;

    if (!mpu9250_init_imu(mpu))
        return NULL;

    if (!mpu9250_init_mag(mpu))
        return NULL;

    return mpu;
}
