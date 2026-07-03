# Full manual flash program script for STM32H7RS

proc flash_program_bin {binfile} {
    set FLASH_BASE 0x52002000
    set FLASH_KEYR [expr {$FLASH_BASE + 0x04}]
    set FLASH_CR   [expr {$FLASH_BASE + 0x0C}]
    set FLASH_SR   [expr {$FLASH_BASE + 0x20}]
    set FLASH_CCR  [expr {$FLASH_BASE + 0x28}]
    set FLASH_ADDR 0x08000000

    # Open binary file
    set fd [open $binfile rb]
    fconfigure $fd -translation binary
    set data [read $fd]
    close $fd

    set len [string length $data]
    echo "Programming $len bytes to flash..."

    # Unlock flash
    mww $FLASH_KEYR 0x45670123
    mww $FLASH_KEYR 0xCDEF89AB
    set cr [mrw $FLASH_CR]
    if {$cr & 0x80000000} {
        error "Flash unlock failed: CR=0x[format %08x $cr]"
    }

    # Erase all sectors needed (first 8KB = sector 0)
    echo "Erasing sector 0..."
    set cr [mrw $FLASH_CR]
    set cr [expr {$cr & ~0xFFF}]  ;# clear bits 0-11
    set cr [expr {$cr | 0x02}]     ;# SER=1
    mww $FLASH_CR $cr
    set cr [mrw $FLASH_CR]
    set cr [expr {$cr | 0x10000}]  ;# START=1
    mww $FLASH_CR $cr
    for {set i 0} {$i < 10000} {incr i} {
        set sr [mrw $FLASH_SR]
        if {($sr & 0x01) == 0} break
        sleep 1
    }
    mww $FLASH_CR [expr {[mrw $FLASH_CR] & ~0x02}]
    echo "Sector 0 erased"

    # Program: set PG=1, PSIZE=3 (64-bit)
    set cr [mrw $FLASH_CR]
    set cr [expr {$cr & ~0xFFF}]
    set cr [expr {$cr | 0x01 | (3 << 8)}]  ;# PG=1, PSIZE=3(64-bit)
    mww $FLASH_CR $cr

    # Write data 8 words (32 bytes) at a time — 256-bit flash word
    set addr $FLASH_ADDR
    set pos 0
    set words_per_block 8

    while {$pos < $len} {
        # Write up to 8 words (256-bit flash row)
        set block_end [expr {$pos + $words_per_block * 4}]
        if {$block_end > $len} { set block_end $len }

        for {set i $pos} {$i < $block_end} {incr i 4} {
            binary scan $data "@${i}I" word
            mww [expr {$addr + $i - $pos}] $word
        }

        # Wait for BSY to clear
        for {set w 0} {$w < 10000} {incr w} {
            set sr [mrw $FLASH_SR]
            if {($sr & 0x01) == 0} break
            sleep 1
        }

        # Check for errors
        set sr [mrw $FLASH_SR]
        if {$sr & 0xFFFFFFFE} {
            echo "Flash error: SR=0x[format %08x $sr] at addr 0x[format %08x $addr]"
            # Clear errors
            mww $FLASH_CCR $sr
        }

        set addr [expr {$addr + $words_per_block * 4}]
        set pos $block_end

        if {[expr {$pos % 32}] == 0} {
            echo -n "."
        }
    }

    # Clear PG
    mww $FLASH_CR [expr {[mrw $FLASH_CR] & ~0x01}]
    echo ""
    echo "Programming complete: $len bytes written"

    # Lock flash
    set cr [mrw $FLASH_CR]
    mww $FLASH_CR [expr {$cr | 0x80000000}]
}
