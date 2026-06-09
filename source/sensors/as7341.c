#include "as7341.h"
#include "nrf_delay.h"
#include <stdio.h>



#define AS7341_ENABLE   0x80
#define AS7341_ATIME    0x81
#define AS7341_CFG0     0xA9
#define AS7341_CFG1     0xAA
#define AS7341_CFG6     0xAF
#define AS7341_STATUS2  0xA3
#define AS7341_ASTEP_L  0xCA
#define AS7341_ASTEP_H  0xCB
#define AS7341_CLR_L    0x9D
#define AS7341_CLR_H    0x9E
#define AS7341_WHOAMI   0x92

#define AS7341_ENABLE_PON    0x01
#define AS7341_ENABLE_SP_EN  0x02
#define AS7341_ENABLE_SMUXEN 0x10

static const nrf_twi_mngr_t *i2c_manager = NULL;

static void as7341_write_reg(uint8_t reg, uint8_t val) {
  uint8_t buf[2] = {reg, val};
  nrf_twi_mngr_transfer_t const xfer[] = {
    NRF_TWI_MNGR_WRITE(AS7341_ADDR, buf, 2, 0),
  };
  ret_code_t r = nrf_twi_mngr_perform(i2c_manager, NULL, xfer, 1, NULL);
  if (r != NRF_SUCCESS) printf("AS7341 write error: %lX\n", r);
}

static uint8_t as7341_reg_read(uint8_t reg) {
  uint8_t val = 0;
  nrf_twi_mngr_transfer_t const xfer[] = {
    NRF_TWI_MNGR_WRITE(AS7341_ADDR, &reg, 1, NRF_TWI_MNGR_NO_STOP),
    NRF_TWI_MNGR_READ(AS7341_ADDR, &val, 1, 0),
  };
  ret_code_t r = nrf_twi_mngr_perform(i2c_manager, NULL, xfer, 2, NULL);
  if (r != NRF_SUCCESS) printf("AS7341 read error: %lX\n", r);
  return val;
}


static void smux_config(void) {
  as7341_write_reg(AS7341_CFG6, 0x10);  // bits[4:3]=0b10: load SMUX from I2C

  static uint8_t smux[][2] = {
    {0x00,0x30},{0x01,0x01},{0x02,0x00},{0x03,0x00},
    {0x04,0x00},{0x05,0x42},{0x06,0x00},{0x07,0x00},
    {0x08,0x50},{0x09,0x00},{0x0A,0x00},{0x0B,0x00},
    {0x0C,0x20},{0x0D,0x04},{0x0E,0x00},{0x0F,0x30},
    {0x10,0x01},{0x11,0x50},{0x12,0x00},{0x13,0x06},
  };

  for (int i = 0; i < 20; i++) {
    as7341_write_reg(smux[i][0], smux[i][1]);
  }

  as7341_write_reg(AS7341_ENABLE, AS7341_ENABLE_PON | AS7341_ENABLE_SMUXEN);
  nrf_delay_ms(1);

  uint32_t timeout = 100;
  while ((as7341_reg_read(AS7341_ENABLE) & AS7341_ENABLE_SMUXEN) && timeout--) {
    nrf_delay_ms(1);
  }

  if (timeout == 0) {
    printf("AS7341 SMUX timeout\n");
  }
}

void as7341_init(const nrf_twi_mngr_t *i2c) {
  i2c_manager = i2c;

  as7341_write_reg(AS7341_ENABLE, AS7341_ENABLE_PON);
  nrf_delay_ms(10);

  as7341_write_reg(AS7341_CFG0, 0x00);  // normal mode, high register bank

  uint8_t whoami = as7341_reg_read(AS7341_WHOAMI);
  if ((whoami & 0xFC) != 0x24) {
    printf("AS7341 not found (id 0x%02X)\n", whoami & 0xFC);
  }

  as7341_write_reg(AS7341_ATIME, 29);
  as7341_write_reg(AS7341_ASTEP_L, 0x57);
  as7341_write_reg(AS7341_ASTEP_H, 0x02);

  as7341_write_reg(AS7341_CFG1, 0x04);  // ADC gain

  smux_config();
}

static ret_code_t as7341_reg_read2(uint8_t reg, uint8_t *p_val) {
  if (p_val == NULL) return NRF_ERROR_NULL;

  *p_val = 0;
  nrf_twi_mngr_transfer_t const xfer[] = {
    NRF_TWI_MNGR_WRITE(AS7341_ADDR, &reg, 1, NRF_TWI_MNGR_NO_STOP),
    NRF_TWI_MNGR_READ(AS7341_ADDR, p_val, 1, 0),
  };

  return nrf_twi_mngr_perform(i2c_manager, NULL, xfer, 2, NULL);
}

static ret_code_t as7341_read_clear_raw(uint16_t *p_raw) {
  if (p_raw == NULL) return NRF_ERROR_NULL;

  ret_code_t err;
  uint8_t lo, hi;

  err = as7341_reg_read2(AS7341_CLR_L, &lo);
  if (err != NRF_SUCCESS) return err;

  err = as7341_reg_read2(AS7341_CLR_H, &hi);
  if (err != NRF_SUCCESS) return err;

  *p_raw = ((uint16_t)hi << 8) | lo;
  return NRF_SUCCESS;
}

ret_code_t as7341_read_light(uint8_t *p_light) {
  if (p_light == NULL) return NRF_ERROR_NULL;
  if (i2c_manager == NULL) return NRF_ERROR_INVALID_STATE;

  // AS7341 is one-shot: cycle SP_EN to trigger a fresh integration
  as7341_write_reg(AS7341_ENABLE, AS7341_ENABLE_PON);
  nrf_delay_ms(1);
  as7341_write_reg(AS7341_ENABLE, AS7341_ENABLE_PON | AS7341_ENABLE_SP_EN);

  ret_code_t err;
  uint8_t status = 0;
  uint32_t timeout = 200;

  while (timeout--) {
    err = as7341_reg_read2(AS7341_STATUS2, &status);
    if (err != NRF_SUCCESS) return err;

    if (status & 0x40) break;

    nrf_delay_ms(1);
  }

  if (!(status & 0x40)) {
    return NRF_ERROR_TIMEOUT;
  }

  /* Sum the six ADC channels for a broadband light level. */
  static const uint8_t ch_base[] = {0x95, 0x97, 0x99, 0x9B, 0x9D, 0x9F};
  uint32_t sum = 0;
  for (int i = 0; i < 6; i++) {
    uint8_t lo = 0, hi = 0;
    as7341_reg_read2(ch_base[i],   &lo);
    as7341_reg_read2(ch_base[i]+1, &hi);
    sum += ((uint16_t)hi << 8) | lo;
  }

  uint32_t scaled = sum / 50u;
  if (scaled > 255u) scaled = 255u;

  *p_light = (uint8_t)scaled;
  return NRF_SUCCESS;
}