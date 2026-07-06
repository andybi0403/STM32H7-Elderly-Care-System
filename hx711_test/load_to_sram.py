"""Load HX711+LCD firmware to STM32H7R3 SRAM via pyOCD"""
import sys
from pyocd.core.helpers import ConnectHelper

BIN_FILE = "build/hx711_lcd.bin"
SRAM_BASE = 0x20000000
PROBE_ID = "ATK-03262021"

with open(BIN_FILE, "rb") as f:
    data = f.read()

print(f"Loading {len(data)} bytes to SRAM...")

session = ConnectHelper.session_with_chosen_probe(
    target_override="cortex_m", unique_id=PROBE_ID
)
session.open()
target = session.target
target.reset_and_halt()

# Write binary to SRAM
for i in range(0, len(data), 4):
    if i + 3 < len(data):
        word = data[i] | (data[i+1] << 8) | (data[i+2] << 16) | (data[i+3] << 24)
    else:
        word = data[i]
    target.write32(SRAM_BASE + i, word)

# Verify
errors = 0
for i in range(0, len(data), 4):
    expected = (
        data[i] | (data[i+1] << 8) | (data[i+2] << 16) | (data[i+3] << 24)
        if i + 3 < len(data) else data[i]
    )
    if target.read32(SRAM_BASE + i) != expected:
        errors += 1

if errors:
    print(f"FAIL: {errors} verification errors")
    sys.exit(1)

# Set SP and PC from vector table
sp = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24)
pc = (data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24)) & ~1
target.write_core_register("sp", sp)
target.write_core_register("pc", pc)
target.resume()

print(f"OK! SP=0x{sp:08X} PC=0x{pc:08X}")
print("Screen should show PILL WEIGHT.")
session.close()
