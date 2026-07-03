#include "stm32h7rsxx.h"
#include "main.h"

int main(void) {
    SystemInit();

    /* PD14 (LED0) and PC0 (LED1) as output */
    GPIOD->MODER &= ~(0x3UL << 28);
    GPIOD->MODER |= (0x1UL << 28);
    GPIOC->MODER &= ~(0x3UL << 0);
    GPIOC->MODER |= (0x1UL << 0);

    /* Both LEDs OFF (high-side: GPIO_PIN_SET = OFF) */
    GPIOD->BSRR = GPIO_PIN_14;
    GPIOC->BSRR = GPIO_PIN_0;

    while (1) {
        __asm volatile("wfi");
    }

    return 0;
}
