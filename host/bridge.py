import serial 
import serial.tools.list_ports
import time 
import os 
import spotipy
from spotipy.oauth2 import SpotifyOAuth
from dotenv import load_dotenv

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

        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8').strip()

                if line.startswith("CMD:"):
                    try: 
                        if line == "CMD:PLAY":
                            current = sp.current_playback()
                            if current and current['is_playing']:
                                sp.pause_playback()
                            else:
                                sp.start_playback()
                        elif line == "CMD:NEXT":
                            sp.next_track()
                        elif line == "CMD:PREV":
                            sp.previous_track()
                    except spotipy.exceptions.SpotifyException as e:
                        print("Spotify API error:", e)
            time.sleep(0.1)
    except serial.SerialException as e:
        print("Serial port error:", e)

if __name__ == "__main__":
    main()
