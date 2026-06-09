#pragma once
#include <stdint.h>
#include "nrf_twi_mngr.h"

#define AS7341_ADDR 0x39

void    as7341_init(const nrf_twi_mngr_t *i2c);
ret_code_t as7341_read_light(uint8_t *p_light);