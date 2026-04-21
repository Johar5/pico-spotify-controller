import serial 
import serial.tools.list_ports
import time 
import os 
import spotipy
from spotipy.oauth2 import SpotifyOAuth
from dotenv import load_dotenv
import requests
from PIL import Image
from io import BytesIO

# Load credentials from .env file
load_dotenv()

#define the permissions the app will need
SCOPE = "user-modify-playback-state user-read-playback-state"

#Initialize spotify client
try: 
    sp = spotipy.Spotify(auth_manager=SpotifyOAuth(scope=SCOPE))
    user = sp.current_user()
    print(f"Logged in as {user['display_name']}")
except Exception as e:
    print("Error logging into Spotify. Check your .env file:", e)
    exit(1)

# Serial setup 
BAUD_RATE = 115200
def find_pico_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if "USB Serial Device" in port.description: 
            return port.device
    return None

def send_art_to_pico(ser, img_url):
    print("Fetching album art...")
    try:
        response = requests.get(img_url)
        img = Image.open(BytesIO(response.content)).convert('RGB')
        img = img.resize((200, 200))
        
        # Convert to RGB565: RRRRRGGG GGGBBBBB
        pixels = list(img.getdata())
        rgb565_data = bytearray()
        for r, g, b in pixels:
            val = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            rgb565_data.append(val >> 8)
            rgb565_data.append(val & 0xFF)
            
        print("Sending art to Pico...")
        ser.write(b"CMD:ART_START\n")
        ser.flush()
        
        # Wait for ACK
        start_t = time.time()
        ack_received = False
        while time.time() - start_t < 2:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line == "ACK:ART_START":
                    ack_received = True
                    break
        
        if not ack_received:
            print("Timeout waiting for ACK:ART_START")
            return
            
        # Send image data in larger chunks without artificial sleep
        chunk_size = 8192
        for i in range(0, len(rgb565_data), chunk_size):
            ser.write(rgb565_data[i:i+chunk_size])
            ser.flush()
        
        # Wait for completion
        start_t = time.time()
        while time.time() - start_t < 5:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line == "ACK:ART_DONE":
                    print("Art transfer complete!")
                    break
    except Exception as e:
        print(f"Error transferring art: {e}")

def main():
    print("--- Spotify API Bridge Started ---")

    port_name = find_pico_port()
    if not port_name:
        print("Raspberry Pi Pico not found. Please connect the device.")
        return
    
    try: 
        ser = serial.Serial(port_name, BAUD_RATE, timeout=1) 
        time.sleep(2)
        print(f"Connected to {port_name}")

        last_track_id = None
        last_check_time = 0

        while True:
            # Check for incoming commands
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()

                if line.startswith("CMD:"):
                    try: 
                        if line == "CMD:PLAY":
                            current = sp.current_playback()
                            if current and current['is_playing']:
                                sp.pause_playback()
                            else:
                                sp.start_playback()
                            time.sleep(0.2) # Give Spotify API a moment to register playback
                            last_check_time = 0 # Force a re-poll immediately
                        elif line == "CMD:NEXT":
                            sp.next_track()
                            time.sleep(0.2) 
                            last_check_time = 0
                        elif line == "CMD:PREV":
                            sp.previous_track()
                            time.sleep(0.2)
                            last_check_time = 0
                    except spotipy.exceptions.SpotifyException as e:
                        print("Spotify API error:", e)
            
            # Periodically check for track changes
            now = time.time()
            if now - last_check_time > 1.5:  # Check more frequently
                last_check_time = now
                try:
                    current_track = sp.current_user_playing_track()
                    if current_track and current_track['item']:
                        track_id = current_track['item']['id']
                        if track_id != last_track_id:
                            last_track_id = track_id
                            print(f"New track playing: {current_track['item']['name']}")
                            # Get the medium-sized album art (usually 300x300, index 1)
                            images = current_track['item']['album']['images']
                            if len(images) > 0:
                                # Prioritize mid-size image or fallback
                                img_url = images[1]['url'] if len(images) > 1 else images[0]['url']
                                send_art_to_pico(ser, img_url)
                except Exception as e:
                    pass # Ignore temporary errors
            
            time.sleep(0.05)
    except serial.SerialException as e:
        print("Serial port error:", e)

if __name__ == "__main__":
    main()
