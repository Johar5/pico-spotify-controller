#include <stdio.h>
#include "pico/stdlib.h"
#include "pins.h"
#include "display.h"

// Colors (RGB565 format)
#define RED   0xF800
#define GREEN 0x07E0
#define BLUE  0x001F
#define BLACK 0x0000

int main() {
    stdio_init_all();

    // 1. Setup LED (Heartbeat)
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);
    gpio_put(PIN_LED, 1); // Turn ON during init

    // 2. Initialize Screen
    printf("Initializing Display...\n");
    display_init();
    
    // 3. Draw Test Pattern (Red Screen)
    printf("Drawing RED...\n");
    display_fill(RED);

    // 4. Turn LED OFF (Init complete)
    gpio_put(PIN_LED, 0);

    while (true) {
        // Blink slowly to show system is running
        gpio_put(PIN_LED, 1); sleep_ms(1000);
        gpio_put(PIN_LED, 0); sleep_ms(1000);
    }
}