#include <stdio.h>
#include <string.h>
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

    char cmdbuf[64];
    int cmdpos = 0;

    while (true) {
        int c = getchar_timeout_us(0);
        while (c != PICO_ERROR_TIMEOUT) {
            if (c == '\n' || c == '\r') {
                if (cmdpos > 0) {
                    cmdbuf[cmdpos] = '\0';
                    if (strcmp(cmdbuf, "CMD:ART_START") == 0) {
                        printf("ACK:ART_START\n");
                        
                        int art_w = 200;
                        int art_h = 200;
                        int total_bytes = art_w * art_h * 2;
                        int received_bytes = 0;
                        
                        display_set_window(20, 20, art_w, art_h);
                        
                        uint8_t pixel_buf[1024];
                        int buf_pos = 0;
                        
                        while (received_bytes < total_bytes) {
                            int byteC = getchar_timeout_us(2000000); // 2s timeout
                            if (byteC != PICO_ERROR_TIMEOUT) {
                                pixel_buf[buf_pos++] = (uint8_t)byteC;
                                received_bytes++;
                                
                                if (buf_pos == sizeof(pixel_buf) || received_bytes == total_bytes) {
                                    display_write_pixels(pixel_buf, buf_pos);
                                    buf_pos = 0;
                                }
                            } else {
                                break; // timeout
                            }
                        }
                        printf("ACK:ART_DONE\n");
                    }
                    cmdpos = 0;
                }
            } else {
                if (cmdpos < sizeof(cmdbuf) - 1) {
                    cmdbuf[cmdpos++] = (char)c;
                }
            }
            c = getchar_timeout_us(0);
        }

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
                    printf("CMD:PREV\n");
                    draw_rect(20, 265, 40, 40, C_GREEN); // Highlight
                    sleep_ms(40);
                    draw_rect(20, 265, 40, 40, C_WHITE); // Restore
                } 
                else if (px > 160) {
                    printf("CMD:NEXT\n");
                    draw_rect(180, 265, 40, 40, C_GREEN);
                    sleep_ms(40);
                    draw_rect(180, 265, 40, 40, C_WHITE);
                } 
                else {
                    printf("CMD:PLAY\n");
                    draw_rect(100, 265, 40, 40, C_WHITE); // Blink White
                    sleep_ms(40);
                    draw_rect(100, 265, 40, 40, C_GREEN); // Restore Green
                }
                sleep_ms(80);
            }
        }
        sleep_ms(10);
    }
}