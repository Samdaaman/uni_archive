#include "imu.h"
#include "mpu9250.h"
#include "mcu.h"
#include "core.h"
#include "delay.h"

static mpu_t* mpu_p;
static twi_cfg_t mpu_twi_cfg =
{
    .channel = TWI_CHANNEL_1,
    .period = TWI_PERIOD_DIVISOR(100000), // 100 kHz
    .slave_addr = 0
};

void imu_initialise(void)
{
    mcu_jtag_disable();

    delay_ms(100);

    twi_t twi_mpu = twi_init (&mpu_twi_cfg);
    // Initialise the MPU9250 IMU
    mpu_p = mpu9250_create (twi_mpu, MPU_ADDRESS);
}

payload_h2r_t imu_read(void)
{
    int16_t accel[3];
    if (mpu9250_read_accel (mpu_p, accel))
    {
        int16_t left_scaled = -(accel[0] * 3 / (164 * 3)) + accel[1] * 1 / (164 * 5);
        int16_t right_scaled = -(accel[0] * 3 / (164 * 3)) - accel[1] * 1 / (164 * 5);

        return (payload_h2r_t){
            .motor_left = (100-limit_int(0, 100, left_scaled + 50)),
            .motor_right = (100-limit_int(0, 100, right_scaled + 50)),
            .should_deploy_string = false,
        };
    }
    else
    {
        core_panic();
    }
}