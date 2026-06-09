#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define NRFX_UARTE_ENABLED 1
#define NRFX_UARTE0_ENABLED 1

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrfx.h"
#include "nrfx_uarte.h"
#include "nrf_uarte.h"
#include "config.h"

/* Button-triggered printer test.
   Same working print path as test_breadboard.c (TX P0.10 / P8 @ 115200),
   but printing fires on each press of button A instead of once at boot. */

static nrfx_uarte_t m_uart = NRFX_UARTE_INSTANCE(1);

/* Button A: P0.14, active low with internal pull-up */
#define BUTTON_A NRF_GPIO_PIN_MAP(0, 14)

static void uart_event_handler(nrfx_uarte_event_t const *p_event, void *p_context) {
    (void)p_event;
    (void)p_context;
}

static void uart_init(void) {
    nrfx_uarte_config_t config;
    config.pseltxd            = NRF_GPIO_PIN_MAP(0, 10);  // Printer RX -> P8
    config.pselrxd            = NRF_GPIO_PIN_MAP(0, 9);   // Printer TX -> P9
    config.baudrate           = NRF_UARTE_BAUDRATE_115200;
    config.parity             = NRF_UARTE_PARITY_EXCLUDED;
    config.hwfc               = NRF_UARTE_HWFC_DISABLED;
    config.interrupt_priority = NRFX_UARTE_DEFAULT_CONFIG_IRQ_PRIORITY;

    nrfx_err_t err = nrfx_uarte_init(&m_uart, &config, uart_event_handler);
    printf("UART init: %ld (0=OK)  TX=P0.10(P8)  RX=P0.09(P9)\n", err);
    APP_ERROR_CHECK(err);
}

static int printer_send(uint8_t *bytes, int count) {
    return nrfx_uarte_tx(&m_uart, bytes, count);
}

/* --- Bitmap: 48px wide x 32px tall checkerboard --- */
#define IMG_WIDTH_PX      48
#define IMG_HEIGHT_PX     32
#define IMG_BYTES_PER_ROW (IMG_WIDTH_PX / 8)  // = 6

static uint8_t image_data[IMG_HEIGHT_PX * IMG_BYTES_PER_ROW] = {
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
};

static void printer_print_bitmap(void) {
    const uint16_t xbytes = IMG_BYTES_PER_ROW;
    const uint16_t yrows  = IMG_HEIGHT_PX;

    static uint8_t header[8] = {
        0x1D, 0x76, 0x30,            // GS v 0
        0x00,                         // m = normal density
        xbytes & 0xFF, xbytes >> 8,  // xL, xH
        yrows  & 0xFF, yrows  >> 8,  // yL, yH
    };

    nrfx_uarte_tx(&m_uart, header, sizeof(header));
    nrf_delay_ms(10);
    nrfx_uarte_tx(&m_uart, image_data, sizeof(image_data));

    static uint8_t feed[] = { 0x1B, 0x64, 0x03 }; // ESC d 3, feed 3 lines
    nrf_delay_ms(100);
    nrfx_uarte_tx(&m_uart, feed, sizeof(feed));
}

static uint8_t receipt_mode_cmd[] = { 0x1F, 0x2F, 0x0B, 0x00, 0x01, 0x00, 0x00 };
static uint8_t test_line[]        = "Hello from micro:bit\r\n";

static void do_print(void) {
    printer_send(receipt_mode_cmd, sizeof(receipt_mode_cmd));
    nrf_delay_ms(500);
    printer_send(test_line, sizeof(test_line) - 1);
    nrf_delay_ms(500);
    printer_print_bitmap();
}

static bool button_a_pressed(void) {
    if (nrf_gpio_pin_read(BUTTON_A) == 0) {
        nrf_delay_ms(20); /* debounce */
        if (nrf_gpio_pin_read(BUTTON_A) == 0) {
            while (nrf_gpio_pin_read(BUTTON_A) == 0) {} /* wait for release */
            return true;
        }
    }
    return false;
}

int main(void) {
    uart_init();
    nrf_gpio_cfg_input(BUTTON_A, NRF_GPIO_PIN_PULLUP);

    nrf_delay_ms(500); /* let the printer come up */
    printf("Ready. Press A to print.\n");

    while (1) {
        if (button_a_pressed()) {
            printf("Button A pressed, printing...\n");
            do_print();
            printf("Print done.\n");
        }
        nrf_delay_ms(20);
    }
}
