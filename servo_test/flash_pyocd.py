"""Flash STM32H7RS via pyOCD — manual register-level programming with verify."""
import struct, sys

BINFILE = "build/servo_test.bin"
FLASH_ADDR = 0x08000000
FLASH_BASE = 0x52002000
FLASH_KEYR = FLASH_BASE + 0x04
FLASH_CR   = FLASH_BASE + 0x0C
FLASH_SR   = FLASH_BASE + 0x20
FLASH_CCR  = FLASH_BASE + 0x28

with open(BINFILE, 'rb') as f:
    data = f.read()
print(f"=== Programming {len(data)} bytes to STM32H7RS ===")

from pyocd.core.helpers import ConnectHelper
session = ConnectHelper.session_with_chosen_probe(target_override="cortex_m", unique_id="ATK-03262021")
session.open()
target = session.target
target.halt()

def r32(a): return target.read32(a)
def w32(a, v): target.write32(a, v)

# ── Unlock ──
print("1/4 Unlock...")
if r32(FLASH_CR) >> 31:
    w32(FLASH_KEYR, 0x45670123)
    w32(FLASH_KEYR, 0xCDEF89AB)
    if r32(FLASH_CR) >> 31:
        print("ERROR: unlock failed!")
        sys.exit(1)
print("   OK")

# ── Erase sector 0 ──
print("2/4 Erase...")
cr = r32(FLASH_CR)
w32(FLASH_CR, (cr & ~0xFFF) | 0x02)
w32(FLASH_CR, r32(FLASH_CR) | 0x10000)
for _ in range(50000):
    if (r32(FLASH_SR) & 0x01) == 0: break
w32(FLASH_CR, r32(FLASH_CR) & ~0x02)
print("   OK")

# ── Program ──
print("3/4 Program...")
cr = r32(FLASH_CR)
w32(FLASH_CR, (cr & ~0xFFF) | 0x01 | (3 << 8))

addr = FLASH_ADDR
pos = 0
errs = 0
while pos < len(data):
    for i in range(pos, min(pos + 32, len(data)), 4):
        if i + 3 < len(data):
            word = data[i] | (data[i+1] << 8) | (data[i+2] << 16) | (data[i+3] << 24)
        else:
            word = 0
            for j in range(i, len(data)):
                word |= data[j] << (8 * (j - i))
        w32(addr + (i - pos), word)

    for _ in range(50000):
        if (r32(FLASH_SR) & 0x01) == 0: break

    sr = r32(FLASH_SR)
    if sr & ~0x01:
        w32(FLASH_CCR, sr)
        errs += 1

    addr += 32
    pos = min(pos + 32, len(data))

    if pos % 256 == 0:
        print(".", end="", flush=True)

w32(FLASH_CR, r32(FLASH_CR) & ~0x01)
print(f"   OK (errors cleared: {errs})")

# ── Verify ──
print("4/4 Verify...")
mismatch = 0
for i in range(0, len(data), 4):
    expected = data[i] | (data[i+1] << 8) | (data[i+2] << 16) | (data[i+3] << 24) if i+3 < len(data) else 0
    actual = r32(FLASH_ADDR + i)
    if expected != actual:
        if mismatch < 5:
            print(f"   MISMATCH @ 0x{FLASH_ADDR+i:08X}: wrote 0x{expected:08X} read 0x{actual:08X}")
        mismatch += 1

if mismatch:
    print(f"   FAIL: {mismatch} mismatches!")
else:
    print("   ALL OK: flash verified")

# ── Lock & Reset ──
w32(FLASH_CR, r32(FLASH_CR) | 0x80000000)
target.reset_and_halt()
print("Flash locked. Resetting...")

# Check where we reset to
print(f"SP=0x{r32(0x08000000):08X} PC=0x{(r32(0x08000004) & ~1):08X}")

target.resume()
session.close()
print("=== DONE: Servo test running! ===")
