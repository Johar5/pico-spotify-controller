# Pico Spotify Controller

A hardware Spotify controller powered by a Raspberry Pi Pico. This project fetches your current Spotify playback via the Spotipy API running on a host machine, renders a custom UI (including playing album art, track information, and interactive playback buttons), and streams it directly to your Pico's connected LCD. It features native touch-screen control to skip, go back, and play/pause your music.

## Features
- **Live Album Art**: Fetches and downscales current playing cover art to cleanly fit the 240x320 LCD screen.
- **Dynamic Track Data**: Automatically scales long song titles to fit the screen and natively renders track info using built-in Windows system fonts.
- **Touch-screen Buttons**: Tap directly on the Pi screen to control music (Play, Pause, Skip, Prev). Built with dynamic hardware bounds calibration.
- **Optimized SPI Over Serial**: Pushes UI changes in optimized horizontal band chunks to ensure UI stays fast and snappy.

## Requirements
### Hardware
- Raspberry Pi Pico (RP2040)
- SPI LCD screen with Touch support
- USB Cable to host PC

### Software
- **C/C++ Pico SDK**: For compiling and flashing the Pico firmware.
- **Python 3.12+**: Host-side script to connect to Spotify.

## Setup

1. **Spotify API Setup**: 
    - Create an app on the [Spotify Developer Dashboard](https://developer.spotify.com/dashboard/).
    - Grab your `Client ID`, `Client Secret`, and set the Redirect URI to `http://localhost:8888/callback`.
    - Create a `.env` file inside the `host/` folder structured as:
      ```env
      SPOTIPY_CLIENT_ID='your_client_id_here'
      SPOTIPY_CLIENT_SECRET='your_client_secret_here'
      SPOTIPY_REDIRECT_URI='http://localhost:8888/callback'
      ```
    
2. **Flash the Pico**: 
    - Compile the C script `pico-spotify-controller.c` with CMake and the Pico SDK, and flash the compiled `.uf2` file onto your Pico.
   
3. **Install Dependencies**:
    - Open your terminal and run the following in your host environment:
      ```bash
      pip install spotipy pyserial pillow python-dotenv requests
      ```

## Running the App
Once your Pico is plugged in and displaying the waiting screen, simply double-click the **`start_bridge.bat`** file in the main folder. It will boot up the Python bridge, automatically locate the Pico via USB serial, and seamlessly sync your current Spotify session onto the hardware controller!
