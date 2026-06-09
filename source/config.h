#pragma once

/* UART pins: P8 = P0.10 (TX to printer RX), P9 = P0.09 (RX from printer TX) */
#define PRINTER_TX_PIN      NRF_GPIO_PIN_MAP(0, 10)
#define PRINTER_RX_PIN      NRF_GPIO_PIN_MAP(0,  9)
#define PRINTER_BAUD        NRF_UARTE_BAUDRATE_115200

/* DFR0503 paper width: 384 dots (48mm) */
#define PRINTER_DOT_WIDTH       384
#define PRINTER_BYTES_PER_ROW   (PRINTER_DOT_WIDTH / 8)
