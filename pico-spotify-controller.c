#include <stdio.h>
#include "pico/stdlib.h"
#include "pins.h"
#include "display.h"
#include "touch.h"

// COLORS
#define C_BLACK   0x0000 
#define C_WHITE   0xFFFF
#define C_GRAY    0x3186 
#define C_GREEN   0x07E0 

// --- MAP FUNCTION ---
long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// --- DRAW UI ---
void draw_interface() {
    display_fill(C_BLACK);

    // 1. Draw Button Bar Background (Bottom 70 pixels)
    draw_rect(0, 250, 240, 70, C_GRAY);

    // 2. Draw "Fake" Buttons (Just colored squares for now)
    // Prev Button
    draw_rect(20, 265, 40, 40, C_WHITE);
    
    // Play Button (Green)
    draw_rect(100, 265, 40, 40, C_GREEN);
    
    // Next Button
    draw_rect(180, 265, 40, 40, C_WHITE);
    
    // 3. Draw "Art Placeholder"
    draw_rect(20, 20, 200, 200, 0x18E3); // Dark Gray box for art
}

int main() {
    stdio_init_all();
    display_init();
    touch_init();
    
    draw_interface();

    printf("Spotify Controller Ready.\n");

    while (true) {
        if (touch_is_pressed()) {
            uint16_t raw_x, raw_y;
            touch_read_raw(&raw_x, &raw_y);

            // CALIBRATION 
            int px = map(raw_x, 3800, 200, 0, 240);
            int py = map(raw_y, 150, 3850, 0, 320);

            // Bounds check
            if (px < 0) px = 0; if (px > 240) px = 240;
            if (py < 0) py = 0; if (py > 320) py = 320;

            if (py > 250) { // Button Area
                if (px < 80) {
                    printf("<< PREV\n");
                    draw_rect(20, 265, 40, 40, C_GREEN); // Highlight
                    sleep_ms(100);
                    draw_rect(20, 265, 40, 40, C_WHITE); // Restore
                } 
                else if (px > 160) {
                    printf(">> NEXT\n");
                    draw_rect(180, 265, 40, 40, C_GREEN);
                    sleep_ms(100);
                    draw_rect(180, 265, 40, 40, C_WHITE);
                } 
                else {
                    printf("|| PLAY\n");
                    draw_rect(100, 265, 40, 40, C_WHITE); // Blink White
                    sleep_ms(100);
                    draw_rect(100, 265, 40, 40, C_GREEN); // Restore Green
                }
                sleep_ms(150);
            }
        }
        sleep_ms(10);
    }
}