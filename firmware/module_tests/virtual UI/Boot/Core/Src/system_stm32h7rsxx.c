#include "stm32h7rsxx.h"

/* SystemCoreClock: default HSI = 64MHz */
uint32_t SystemCoreClock = 64000000U;

/* Minimal SystemInit — enable GPIO clocks via RCC AHB4ENR */
void SystemInit(void)
{
    /* Enable PWR clock (needed for H7 voltage scaling) */
    /* Enable FLASH, GPIO, RCC clocks are already enabled at reset */

    /* For H7RS, GPIO clocks need to be explicitly enabled */
    /* We'll do this in MSP init, not here in SystemInit */

    /* Enable FPU */
    SCB_CPACR |= ((3UL << 20) | (3UL << 22));
    __asm volatile("dsb");
    __asm volatile("isb");
}
