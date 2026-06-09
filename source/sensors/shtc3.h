#pragma once
#include <stdint.h>
#include "nrf_twi_mngr.h"

#define SHTC3_ADDR 0x70

typedef struct {
    float temperature_c;
    float humidity_pct;
} Shtc3Reading;

void       shtc3_init(const nrf_twi_mngr_t *i2c);
Shtc3Reading shtc3_read(void);
