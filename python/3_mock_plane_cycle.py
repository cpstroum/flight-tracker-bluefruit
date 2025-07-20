import asyncio
import os
from dotenv import load_dotenv
import random
from bleak import BleakClient, BleakScanner

load_dotenv()

# Low-energy Bluetooth Destination
DEVICE_NAME = os.getenv("DEVICE_NAME")
UART_CHAR_UUID_RX = os.getenv("UART_CHAR_UUID_RX")

# Mock planes with position info
MOCK_PLANES = [
    {"callsign": "DAL122", "dist": 0.4, "bearing": 45},
    {"callsign": "UAL839", "dist": 0.6, "bearing": 135},
    {"callsign": "AAL77",  "dist": 0.2, "bearing": 270},
]

def update_planes():
    for p in MOCK_PLANES:
        # Simulate movement
        p["bearing"] += random.uniform(-5, 5)
        p["bearing"] %= 360
        p["dist"] += random.uniform(-0.02, 0.02)
        p["dist"] = max(0.1, min(0.9, p["dist"]))  # keep within radar range

def format_plane(p):
    return f"{p['callsign']},{p['dist']:.2f},{p['bearing']:.1f}"

async def main():
    print("🔍 Scanning for devices...")
    devices = await BleakScanner.discover()
    target = next((d for d in devices if d.name and DEVICE_NAME in d.name), None)

    if not target:
        print(f"❌ Device '{DEVICE_NAME}' not found.")
        return

    async with BleakClient(target.address) as client:
        print(f"✅ Connected to {DEVICE_NAME}")

        while True:
            update_planes()

            for p in MOCK_PLANES:
                msg = format_plane(p) + "\n"
                print(f"📡 Sending: {msg.strip()}")
                await client.write_gatt_char(UART_CHAR_UUID_RX, msg.encode('utf-8'))
                await asyncio.sleep(0.5)

            print("⏱️ Waiting 60 seconds for next update...\n")
            await asyncio.sleep(60)

asyncio.run(main())
