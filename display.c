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
    
    // 0x88 = portrait
    lcd_write_cmd(0x36); lcd_write_data(0x88); 
    // -----------------------
    
    lcd_write_cmd(0x29); // Display ON
    sleep_ms(50);
}

void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if ((x >= 240) || (y >= 320)) return;
    if ((x + w - 1) >= 240) w = 240 - x;
    if ((y + h - 1) >= 320) h = 320 - y;

    //Set window 
    lcd_write_cmd(0x2A);
    lcd_write_data(x >> 8); lcd_write_data(x & 0xFF);
    lcd_write_data((x + w - 1) >> 8); lcd_write_data((x + w - 1) & 0xFF);

    lcd_write_cmd(0x2B);
    lcd_write_data(y >> 8); lcd_write_data(y & 0xFF);
    lcd_write_data((y + h - 1) >> 8); lcd_write_data((y + h - 1) & 0xFF);

    //Write memory
    lcd_write_cmd(0x2C);

    //Prepare color bytes
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    gpio_put(PIN_DC_LCD, 1); 
    gpio_put(PIN_CS_LCD, 0);

    //Optimization strategy: Create a small buffer to speed up SPI
    //Writing chunks is faster then 1 byte at a time
    uint8_t line_buffer[64];
    for (int i = 0; i < 32; i++) {
        line_buffer[i*2] = hi;
        line_buffer[i*2 + 1] = lo;
    }

    int total_pixels = w * h;
    int bytes_per_chunk = 64;
    int pixels_per_chunk = bytes_per_chunk / 2;

    while(total_pixels > 0) {
        int pixels_to_send = (total_pixels > pixels_per_chunk) ? pixels_per_chunk : total_pixels;
        spi_write_blocking(SPI_PORT_LCD, line_buffer, pixels_to_send * 2);
        total_pixels -= pixels_to_send;
    }

    gpio_put(PIN_CS_LCD, 1);
}

void display_fill(uint16_t color) {
    draw_rect(0,0,240,320,color);
}