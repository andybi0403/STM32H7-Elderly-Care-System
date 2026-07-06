# Flash script for STM32H7RS — manual register access
# Flash controller base: 0x52002000

proc flash_servo {binfile} {
    set FLASH_KEYR [expr {0x52002000 + 0x04}]
    set FLASH_CR   [expr {0x52002000 + 0x0C}]
    set FLASH_SR   [expr {0x52002000 + 0x20}]
    set FLASH_CCR  [expr {0x52002000 + 0x28}]
    set FLASH_ADDR 0x08000000

    # Read binary file as hex bytes
    set fd [open $binfile r]
    set data [read $fd]
    close $fd
    set len [string length $data]
    echo "=== Programming $len bytes to STM32H7RS flash ==="

    # Unlock
    echo "Unlocking flash..."
    mww $FLASH_KEYR 0x45670123
    mww $FLASH_KEYR 0xCDEF89AB
    if {[mrw $FLASH_CR] & 0x80000000} { error "Flash unlock failed" }

    # Sector erase
    echo "Erasing..."
    set cr [mrw $FLASH_CR]
    set cr [expr {($cr & ~0xFFF) | 0x02}]
    mww $FLASH_CR $cr
    set cr [mrw $FLASH_CR]
    mww $FLASH_CR [expr {$cr | 0x10000}]
    for {set i 0} {$i < 50000} {incr i} {
        if {([mrw $FLASH_SR] & 0x01) == 0} break
    }
    mww $FLASH_CR [expr {[mrw $FLASH_CR] & ~0x02}]
    echo "Erase OK"

    # Program: PG=1, PSIZE=3(64bit)
    echo "Programming..."
    set cr [mrw $FLASH_CR]
    set cr [expr {($cr & ~0xFFF) | 0x01 | (3 << 8)}]
    mww $FLASH_CR $cr

    set addr $FLASH_ADDR
    set pos 0
    while {$pos < $len} {
        # Write up to 8 words (32 bytes = one 256-bit flash row)
        set word_count 0
        for {set i $pos} {$i < $pos + 32 && $i < $len} {incr i 4} {
            # Build 32-bit word from 4 bytes (little-endian)
            set b0 [scan [string index $data $i]     %c]
            set b1 [scan [string index $data [expr {$i+1}]] %c]
            set b2 [scan [string index $data [expr {$i+2}]] %c]
            set b3 [scan [string index $data [expr {$i+3}]] %c]
            set word [expr {$b0 | ($b1 << 8) | ($b2 << 16) | ($b3 << 24)}]
            mww [expr {$addr + ($i - $pos)}] $word
            incr word_count
        }

        # Wait BSY
        for {set w 0} {$w < 50000} {incr w} {
            if {([mrw $FLASH_SR] & 0x01) == 0} break
        }
        set sr [mrw $FLASH_SR]
        if {$sr & ~0x01} { mww $FLASH_CCR $sr }

        set addr [expr {$addr + $word_count * 4}]
        set pos  [expr {$pos  + $word_count * 4}]

        if {[expr {$pos % 128}] == 0} { echo -n "." }
    }

    # Done
    mww $FLASH_CR [expr {[mrw $FLASH_CR] & ~0x01}]
    echo ""
    echo "=== Program OK: $len bytes ==="

    # Lock
    mww $FLASH_CR [expr {[mrw $FLASH_CR] | 0x80000000}]
}
