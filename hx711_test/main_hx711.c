/**
 * 三路HX711称重 + LCD显示 — STM32H7R3
 * ========================================
 * 传感器1: PB6/PB7 | 传感器2: PB8/PB9 | 传感器3: PC2/PC3
 * VCC→DAPLink 3.3V, GND→共地
 * 屏幕显示: W1/W2/W3
 */

#include <stdint.h>
#include "stm32h7rsxx.h"
#include "lcd_ui.h"

/* ════════════════ 寄存器 ════════════════ */
#define REG32(a)          (*(volatile uint32_t *)(a))
#define GPIOB_IDR_HX      REG32(0x58020400 + 0x10)
#define GPIOB_BSRR_HX     REG32(0x58020400 + 0x18)
#define GPIOB_BRR_HX      REG32(0x58020400 + 0x28)
#define GPIOB_MODER_HX    REG32(0x58020400 + 0x00)
#define GPIOB_OTYPER_HX   REG32(0x58020400 + 0x04)
#define GPIOB_OSPEEDR_HX  REG32(0x58020400 + 0x08)
#define GPIOB_PUPDR_HX    REG32(0x58020400 + 0x0C)
#define GPIOC_IDR_HX      REG32(0x58020800 + 0x10)
#define GPIOC_BSRR_HX     REG32(0x58020800 + 0x18)
#define GPIOC_BRR_HX      REG32(0x58020800 + 0x28)
#define GPIOC_MODER_HX    REG32(0x58020800 + 0x00)
#define GPIOC_OTYPER_HX   REG32(0x58020800 + 0x04)
#define GPIOC_OSPEEDR_HX  REG32(0x58020800 + 0x08)
#define GPIOC_PUPDR_HX    REG32(0x58020800 + 0x0C)

#define HX1_DOUT 6
#define HX1_SCK  7
#define HX2_DOUT 8
#define HX2_SCK  9
#define HX3_DOUT 4
#define HX3_SCK  7

/* ════════════════ 调试变量 ════════════════ */
static volatile int32_t dbg_r1, dbg_r2, dbg_r3;
static volatile int32_t dbg_o1, dbg_o2, dbg_o3;

/* ════════════════ HX711读取 ════════════════ */
static int32_t hx711_read(int port, int dout, int sck) {
    uint32_t dm = 1U << dout, sm = 1U << sck;
    uint32_t idr_a, bsr_a, br_a;
    if      (port == 0) { idr_a=0x58020410; bsr_a=0x58020418; br_a=0x58020428; } /* GPIOB */
    else if (port == 1) { idr_a=0x58020010; bsr_a=0x58020018; br_a=0x58020028; } /* GPIOA */

    uint32_t t = 0;
    while (REG32(idr_a) & dm)
        if (++t > 2000000) return 0x80000000;

    int32_t v = 0;
    for (int i = 0; i < 24; i++) {
        REG32(bsr_a) = sm;
        for (volatile int d = 0; d < 10; d++) __asm__("nop");
        v = (v << 1) | ((REG32(idr_a) & dm) ? 1 : 0);
        REG32(br_a) = sm;
        for (volatile int d = 0; d < 10; d++) __asm__("nop");
    }
    REG32(bsr_a) = sm;
    for (volatile int d = 0; d < 10; d++) __asm__("nop");
    REG32(br_a) = sm;

    if (v & 0x800000) v |= 0xFF000000;
    return v;
}

/* ════════════════ GPIO初始化 ════════════════ */
static void hx711_gpio_init(void) {
    /* GPIOB/PC时钟已由lcd_bus_init开启 */

    /* 传感器1: PB6输入上拉, PB7输出 */
    GPIOB_MODER_HX = (GPIOB_MODER_HX & ~(3U<<12)) | (0U<<12);
    GPIOB_PUPDR_HX = (GPIOB_PUPDR_HX & ~(3U<<12)) | (1U<<12);
    GPIOB_MODER_HX = (GPIOB_MODER_HX & ~(3U<<14)) | (1U<<14);
    GPIOB_OTYPER_HX &= ~(1U<<7);
    GPIOB_OSPEEDR_HX = (GPIOB_OSPEEDR_HX & ~(3U<<14)) | (2U<<14);
    GPIOB_PUPDR_HX &= ~(3U<<14);

    /* 传感器2: PB8输入上拉, PB9输出 */
    GPIOB_MODER_HX = (GPIOB_MODER_HX & ~(3U<<16)) | (0U<<16);
    GPIOB_PUPDR_HX = (GPIOB_PUPDR_HX & ~(3U<<16)) | (1U<<16);
    GPIOB_MODER_HX = (GPIOB_MODER_HX & ~(3U<<18)) | (1U<<18);
    GPIOB_OTYPER_HX &= ~(1U<<9);
    GPIOB_OSPEEDR_HX = (GPIOB_OSPEEDR_HX & ~(3U<<18)) | (2U<<18);
    GPIOB_PUPDR_HX &= ~(3U<<18);

    /* 传感器3: PA4输入上拉, PA7输出 */
    GPIOA->MODER   = (GPIOA->MODER   & ~(3U<<8))  | (0U<<8);
    GPIOA->PUPDR   = (GPIOA->PUPDR   & ~(3U<<8))  | (1U<<8);
    GPIOA->MODER   = (GPIOA->MODER   & ~(3U<<14)) | (1U<<14);
    GPIOA->OTYPER &= ~(1U<<7);
    GPIOA->OSPEEDR = (GPIOA->OSPEEDR & ~(3U<<14)) | (2U<<14);
    GPIOA->PUPDR  &= ~(3U<<14);

    GPIOB_BRR_HX = (1U<<7) | (1U<<9);
    GPIOA->BRR    = (1U<<7);
}

/* ════════════════ 延时 ════════════════ */
static void delay_ms(uint32_t ms) {
    SysTick->LOAD = 63999; SysTick->VAL = 0; SysTick->CTRL = 5;
    while (ms--) { while (!(SysTick->CTRL & (1 << 16))) {} }
    SysTick->CTRL = 0;
}

/* ════════════════ Main ════════════════ */
int main(void) {
    extern uint32_t __bss_start__, __bss_end__;
    for (uint32_t *p = &__bss_start__; p < &__bss_end__; p++) *p = 0;

    SCB_CPACR |= (3UL << 20) | (3UL << 22);

    UI_Init();
    hx711_gpio_init();
    delay_ms(1000);

    /* 丢弃不稳定读数 */
    for (int s = 0; s < 10; s++) {
        hx711_read(0, HX1_DOUT, HX1_SCK);
        hx711_read(0, HX2_DOUT, HX2_SCK);
        hx711_read(1, HX3_DOUT, HX3_SCK);
        delay_ms(30);
    }

    /* 零点标定: 10次平均 */
    int32_t off1 = 0, off2 = 0, off3 = 0;
    for (int i = 0; i < 10; i++) {
        int32_t r1 = hx711_read(0, HX1_DOUT, HX1_SCK);
        int32_t r2 = hx711_read(0, HX2_DOUT, HX2_SCK);
        int32_t r3 = hx711_read(1, HX3_DOUT, HX3_SCK);
        if (r1 != 0x80000000) off1 += r1;
        if (r2 != 0x80000000) off2 += r2;
        if (r3 != 0x80000000) off3 += r3;
        delay_ms(50);
    }
    off1 /= 10; off2 /= 10; off3 /= 10;
    dbg_o1 = off1; dbg_o2 = off2; dbg_o3 = off3;

    UI_SetPage(2);
    UI_Data_t ui = {0};
    uint16_t lw1 = 0xFFFF, lw2 = 0xFFFF, lw3 = 0xFFFF;

    while (1) {
        int32_t r1 = hx711_read(0, HX1_DOUT, HX1_SCK);
        int32_t r2 = hx711_read(0, HX2_DOUT, HX2_SCK);
        int32_t r3 = hx711_read(1, HX3_DOUT, HX3_SCK);
        dbg_r1 = r1; dbg_r2 = r2; dbg_r3 = r3;

        uint16_t w1 = 0, w2 = 0, w3 = 0;
        if (r1 != 0x80000000) w1 = (uint16_t)((r1 > off1) ? (r1 - off1) : (off1 - r1));
        if (r2 != 0x80000000) w2 = (uint16_t)((r2 > off2) ? (r2 - off2) : (off2 - r2));
        if (r3 != 0x80000000) w3 = (uint16_t)((r3 > off3) ? (r3 - off3) : (off3 - r3));

        ui.pill_weight_g   = w1;
        ui.temperature_c10 = w2;
        ui.pitch_deg       = w3;

        if (w1 != lw1 || w2 != lw2 || w3 != lw3) {
            lw1 = w1; lw2 = w2; lw3 = w3;
            UI_Update(&ui);
            UI_Redraw();
        }

        delay_ms(100);
    }
    return 0;
}
