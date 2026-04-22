import serial 
import serial.tools.list_ports
import time 
import os 
import spotipy
from spotipy.oauth2 import SpotifyOAuth
from dotenv import load_dotenv
import requests
from PIL import Image, ImageDraw, ImageFont
from io import BytesIO
import os

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

def get_font(url, filename, size):
    # Use absolute path for caching so it never downloads repeatedly depending on workdir
    cache_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), ".cache")
    if not os.path.exists(cache_dir):
        os.makedirs(cache_dir)
    filepath = os.path.join(cache_dir, filename)
    
    if not os.path.exists(filepath):
        try:
            print(f"Downloading font {filename} (Initial setup only)...")
            r = requests.get(url, timeout=10)
            if r.status_code == 200:
                with open(filepath, 'wb') as f:
                    f.write(r.content)
            else:
                print(f"Failed to download font: {r.status_code}")
                return ImageFont.load_default()
        except Exception as e:
            print(f"Error downloading font: {e}")
            return ImageFont.load_default()
    try:
        return ImageFont.truetype(filepath, size)
    except:
        # If the file is corrupted, delete it so it can retry next run
        try:
            os.remove(filepath)
        except:
            pass
        return ImageFont.load_default()

# Global cache to prevent re-downloading same album art
cached_art_url = None
cached_art_image = None

def render_full_canvas(img_url, title, artist, is_playing):
    global cached_art_url, cached_art_image
    
    canvas = Image.new('RGB', (240, 320), (0, 0, 0))
    draw = ImageDraw.Draw(canvas)
    
    # 1. Album Art
    if img_url:
        if img_url != cached_art_url:
            try:
                response = requests.get(img_url, timeout=3)
                cached_art_image = Image.open(BytesIO(response.content)).convert('RGB').resize((200, 200)) 
                cached_art_url = img_url
            except:
                pass
        
        if cached_art_image:
            canvas.paste(cached_art_image, (20, 5)) # Centered horizontally, pushed to top
            
    # 2. Text
    font_bold_size = 20
    font_reg_size = 14 # slightly smaller default artist font to make it distinct
    
    def get_fonts(bold_size, reg_size):
        try:
            fb = ImageFont.truetype("meiryob.ttc", bold_size)
            fr = ImageFont.truetype("meiryo.ttc", reg_size)
            return fb, fr
        except:
            try:
                fb = ImageFont.truetype("msgothic.ttc", bold_size)
                fr = ImageFont.truetype("msgothic.ttc", reg_size)
                return fb, fr
            except:
                try:
                    fb = ImageFont.truetype("arialbd.ttf", bold_size)
                    fr = ImageFont.truetype("arial.ttf", reg_size)
                    return fb, fr
                except:
                    return ImageFont.load_default(), ImageFont.load_default()

    font_bold, font_reg = get_fonts(font_bold_size, font_reg_size)
    
    # Scale down title if too big
    try:
        tw = draw.textlength(title, font=font_bold)
    except:
        tw = font_bold.getsize(title)[0]
        
    while tw > 230 and font_bold_size > 10:
        font_bold_size -= 1
        font_bold, _ = get_fonts(font_bold_size, font_reg_size)
        try:
            tw = draw.textlength(title, font=font_bold)
        except:
            tw = font_bold.getsize(title)[0]
            
    # Scale down artist if too big
    try:
        aw = draw.textlength(artist, font=font_reg)
    except:
        aw = font_reg.getsize(artist)[0]
        
    while aw > 230 and font_reg_size > 9:
        font_reg_size -= 1
        _, font_reg = get_fonts(font_bold_size, font_reg_size)
        try:
            aw = draw.textlength(artist, font=font_reg)
        except:
            aw = font_reg.getsize(artist)[0]
            
    # Draw text centered, adjusting y-position slightly to account for font height differences
    draw.text((120 - tw / 2, 212 + (20 - font_bold_size) // 2), title, font=font_bold, fill=(255, 255, 255))
    draw.text((120 - aw / 2, 238 + (14 - font_reg_size) // 2), artist, font=font_reg, fill=(180, 180, 180))
    
    # 3. Buttons
    px, py = 50, 285 # Prev
    draw.polygon([(px+5, py-10), (px+5, py+10), (px-10, py)], fill=(160, 160, 160))
    draw.rectangle([(px-15, py-10), (px-12, py+10)], fill=(160, 160, 160))
    
    nx, ny = 190, 285 # Next
    draw.polygon([(nx-5, ny-10), (nx-5, ny+10), (nx+10, ny)], fill=(160, 160, 160))
    draw.rectangle([(nx+12, ny-10), (nx+15, ny+10)], fill=(160, 160, 160))
    
    cx, cy = 120, 285 # Play/Pause
    r = 24
    draw.ellipse([(cx-r, cy-r), (cx+r, cy+r)], fill=(255, 255, 255))
    
    if is_playing:
        draw.rectangle([(cx-6, cy-8), (cx-2, cy+8)], fill=(0, 0, 0))
        draw.rectangle([(cx+2, cy-8), (cx+6, cy+8)], fill=(0, 0, 0))
    else:
        draw.polygon([(cx-4, cy-9), (cx-4, cy+9), (cx+8, cy)], fill=(0, 0, 0))
        
    return canvas

def send_patch_to_pico(ser, canvas, x, y, w, h):
    patch = canvas.crop((x, y, x+w, y+h))
    
    pixels = list(patch.getdata())
    rgb565_data = bytearray()
    for r, g, b in pixels:
        val = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
        rgb565_data.append(val >> 8)
        rgb565_data.append(val & 0xFF)
        
    # print(f"Pushing patch X:{x} Y:{y} W:{w} H:{h}...")
    ser.write(f"CMD:DRAW_START {x} {y} {w} {h}\n".encode('utf-8'))
    ser.flush()
    
    start_t = time.time()
    ack_received = False
    while time.time() - start_t < 2:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line == "ACK:DRAW_START":
                ack_received = True
                break
    
    if not ack_received:
        print("Timeout waiting for ACK:DRAW_START")
        return
        
    # Write fully in bulk
    ser.write(rgb565_data)
    ser.flush()
    
    start_t = time.time()
    while time.time() - start_t < 3:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line == "ACK:DRAW_DONE":
                break

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
        last_is_playing = None
        last_title = ""
        last_artist = ""
        last_img_url = ""
        last_check_time = 0

        while True:
            # Check for incoming commands
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()

                if line.startswith("CMD:"):
                    try: 
                        if line == "CMD:PLAY":
                            if last_is_playing is not None:
                                # Locally toggle state instantly rather than querying Spotify
                                if last_is_playing:
                                    sp.pause_playback()
                                    last_is_playing = False
                                else:
                                    sp.start_playback()
                                    last_is_playing = True
                                
                                # Instantly re-render buttons
                                canvas = render_full_canvas(last_img_url, last_title, last_artist, last_is_playing)
                                # ONLY update the Play/Pause circle footprint (X:90, Y:260, W:60, H:50)
                                send_patch_to_pico(ser, canvas, 90, 260, 60, 50)
                                last_check_time = time.time() + 1.0 # delay next sync
                            else:
                                current = sp.current_playback()
                                if current and current['is_playing']:
                                    sp.pause_playback()
                                else:
                                    sp.start_playback()
                                last_check_time = 0 
                                
                        elif line == "CMD:NEXT":
                            sp.next_track()
                            last_check_time = 0
                        elif line == "CMD:PREV":
                            sp.previous_track()
                            last_check_time = 0
                    except spotipy.exceptions.SpotifyException as e:
                        print("Spotify API error:", e)
            
            # Periodically check for track changes
            now = time.time()
            if now - last_check_time > 1.5:  # Check more frequently
                last_check_time = now
                try:
                    current = sp.current_playback()
                    if current and current['item']:
                        track_id = current['item']['id']
                        is_playing = current['is_playing']
                        
                        if track_id != last_track_id or is_playing != last_is_playing:
                            track_changed = (track_id != last_track_id)
                            play_changed = (is_playing != last_is_playing)
                            
                            last_track_id = track_id
                            last_is_playing = is_playing
                            
                            last_title = current['item']['name']
                            last_artist = current['item']['artists'][0]['name']
                            
                            images = current['item']['album']['images']
                            last_img_url = images[1]['url'] if len(images) > 1 else (images[0]['url'] if images else None)
                            
                            print(f"Updating Display: {last_title} - {last_artist} [{'Playing' if is_playing else 'Paused'}]")
                            
                            canvas = render_full_canvas(last_img_url, last_title, last_artist, last_is_playing)
                            
                            if track_changed:
                                print("Flushing track changes iteratively to maintain speed...")
                                # Send Top Half of Image (X:0 Y:0 W:240 H:105)
                                send_patch_to_pico(ser, canvas, 0, 0, 240, 105)
                                # Send Bottom Half of Image (X:0 Y:105 W:240 H:105)
                                send_patch_to_pico(ser, canvas, 0, 105, 240, 105)
                                # Send Meta Text Box (X:0 Y:210 W:240 H:50)
                                send_patch_to_pico(ser, canvas, 0, 210, 240, 50)
                                # Send Buttons Box (X:0 Y:260 W:240 H:60)
                                send_patch_to_pico(ser, canvas, 0, 260, 240, 60)
                            elif play_changed:
                                print("Flushing ONLY play button state...")
                                # Only update the Play/Pause Icon Box
                                send_patch_to_pico(ser, canvas, 90, 260, 60, 50)
                except Exception as e:
                    pass # Ignore temporary errors
            
            time.sleep(0.05)
    except serial.SerialException as e:
        print("Serial port error:", e)

if __name__ == "__main__":
    main()
