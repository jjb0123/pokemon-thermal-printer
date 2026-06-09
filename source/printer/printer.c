#include "printer.h"
#include "../config.h"
#include "font5x7.h"

#include <string.h>
#include <stdio.h>

#define NRFX_UARTE_ENABLED 1
#define NRFX_UARTE0_ENABLED 1

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrfx.h"
#include "nrfx_uarte.h"
#include "nrf_uarte.h"

/* Instance 0 is the console UART, so the printer uses instance 1 */
static nrfx_uarte_t m_uart = NRFX_UARTE_INSTANCE(1);

/* UARTE DMA reads from RAM only, so payloads are staged here in chunks. */
#define TX_CHUNK 128
static uint8_t tx_buf[TX_CHUNK];

static void uart_event_handler(nrfx_uarte_event_t const *p_event, void *p_context) {
    (void)p_event;
    (void)p_context;
}

static void tx_blocking(const uint8_t *data, size_t len) {
    size_t off = 0;
    while (off < len) {
        size_t chunk = (len - off) > TX_CHUNK ? TX_CHUNK : (len - off);
        memcpy(tx_buf, data + off, chunk);
        nrfx_uarte_tx(&m_uart, tx_buf, chunk);
        nrf_delay_ms((uint32_t)(chunk / 10) + 5);
        off += chunk;
    }
}

void printer_init(void) {
    nrfx_uarte_config_t config;
    config.pseltxd            = PRINTER_TX_PIN;
    config.pselrxd            = PRINTER_RX_PIN;
    config.pselcts            = NRF_UARTE_PSEL_DISCONNECTED;
    config.pselrts            = NRF_UARTE_PSEL_DISCONNECTED;
    config.p_context          = NULL;
    config.baudrate           = PRINTER_BAUD;
    config.parity             = NRF_UARTE_PARITY_EXCLUDED;
    config.hwfc               = NRF_UARTE_HWFC_DISABLED;
    config.interrupt_priority = NRFX_UARTE_DEFAULT_CONFIG_IRQ_PRIORITY;

    nrfx_err_t err = nrfx_uarte_init(&m_uart, &config, uart_event_handler);
    printf("printer UART init: %ld\n", err);

    nrf_delay_ms(500);

    static uint8_t receipt_mode_cmd[] = { 0x1F, 0x2F, 0x0B, 0x00, 0x01, 0x00, 0x00 };
    tx_blocking(receipt_mode_cmd, sizeof(receipt_mode_cmd));
    nrf_delay_ms(200);
}

void printer_print_text(const char *text) {
    tx_blocking((const uint8_t *)text, strlen(text));
}

void printer_feed(uint8_t lines) {
    uint8_t feed[] = { 0x1B, 0x64, lines };  /*ESC d n */
    tx_blocking(feed, sizeof(feed));
}

void printer_print_ascii_art(const char *const *lines, uint16_t nlines) {
    if (!lines || nlines == 0) return;

    /* one 8-row raster band per text line, glyphs packed left to right */
    static uint8_t band[PRINTER_BYTES_PER_ROW * FONT5X7_H];

    for (uint16_t li = 0; li < nlines; li++) {
        const char *s = lines[li];
        size_t len = s ? strlen(s) : 0;

        uint16_t width_bytes = (uint16_t)((len * FONT5X7_W + 7) / 8);
        if (width_bytes == 0) width_bytes = 1;
        if (width_bytes > PRINTER_BYTES_PER_ROW) width_bytes = PRINTER_BYTES_PER_ROW;
        uint16_t max_x = (uint16_t)(width_bytes * 8);

        memset(band, 0, (size_t)width_bytes * FONT5X7_H);

        for (size_t ci = 0; ci < len; ci++) {
            unsigned char ch = (unsigned char)s[ci];
            if (ch < FONT5X7_FIRST || ch > 126) ch = '?';
            const uint8_t *g = font5x7[ch - FONT5X7_FIRST];
            for (uint8_t col = 0; col < FONT5X7_W; col++) {
                uint16_t x = (uint16_t)(ci * FONT5X7_W + col);
                if (x >= max_x) break;
                uint8_t gv = g[col];
                for (uint8_t r = 0; r < FONT5X7_H; r++) {
                    if (gv & (1u << r)) {
                        band[r * width_bytes + (x >> 3)] |= (uint8_t)(0x80 >> (x & 7));
                    }
                }
            }
        }

        uint8_t header[8] = {
            0x1D, 0x76, 0x30, 0x00,
            (uint8_t)(width_bytes & 0xFF), (uint8_t)(width_bytes >> 8),
            (uint8_t)(FONT5X7_H & 0xFF),   (uint8_t)(FONT5X7_H >> 8),
        };
        tx_blocking(header, sizeof(header));
        tx_blocking(band, (size_t)width_bytes * FONT5X7_H);
    }
}

/*type-specific border, 30 chars wide */
static const char *border_for_type(PokemonType t) {
    switch (t) {
        case PTYPE_FIRE:     return "(*)(*)(*)(*)(*)(*)(*)(*)(*)(*)";
        case PTYPE_WATER:    return "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
        case PTYPE_GRASS:    return ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.";
        case PTYPE_ICE:      return "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-";
        case PTYPE_GHOST:    return "~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.";
        case PTYPE_ELECTRIC: return "/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\";
        case PTYPE_PSYCHIC:  return "*.*.*.*.*.*.*.*.*.*.*.*.*.*.*.";
        case PTYPE_NORMAL:   default: return "==============================";
    }
}

void printer_print_card(const PokemonCard *card) {
    if (!card) return;
    const char *b = border_for_type(card->type);
    char line[64];

    printer_print_text(b);
    printer_print_text("\n");

    snprintf(line, sizeof line, "%s\n", card->name);
    printer_print_text(line);
    snprintf(line, sizeof line, "Type: %-9s HP: %u\n",
             pokemon_type_name(card->type), card->hp);
    printer_print_text(line);

    printer_print_text(b);
    printer_print_text("\n");

    if (card->art) printer_print_ascii_art(card->art, card->art_rows);

    printer_print_text(b);
    printer_print_text("\n");

    printer_print_text("Moves:\n");
    snprintf(line, sizeof line, " %-14s %3u dmg\n",
             card->move1.name, card->move1.damage);
    printer_print_text(line);
    snprintf(line, sizeof line, " %-14s %3u dmg\n",
             card->move2.name, card->move2.damage);
    printer_print_text(line);

    printer_print_text(b);
    printer_print_text("\n");
    printer_feed(4);
}
