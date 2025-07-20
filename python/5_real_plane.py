import asyncio
import requests
import math
import os
from dotenv import load_dotenv
from bleak import BleakClient, BleakScanner

load_dotenv()

# Your location or Seattle, WA
MY_LAT = float(os.getenv("MY_LAT","47.603153"))
MY_LON = float(os.getenv("MY_LON","-122.34272"))

# ADS-B Exchange API
ADSBX_KEY = os.getenv("ADSBX_KEY")
ADSBX_HOST = os.getenv("ADSBX_HOST")
DIST_NM = int(os.getenv("RADIUS_NM", "50"))  # nautical miles

# Low-energy Bluetooth Destination
DEVICE_NAME = os.getenv("DEVICE_NAME")
UART_CHAR_UUID_RX = os.getenv("UART_CHAR_UUID_RX")

def haversine(lat1, lon1, lat2, lon2):
    R = 6371  # km
    dlat = math.radians(lat2 - lat1)
    dlon = math.radians(lon2 - lon1)
    a = math.sin(dlat / 2)**2 + math.cos(math.radians(lat1)) * math.cos(math.radians(lat2)) * math.sin(dlon / 2)**2
    return R * 2 * math.asin(math.sqrt(a))

def bearing(lat1, lon1, lat2, lon2):
    dlon = math.radians(lon2 - lon1)
    y = math.sin(dlon) * math.cos(math.radians(lat2))
    x = math.cos(math.radians(lat1)) * math.sin(math.radians(lat2)) - math.sin(math.radians(lat1)) * math.cos(math.radians(lat2)) * math.cos(dlon)
    return (math.degrees(math.atan2(y, x)) + 360) % 360

def fetch_planes():
    url = f"https://{ADSBX_HOST}/v2/lat/{MY_LAT}/lon/{MY_LON}/dist/{DIST_NM}/"
    headers = {
        "x-rapidapi-host": ADSBX_HOST,
        "x-rapidapi-key": ADSBX_KEY,
    }

    r = requests.get(url, headers=headers, timeout=10)
    r.raise_for_status()
    ac = r.json().get("ac", [])

    planes = []
    for p in ac:
        if "lat" in p and "lon" in p and "flight" in p:
            dist_km = haversine(MY_LAT, MY_LON, p["lat"], p["lon"])
            brg = bearing(MY_LAT, MY_LON, p["lat"], p["lon"])
            dist_ratio = min(dist_km / 100, 1.0)  # scale for screen (100km max)
            callsign = p["flight"].strip() or "UNK"
            planes.append((callsign, round(dist_ratio, 2), round(brg, 1)))
    return planes

async def main():
    print("🔍 Scanning for devices...")
    devices = await BleakScanner.discover()
    target = next((d for d in devices if d.name and DEVICE_NAME in d.name), None)

    if not target:
        print("❌ Device not found")
        return

    async with BleakClient(target.address) as client:
        print(f"✅ Connected to {DEVICE_NAME}")

        while True:
            print("📡 Fetching real planes...")
            try:
                planes = fetch_planes()
                for cs, dist, brg in planes[:5]:
                    msg = f"{cs},{dist},{brg}\n"
                    print(f"🛬 {msg.strip()}")
                    await client.write_gatt_char(UART_CHAR_UUID_RX, msg.encode('utf-8'))
                    await asyncio.sleep(0.5)
                print("⏱️ Waiting 60s...")
                await asyncio.sleep(60)
            except Exception as e:
                print("⚠️ Error fetching or sending:", e)
                await asyncio.sleep(15)

asyncio.run(main())
