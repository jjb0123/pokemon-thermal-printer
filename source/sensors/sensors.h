#pragma once
#include <stdint.h>

typedef struct {
    float   temperature_c;
    float   humidity_pct;
    uint8_t light_level;
} SensorReading;

void          sensors_init(void);
SensorReading sensors_read(void);
