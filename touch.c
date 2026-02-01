#include "pico/stdlib.h"
#include "pins.h"
#include "touch.h"

void touch_init(void) {
    //Configure output pins
    gpio_init(PIN_CS_TOUCH); gpio_set_dir(PIN_CS_TOUCH, GPIO_OUT);
    gpio_init(PIN_CLK_TOUCH); gpio_set_dir(PIN_CLK_TOUCH, GPIO_OUT);
    gpio_init(PIN_MOSI_TOUCH); gpio_set_dir(PIN_MOSI_TOUCH, GPIO_OUT);

    //Configure input pins
    gpio_init(PIN_IRQ_TOUCH); gpio_set_dir(PIN_IRQ_TOUCH, GPIO_IN);
    gpio_init(PIN_MISO_TOUCH); gpio_set_dir(PIN_MISO_TOUCH, GPIO_IN);

    //Enable pull ups
    gpio_pull_up(PIN_IRQ_TOUCH);
    gpio_pull_up(PIN_MISO_TOUCH);

    //Set default pin states
    gpio_put(PIN_CS_TOUCH, 1); //Chip select inactive (high)
    gpio_put(PIN_CLK_TOUCH, 0); //Clock low
    gpio_put(PIN_MOSI_TOUCH, 0); //Data low
}

bool touch_is_pressed(void) {
    //The IRQ pin goes LOW when the screen is touched
    return !gpio_get(PIN_IRQ_TOUCH);
}

//Helper to send/receive 8 bits
static uint8_t touch_spi_transfer(uint8_t data) {
    uint8_t result = 0;

    for(int i = 0; i < 8; i++) {
        //Set MOSI to the current bit of data
        gpio_put(PIN_MOSI_TOUCH, (data >> (7 - i)) & 1);
        sleep_us(10);

        //Clock up 
        gpio_put(PIN_CLK_TOUCH, 1);
        sleep_us(10);

        //Read MISO 
        if(gpio_get(PIN_MISO_TOUCH)) {
            result |= (1 << (7 - i));
        }

        //Clock down
        gpio_put(PIN_CLK_TOUCH, 0);
        sleep_us(10);
    }
    return result;
}

void touch_read_raw(uint16_t *x, uint16_t *y) {
    if (!touch_is_pressed()) return;

    gpio_put(PIN_CS_TOUCH, 0); //Select chip

    //Read X coordinate
    touch_spi_transfer(0xD0); //Command for X
    uint8_t x_high = touch_spi_transfer(0x00);
    uint8_t x_low = touch_spi_transfer(0x00);
    *x = ((x_high << 8) | x_low) >> 3; //Combine and shift to 12 bits

    //Read Y coordinate
    touch_spi_transfer(0x90); //Command for Y
    uint8_t y_high = touch_spi_transfer(0x00);
    uint8_t y_low = touch_spi_transfer(0x00);
    *y = ((y_high << 8) | y_low) >> 3; //Combine and shift to 12 bits

    gpio_put(PIN_CS_TOUCH, 1); //Deselect chip
}