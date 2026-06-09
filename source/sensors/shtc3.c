#include "shtc3.h"
#include "nrf_delay.h"
#include <stdio.h>

static const nrf_twi_mngr_t *i2c_manager = NULL;

void shtc3_init(const nrf_twi_mngr_t *i2c) {
  i2c_manager = i2c;
  nrf_delay_ms(40); /* wait for sensor power-up */
}

Shtc3Reading shtc3_read(void) {
  Shtc3Reading result = {-99, -99};

  /* Wakeup */
  uint8_t wakeup[2] = {0x35, 0x17};
  nrf_twi_mngr_transfer_t const wake_xfer[] = {
    NRF_TWI_MNGR_WRITE(SHTC3_ADDR, wakeup, 2, 0),
  };
  ret_code_t r = nrf_twi_mngr_perform(i2c_manager, NULL, wake_xfer, 1, NULL);
  if (r != NRF_SUCCESS) {
    printf("SHTC3 wakeup error: %lu\n", r);
    return result;
  }
  nrf_delay_ms(1);

  /* Measure: normal power, T first, no clock stretch */
  uint8_t measure[2] = {0x7C, 0xA2};
  nrf_twi_mngr_transfer_t const meas_xfer[] = {
    NRF_TWI_MNGR_WRITE(SHTC3_ADDR, measure, 2, 0),
  };
  r = nrf_twi_mngr_perform(i2c_manager, NULL, meas_xfer, 1, NULL);
  if (r != NRF_SUCCESS) {
    printf("SHTC3 measure error: %lu\n", r);
    return result;
  }
  nrf_delay_ms(15);

  /* Read 6 bytes: T_MSB T_LSB T_CRC RH_MSB RH_LSB RH_CRC */
  uint8_t data[6] = {0};
  nrf_twi_mngr_transfer_t const read_xfer[] = {
    NRF_TWI_MNGR_READ(SHTC3_ADDR, data, 6, 0),
  };
  r = nrf_twi_mngr_perform(i2c_manager, NULL, read_xfer, 1, NULL);
  if (r != NRF_SUCCESS) {
    printf("SHTC3 read error: %lu\n", r);
    return result;
  }

  /* Sleep */
  uint8_t sleep_cmd[2] = {0xB0, 0x98};
  nrf_twi_mngr_transfer_t const sleep_xfer[] = {
    NRF_TWI_MNGR_WRITE(SHTC3_ADDR, sleep_cmd, 2, 0),
  };
  nrf_twi_mngr_perform(i2c_manager, NULL, sleep_xfer, 1, NULL);

  uint16_t raw_t  = ((uint16_t)data[0] << 8) | data[1];
  uint16_t raw_rh = ((uint16_t)data[3] << 8) | data[4];

  result.temperature_c = -45.0f + 175.0f * ((float)raw_t  / 65535.0f);
  result.humidity_pct  = 100.0f *           ((float)raw_rh / 65535.0f);
  return result;
}
