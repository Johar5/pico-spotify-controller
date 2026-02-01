# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

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
    touch.c
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