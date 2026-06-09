#include "dfr0503.h"
#include "../config.h"
#include <stdlib.h>
#include <string.h>

/* ESC/POS command bytes */
#define ESC  0x1B
#define GS   0x1D
#define DC2  0x12
#define LF   0x0A

struct DFR0503 {
    uart_write_fn write;
    void *ctx;
};

static void send(DFR0503 *p, const uint8_t *buf, size_t len)
{
    p->write(buf, len, p->ctx);
}

static void send1(DFR0503 *p, uint8_t b)
{
    send(p, &b, 1);
}

DFR0503 *dfr0503_init(uart_write_fn write, void *ctx)
{
    DFR0503 *p = malloc(sizeof(DFR0503));
    if (!p) return NULL;
    p->write = write;
    p->ctx   = ctx;
    dfr0503_reset(p);
    return p;
}

void dfr0503_reset(DFR0503 *p)
{
    uint8_t cmd[] = { ESC, '@' };
    send(p, cmd, sizeof(cmd));
}

void dfr0503_feed(DFR0503 *p, uint8_t lines)
{
    uint8_t cmd[] = { ESC, 'd', lines };
    send(p, cmd, sizeof(cmd));
}

void dfr0503_print_text(DFR0503 *p, const char *text)
{
    /* UARTE DMA reads from RAM only, so copy the string off flash first. */
    size_t len = strlen(text);
    uint8_t ram[128];
    if (len > sizeof(ram)) len = sizeof(ram);
    memcpy(ram, text, len);
    send(p, ram, len);
    send1(p, LF);
}

void dfr0503_print_bitmap(DFR0503 *p, const uint8_t *bitmap, uint16_t rows)
{
    uint8_t hdr[] = {
        DC2, '*',
        (uint8_t)(rows & 0xFF),
        PRINTER_BYTES_PER_ROW
    };
    send(p, hdr, sizeof(hdr));
    send(p, bitmap, (size_t)rows * PRINTER_BYTES_PER_ROW);
}

void dfr0503_cut(DFR0503 *p)
{
    uint8_t cmd[] = { GS, 'V', 0x42, 0x00 };
    send(p, cmd, sizeof(cmd));
}
