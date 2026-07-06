/**
 * Minimal PA6 blink test — confirms GPIO+SysTick work before servo PWM
 * PA6 = 500ms ON, 500ms OFF (visible with LED or multimeter)
 */
#include "stm32h7rsxx.h"

static volatile uint32_t tick = 0;
void SysTick_Handler(void) { tick++; }
static void delay_ms(uint32_t ms) {
    uint32_t start = tick;
    while ((tick - start) < ms) { __asm__("wfi"); }
}

int main(void) {
    /* VTOR → SRAM */
    SCB->VTOR = 0x20000000;
    __asm__ volatile ("dsb; isb");

    /* SysTick: 1ms (64MHz / 1000) */
    SysTick->LOAD = 63999;
    SysTick->VAL  = 0;
    SysTick->CTRL = 7;

    /* GPIOA clock */
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN;
    __asm__ volatile ("dsb");

    /* PA6: output, push-pull */
    uint32_t s = 6 * 2;
    GPIOA->MODER   = (GPIOA->MODER & ~(3U << s)) | (1U << s);
    GPIOA->OTYPER &= ~GPIO_PIN_6;
    GPIOA->OSPEEDR = (GPIOA->OSPEEDR & ~(3U << s)) | (2U << s);
    GPIOA->PUPDR  &= ~(3U << s);

    while (1) {
        GPIOA->BSRR = GPIO_PIN_6;   /* PA6 = HIGH */
        delay_ms(500);
        GPIOA->BRR  = GPIO_PIN_6;   /* PA6 = LOW */
        delay_ms(500);
    }
    return 0;
}
