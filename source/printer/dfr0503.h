#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct DFR0503 DFR0503;

typedef void (*uart_write_fn)(const uint8_t *buf, size_t len, void *ctx);

DFR0503 *dfr0503_init(uart_write_fn write, void *ctx);
void     dfr0503_reset(DFR0503 *p);
void     dfr0503_feed(DFR0503 *p, uint8_t lines);
void     dfr0503_print_text(DFR0503 *p, const char *text);
void     dfr0503_print_bitmap(DFR0503 *p, const uint8_t *bitmap, uint16_t rows);
void     dfr0503_cut(DFR0503 *p);
