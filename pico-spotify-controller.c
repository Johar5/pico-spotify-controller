#include <stdio.h>
#include "pico/stdlib.h"
#include "pins.h"
#include "display.h"


int main()
{
    stdio_init_all();

    printf("Booting Spotify Controller...\n");

    //Initialize display
    display_init();
    printf("Display SPI initialized.\n");

    //Loop so board doesnt shut down
    while (true) {
        tight_loop_contents();
    }
}

