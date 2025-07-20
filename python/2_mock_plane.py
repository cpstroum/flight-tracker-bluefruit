import asyncio
import os
from dotenv import load_dotenv
from bleak import BleakClient, BleakScanner

load_dotenv()

# 🛩️ Mock flight messages
MOCK_FLIGHTS = [
    "DAL122 → SEA | 32,000 ft | 450 kt",
    "UAL839 → SFO | 36,000 ft | 470 kt",
    "AAL77 → DFW | 34,000 ft | 430 kt"
]

# Low-energy Bluetooth Destination
DEVICE_NAME = os.getenv("DEVICE_NAME")
UART_CHAR_UUID_RX = os.getenv("UART_CHAR_UUID_RX")

async def main():
    print("🔍 Scanning for Bluetooth devices...")
    devices = await BleakScanner.discover()
    for d in devices:
      print(f"📡 Found device: {d.name} [{d.address}]")

    target = next((d for d in devices if d.name and DEVICE_NAME in d.name), None)

    if not target:
        print(f"❌ Couldn't find '{DEVICE_NAME}' — is your board on and advertising?")
        return

    async with BleakClient(target.address) as client:
        print(f"✅ Connected to {DEVICE_NAME}")

        for msg in MOCK_FLIGHTS:
            print(f"📡 Sending: {msg}")
            await client.write_gatt_char(UART_CHAR_UUID_RX, msg.encode('utf-8'))
            await asyncio.sleep(2)

        print("✅ All messages sent.")

asyncio.run(main())
