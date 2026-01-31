cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_FETCH_FROM_GIT ON)


include(pico_sdk_import.cmake)

project(pico_spotify_controller C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# LIST ALL C FILES HERE
add_executable(pico_spotify_controller
    pico-spotify-controller.c
    display.c
)

# Enable USB (so we can maybe see the print message later)
pico_enable_stdio_usb(pico_spotify_controller 1)
pico_enable_stdio_uart(pico_spotify_controller 0)

target_link_libraries(pico_spotify_controller
    pico_stdlib
    hardware_spi
)

# Create the UF2 file
pico_add_extra_outputs(pico_spotify_controller)