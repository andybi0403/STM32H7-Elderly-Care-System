# Manual flash programming for STM32H7RS
# Usage: openocd ... -c "source [find flash_manual.tcl]"

set FLASH_BASE 0x52002000
set FLASH_KEYR [expr {$FLASH_BASE + 0x04}]
set FLASH_CR   [expr {$FLASH_BASE + 0x0C}]
set FLASH_SR   [expr {$FLASH_BASE + 0x20}]
set FLASH_CCR  [expr {$FLASH_BASE + 0x28}]

# Wait for BSY clear
proc flash_wait_bsy {} {
    for {set i 0} {$i < 1000} {incr i} {
        set sr [mrw $::FLASH_SR]
        if {($sr & 0x01) == 0} { return }
        sleep 1
    }
    error "Flash timeout: SR=0x[format %08x $sr]"
}

# Unlock flash CR
proc flash_unlock {} {
    mww $::FLASH_KEYR 0x45670123
    mww $::FLASH_KEYR 0xCDEF89AB
    set cr [mrw $::FLASH_CR]
    if {$cr & 0x80000000} {
        error "Flash still locked after unlock: CR=0x[format %08x $cr]"
    }
    echo "Flash unlocked OK"
}

# Mass erase bank 1
proc flash_mass_erase {} {
    global FLASH_CR FLASH_SR
    set cr [mrw $FLASH_CR]
    set cr [expr {$cr & ~0xF}]
    set cr [expr {$cr | 0x04}]
    mww $FLASH_CR $cr
    set cr [mrw $FLASH_CR]
    set cr [expr {$cr | 0x10000}]
    mww $FLASH_CR $cr
    flash_wait_bsy
    mww $FLASH_CR [expr {[mrw $FLASH_CR] & ~0x04}]
    echo "Mass erase complete"
}
