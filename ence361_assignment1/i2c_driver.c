/**********************************************************
 *
 * i2c_driver.c
 *
 * Helper functions used to communicate with peripherals
 * on the Orbit BoosterPack via I2C.
 *
 * Functions excerpted from Digilent's Orbit Booster full
 * demo, available from
 * https://reference.digilentinc.com
 * /orbit_boosterpack/orbit_boosterpack
 *
 * C. P. Moore
 * 11 Feb 2020
 *
 **********************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "i2c_driver.h"
#include "driverlib/i2c.h"
#include "inc/hw_memmap.h"


void Delay_us(void)
{
    int16_t i=0;
    for (i=0;i<1000;i++)
        ;
}

/* ------------------------------------------------------------ */
/***    I2CGenTransmit
**
**  Parameters:
**      pbData  -   Pointer to transmit buffer (read or write)
**      cSize   -   Number of byte transactions to take place
**
**  Return Value:
**      none
**
**  Errors:
**      none
**
**  Description:
**      Transmits data to a device via the I2C bus. Differs from
**      I2C EEPROM Transmit in that the registers in the device it
**      is addressing are addressed with a single byte. Lame, but..
**      it works.
**
*/
char I2CGenTransmit(char * pbData, int32_t cSize, bool fRW, char bAddr) {

    int32_t         i;
    char *      pbTemp;

    pbTemp = pbData;

/*Start*/

/*Send Address High Byte*/
    /* Send Write Block Cmd
    */
    I2CMasterSlaveAddrSet(I2C0_BASE, bAddr, WRITE);
    I2CMasterDataPut(I2C0_BASE, *pbTemp);

    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    //DelayMs(1);
    Delay_us();

    /* Idle wait
    */
    while(I2CGenIsNotIdle());

    /* Increment data pointer
    */
    pbTemp++;

/*Execute Read or Write*/

    if(fRW == READ) {

        /* Resend Start condition
        ** Then send new control byte
        ** then begin reading
        */
        I2CMasterSlaveAddrSet(I2C0_BASE, bAddr, READ);

        while(I2CMasterBusy(I2C0_BASE));

        /* Begin Reading
        */
        for(i = 0; i < cSize; i++) {

            if(cSize == i + 1 && cSize == 1) {
                I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

                //DelayMs(1);
                Delay_us();
                while(I2CMasterBusy(I2C0_BASE));
            }
            else if(cSize == i + 1 && cSize > 1) {
                I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

                //DelayMs(1);
                Delay_us();
                while(I2CMasterBusy(I2C0_BASE));
            }
            else if(i == 0) {
                I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

                //DelayMs(1);
                Delay_us();
                while(I2CMasterBusy(I2C0_BASE));

                /* Idle wait
                */
                while(I2CGenIsNotIdle());
            }
            else {
                I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);

                //DelayMs(1);
                Delay_us();
                while(I2CMasterBusy(I2C0_BASE));

                /* Idle wait
                */
                while(I2CGenIsNotIdle());
            }

            while(I2CMasterBusy(I2C0_BASE));

            /* Read Data
            */
            *pbTemp = (char)I2CMasterDataGet(I2C0_BASE);

            pbTemp++;

        }

    }
    else if(fRW == WRITE) {

        /*Loop data bytes
        */
        for(i = 0; i < cSize; i++) {
            /* Send Data
            */
            I2CMasterDataPut(I2C0_BASE, *pbTemp);

            while(I2CMasterBusy(I2C0_BASE));

            if(i == cSize - 1) {
                I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

                //DelayMs(1);
                Delay_us();
                while(I2CMasterBusy(I2C0_BASE));
            }
            else {
                I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

                //DelayMs(1);
                Delay_us();
                while(I2CMasterBusy(I2C0_BASE));

                /* Idle wait
                */
                while(I2CGenIsNotIdle());
            }

            pbTemp++;
        }

    }

/*Stop*/

    return 0x00;

}

/* ------------------------------------------------------------ */
/***    I2CGenIsNotIdle()
**
**  Parameters:
**      pbData  -   Pointer to transmit buffer (read or write)
**      cSize   -   Number of byte transactions to take place
**
**  Return Value:
**      TRUE is bus is not idle, FALSE if bus is idle
**
**  Errors:
**      none
**
**  Description:
**      Returns TRUE if the bus is not idle
**
*/
bool I2CGenIsNotIdle() {

    return !I2CMasterBusBusy(I2C0_BASE);

}
