#include "comms.h"
#include "nrf24.h"

#define ADDRESS 0x0101010203
#define PAYLOAD_LENGTH 32
// random offset to make channel higher (should be less than 110)
#define CHANNEL_OFFSET 100

static nrf24_t *nrf_p;

void comms_initialise(void)
{
    uint8_t count = 0;
    spi_cfg_t nrf_spi = {
        .channel = 0,
        .clock_speed_kHz = 1000,
        .cs = RADIO_CS_PIO,
        .mode = SPI_MODE_0,
        .cs_mode = SPI_CS_MODE_FRAME,
        .bits = 8,
    };
    spi_t spi;

    spi = spi_init(&nrf_spi);
    nrf_p = nrf24_create(spi, RADIO_CE_PIO, RADIO_IRQ_PIO);

    if (!nrf_p)
    {
        core_printf("NRF could not be created");
        core_panic();
    }

    int channel = (core_read_dip_value() % 8) + CHANNEL_OFFSET;
    if (!nrf24_begin(nrf_p, channel, ADDRESS, PAYLOAD_LENGTH))
    {
        core_printf("NRF could not begin");
        core_panic();
    }

    #ifdef BOARD_RACER
        nrf24_listen(nrf_p);
    #endif
}

void comms_shutdown(void)
{
    nrf24_power_down(nrf_p);
}

static payload_h2r_t decode_payload_h2r(uint8_t buffer[])
{
    return (payload_h2r_t){
        .motor_left = buffer[0],
        .motor_right = buffer[1],
        .should_deploy_string = (bool)buffer[2],
    };
}

static void encode_payload_h2r(payload_h2r_t payload, uint8_t *buffer)
{
    buffer[0] = payload.motor_left;
    buffer[1] = payload.motor_right;
    buffer[2] = payload.should_deploy_string;
}

static bool send(uint8_t *buffer)
{
    return nrf24_write(nrf_p, buffer, sizeof(buffer)) > 0;
}

bool comms_send_h2r_payload(payload_h2r_t payload)
{
    char buffer[PAYLOAD_LENGTH] = {0};
    encode_payload_h2r(payload, buffer);
    if (nrf24_write(nrf_p, buffer, sizeof(buffer)))
    {
        return true;
    }
    return false;
}

bool comms_receive_h2r_payload(payload_h2r_t *payload_h2r_p)
{
    char buffer[PAYLOAD_LENGTH] = {0};

    if (nrf24_read(nrf_p, buffer, sizeof(buffer)))
    {
        *payload_h2r_p = decode_payload_h2r(buffer);
        return true;
    }
    return false;
}