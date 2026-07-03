#include "stm32h7rsxx.h"

/* Cortex-M7 HardFault handler — trap for debugging */
void HardFault_Handler(void) {
    while (1) {
        __asm volatile("nop");
    }
}

/* SysTick handler (if needed later) */
void SysTick_Handler(void) {
    /* placeholder */
}
