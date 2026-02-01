#ifndef TOUCH_H
#define TOUCH_H

#include <stdint.h>
#include <stdbool.h>

//Initialize touch pins
void touch_init(void);

//Returns true if screen is being touched
bool touch_is_pressed(void);

//Read the raw X and Y coords from the touch screen
//Pointer used to return two values at once
void touch_read_raw(uint16_t *x, uint16_t *y);

#endif //TOUCH_H