#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pins.h"
#include "display.h"

void display_init(void) {
    //Initialize SPI at 62.5 MHz
    spi_init(SPI_PORT_LCD, 62500000);

    //Assign SPI functions to the GPIO pins
    gpio_set_function(PIN_SCK_LCD, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI_LCD, GPIO_FUNC_SPI);

    //Configure control pins (Chip select, data/command, reset)
    gpio_init(PIN_CS_LCD);
    gpio_init(PIN_DC_LCD);
    gpio_init(PIN_RST_LCD);

    gpio_set_dir(PIN_CS_LCD, GPIO_OUT);
    gpio_set_dir(PIN_DC_LCD, GPIO_OUT);
    gpio_set_dir(PIN_RST_LCD, GPIO_OUT);

    //Default CS and DC pins to high
    gpio_put(PIN_CS_LCD, 1);
    gpio_put(PIN_DC_LCD, 1);

    //Hard reset the display
    gpio_put(PIN_RST_LCD, 1);
    sleep_ms(100);
    gpio_put(PIN_RST_LCD, 0);
    sleep_ms(100);
    gpio_put(PIN_RST_LCD, 1);
    sleep_ms(100);
}