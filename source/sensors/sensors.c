#include "sensors.h"
#include "shtc3.h"
#include "as7341.h"
#include "nrf_twi_mngr.h"
#include "nrf_delay.h"
#include <stdio.h>

NRF_TWI_MNGR_DEF(twi_mngr, 4, 0);

void sensors_init(void) {
  nrf_drv_twi_config_t cfg = NRF_DRV_TWI_DEFAULT_CONFIG;
  cfg.scl = 26;  /* P0.26 QWIIC SCL */
  cfg.sda = 32;  /* P1.00 QWIIC SDA */

  ret_code_t r = nrf_twi_mngr_init(&twi_mngr, &cfg);
  printf("TWI init: %lu\n", r);

  printf("I2C scan:\n");
  uint8_t dummy = 0;
  int found = 0;
  for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
    nrf_twi_mngr_transfer_t const xfer[] = {
      NRF_TWI_MNGR_READ(addr, &dummy, 1, 0),
    };
    if (nrf_twi_mngr_perform(&twi_mngr, NULL, xfer, 1, NULL) == NRF_SUCCESS) {
      printf("  0x%02X\n", addr);
      found++;
    }
  }
  if (found == 0) printf("  (nothing found, check cable/power)\n");

  shtc3_init(&twi_mngr);
  as7341_init(&twi_mngr);
}

SensorReading sensors_read(void) {
  Shtc3Reading env = shtc3_read();
  uint8_t light = 0;
  as7341_read_light(&light);
  return (SensorReading){
    .temperature_c = env.temperature_c,
    .humidity_pct  = env.humidity_pct,
    .light_level   = light,
  };
}
