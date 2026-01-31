#ifndef PINS_H
#define PINS_H

#include "pico/stdlib.h"

// PINS for the LCD
#define PIN_CS_LCD 17
#define PIN_SCK_LCD 18
#define PIN_MOSI_LCD 19
#define PIN_DC_LCD 21
#define PIN_RST_LCD 20
#define SPI_PORT_LCD spi0

// PINS for the touch screen
#define PIN_IRQ_TOUCH 2
#define PIN_MISO_TOUCH 4
#define PIN_CS_TOUCH 5
#define PIN_CLK_TOUCH 6
#define PIN_MOSI_TOUCH 7

#endif // PINS_H