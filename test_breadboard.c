#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrfx_uarte.h"
#include "config.h"
#include "printer/dfr0503.h"



#define NRFX_UARTE_ENABLED 1
#define NRFX_UARTE0_ENABLED 1


#include "nrfx.h"
#include "nrfx_uarte.h"
#include "nrf_uarte.h"
#include "nrf_gpio.h"


// Declare the UARTE instance
static nrfx_uarte_t m_uart = NRFX_UARTE_INSTANCE(1);

// Event handler (called from interrupt context)
static void uart_event_handler(nrfx_uarte_event_t const *p_event, void *p_context)
{
    switch (p_event->type)
    {
        case NRFX_UARTE_EVT_TX_DONE:
            // TX complete
            break;
        case NRFX_UARTE_EVT_RX_DONE:
            // RX complete, p_event->data.rxtx.bytes bytes received
            break;
        case NRFX_UARTE_EVT_ERROR:
            // Handle error
            break;
    }
}

void uart_init(void)
{
    nrfx_uarte_config_t config; 

    config.pseltxd = NRF_GPIO_PIN_MAP(0, 10);  // Printer RX -> P8 on edge connector
    config.pselrxd = NRF_GPIO_PIN_MAP(0, 9);   // Printer TX -> P9 on edge connector
    // Override any fields you want
    config.baudrate   = NRF_UARTE_BAUDRATE_115200;
    config.parity     = NRF_UARTE_PARITY_EXCLUDED;
    config.hwfc       = NRF_UARTE_HWFC_DISABLED;
    config.interrupt_priority = NRFX_UARTE_DEFAULT_CONFIG_IRQ_PRIORITY;

    nrfx_err_t err = nrfx_uarte_init(&m_uart, &config, uart_event_handler);
    printf("UART init: %ld (0=OK)  TX=P0.10(P8)  RX=P0.09(P9)\n", err);
    APP_ERROR_CHECK(err);
}

int printer_send(uint8_t *bytes, int count) {
    return nrfx_uarte_tx(&m_uart, bytes, count);
}

// Image: 48px wide x 32px tall checkerboard
// Width must be multiple of 8; 48px = 6 bytes per row
#define IMG_WIDTH_PX   48
#define IMG_HEIGHT_PX  32
#define IMG_BYTES_PER_ROW (IMG_WIDTH_PX / 8)  // = 6

// Your 1bpp image data: 1 = black, 0 = white, MSB first
// Replace this with your actual converted image data
static uint8_t image_data[IMG_HEIGHT_PX * IMG_BYTES_PER_ROW] = {
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,  // row 0: alternating bytes
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,  // row 1
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,  // row 2
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,  // row 3
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,  // row 4
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,  // row 5
};

void printer_print_bitmap(void) {
    const uint16_t xbytes = IMG_BYTES_PER_ROW;
    const uint16_t yrows  = IMG_HEIGHT_PX;

    // Build the GS v 0 header: 1D 76 30 m xL xH yL yH
    static uint8_t header[8] = {
        0x1D, 0x76, 0x30,           // GS v 0
        0x00,                        // m = normal density
        xbytes & 0xFF, xbytes >> 8, // xL, xH (width in bytes, little-endian)
        yrows  & 0xFF, yrows  >> 8, // yL, yH (height in rows, little-endian)
    };

    // Send header, then pixel data
    nrfx_uarte_tx(&m_uart, header, sizeof(header));
    nrf_delay_ms(10);
    nrfx_uarte_tx(&m_uart, image_data, sizeof(image_data));

    // Feed paper to clear the image out of the printer
    static uint8_t feed[] = { 0x1B, 0x64, 0x03 }; // ESC d 3, feed 3 lines
    nrf_delay_ms(100);
    nrfx_uarte_tx(&m_uart, feed, sizeof(feed));
}

static uint8_t receipt_mode_cmd[] = { 0x1F, 0x2F, 0x0B, 0x00, 0x01, 0x00, 0x00 };
static uint8_t test_line[] = "Hello from micro:bit\r\n";

int main(void) {

    // nrf_delay_ms(2000);

    // Initialize UART.
    uart_init();

    nrf_delay_ms(500); // give the printer time to switch modes

    printer_send(receipt_mode_cmd, sizeof(receipt_mode_cmd));
    nrf_delay_ms(500); // give the printer time to switch modes
    printer_send(test_line, sizeof(test_line) - 1);
    nrf_delay_ms(500); // give the printer time to switch modes
    printer_print_bitmap();

    while (1) nrf_delay_ms(1000);
}
