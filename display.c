#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pins.h"
#include "display.h"

// --- Internal Helper Functions ---
static void lcd_write_cmd(uint8_t cmd) {
    gpio_put(PIN_DC_LCD, 0); // Command Mode
    gpio_put(PIN_CS_LCD, 0); // Select
    spi_write_blocking(SPI_PORT_LCD, &cmd, 1);
    gpio_put(PIN_CS_LCD, 1); // Deselect
}

static void lcd_write_data(uint8_t data) {
    gpio_put(PIN_DC_LCD, 1); // Data Mode
    gpio_put(PIN_CS_LCD, 0); // Select
    spi_write_blocking(SPI_PORT_LCD, &data, 1);
    gpio_put(PIN_CS_LCD, 1); // Deselect
}

// --- Public Functions ---
void display_init(void) {
    // 1. Setup SPI
    spi_init(SPI_PORT_LCD, 20000000);
    gpio_set_function(PIN_SCK_LCD, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI_LCD, GPIO_FUNC_SPI);

    // 2. Setup Control Pins
    gpio_init(PIN_CS_LCD); gpio_set_dir(PIN_CS_LCD, GPIO_OUT); gpio_put(PIN_CS_LCD, 1);
    gpio_init(PIN_DC_LCD); gpio_set_dir(PIN_DC_LCD, GPIO_OUT); gpio_put(PIN_DC_LCD, 1);
    gpio_init(PIN_RST_LCD); gpio_set_dir(PIN_RST_LCD, GPIO_OUT);

    // 3. Reset Sequence
    gpio_put(PIN_RST_LCD, 1); sleep_ms(100);
    gpio_put(PIN_RST_LCD, 0); sleep_ms(100);
    gpio_put(PIN_RST_LCD, 1); sleep_ms(100);

    // 4. Magic Initialization Commands
    lcd_write_cmd(0x01); sleep_ms(150); // Software Reset
    lcd_write_cmd(0x11); sleep_ms(150); // Sleep Out
    
    lcd_write_cmd(0x3A); lcd_write_data(0x55); // Pixel Format: 16-bit
    
    // 0x28 = Exchange Row/Col (Landscape) + BGR color order
    lcd_write_cmd(0x36); lcd_write_data(0x28); 
    // -----------------------
    
    lcd_write_cmd(0x29); // Display ON
    sleep_ms(50);
}

void display_fill(uint16_t color) {
    // Define the "Window" to fill (Entire screen 320x240)
    lcd_write_cmd(0x2A); // Column Set
    lcd_write_data(0); lcd_write_data(0); 
    lcd_write_data(0x01); lcd_write_data(0x3F); // 319
    
    lcd_write_cmd(0x2B); // Page Set
    lcd_write_data(0); lcd_write_data(0);
    lcd_write_data(0x00); lcd_write_data(0xEF); // 239

    lcd_write_cmd(0x2C); // Memory Write
    
    // Prepare the pixel color bytes
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    // Blast data
    gpio_put(PIN_DC_LCD, 1);
    gpio_put(PIN_CS_LCD, 0);
    for(int i = 0; i < 76800; i++) {
        spi_write_blocking(SPI_PORT_LCD, &hi, 1);
        spi_write_blocking(SPI_PORT_LCD, &lo, 1);
    }
    gpio_put(PIN_CS_LCD, 1);
}