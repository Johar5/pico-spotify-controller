#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

// Initialize the screen hardware
void display_init(void);

// Fill the entire screen with one color
void display_fill(uint16_t color);

//Draw a specific rectangle 
void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

// Set window for raw pixel data transfer
void display_set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

// Write raw pixel data over SPI
void display_write_pixels(const uint8_t *data, size_t len);

#endif // DISPLAY_H