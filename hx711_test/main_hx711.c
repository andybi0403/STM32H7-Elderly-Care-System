/**
 * HX711 称重模块 + LCD 显示 — STM32H7R3
 * =========================================
 *
 * 【接口定义】
 *   HX711 DOUT  → PB6  (数据输入, 上拉)
 *   HX711 SCK   → PB7  (时钟输出, 推挽)
 *   HX711 VCC   → 3.3V (可从DAPLink取电)
 *   HX711 GND   → GND  (共地)
 *
 *   称重传感器 → HX711绿端子 (E+/E-/A+/A-)
 *   LCD屏幕    → 开发板板载 (GPIO模拟16位并口)
 *
 * 【依赖】
 *   lcd_ui.c/h — 原项目LCD驱动 (GPIO位带并行总线 + ILI9341)
 *   stm32h7rsxx.h — H7RS寄存器定义
 *   STM32H7R3L8_SRAM.ld — SRAM运行链接脚本
 *
 * 【H7RS关键注意事项】
 *   - RCC_AHB4ENR 在 +0x140 (非标准H7的 +0x50!)
 *   - SRAM运行需手动清零BSS段
 *   - Flash烧录暂不支持 (芯片安全限制)
 */
#include <stdint.h>
#include "stm32h7rsxx.h"
#include "lcd_ui.h"

/* ════════════════ HX711 底层 (直接寄存器, 已验证) ════════════════ */
#define REG32(a)            (*(volatile uint32_t *)(a))
#define GPIOB_IDR_HX        REG32(0x58020400 + 0x10)
#define GPIOB_BSRR_HX       REG32(0x58020400 + 0x18)
#define GPIOB_BRR_HX        REG32(0x58020400 + 0x28)
#define GPIOB_MODER_HX      REG32(0x58020400 + 0x00)
#define GPIOB_OTYPER_HX     REG32(0x58020400 + 0x04)
#define GPIOB_OSPEEDR_HX    REG32(0x58020400 + 0x08)
#define GPIOB_PUPDR_HX      REG32(0x58020400 + 0x0C)

static int32_t hx711_read(void) {
    uint32_t t = 0;
    while (GPIOB_IDR_HX & GPIO_PIN_6)
        if (++t > 2000000) return 0x80000000;       /* 超时 */
    int32_t v = 0;
    for (int i = 0; i < 24; i++) {
        GPIOB_BSRR_HX = GPIO_PIN_7;                  /* SCK↑ */
        for (volatile int d = 0; d < 10; d++) __asm__("nop");
        v = (v << 1) | ((GPIOB_IDR_HX & GPIO_PIN_6) ? 1 : 0);
        GPIOB_BRR_HX = GPIO_PIN_7;                   /* SCK↓ */
        for (volatile int d = 0; d < 10; d++) __asm__("nop");
    }
    GPIOB_BSRR_HX = GPIO_PIN_7;                      /* 第25脉冲: 增益=128 */
    for (volatile int d = 0; d < 10; d++) __asm__("nop");
    GPIOB_BRR_HX = GPIO_PIN_7;
    if (v & 0x800000) v |= 0xFF000000;               /* 符号扩展24→32位 */
    return v;
}

static void hx711_gpio_init(void) {
    /* GPIOB时钟已由lcd_bus_init()通过正确地址(+0x140)开启 */
    GPIOB_MODER_HX &= ~(3U << 12);                                     /* PB6 输入 */
    GPIOB_PUPDR_HX  = (GPIOB_PUPDR_HX & ~(3U << 12)) | (1U << 12);   /* 上拉 */
    GPIOB_MODER_HX  = (GPIOB_MODER_HX & ~(3U << 14)) | (1U << 14);   /* PB7 输出 */
    GPIOB_OTYPER_HX &= ~GPIO_PIN_7;                                    /* 推挽 */
    GPIOB_OSPEEDR_HX = (GPIOB_OSPEEDR_HX & ~(3U << 14)) | (2U << 14);/* 高速 */
    GPIOB_PUPDR_HX  &= ~(3U << 14);                                    /* 无上下拉 */
    GPIOB_BRR_HX     = GPIO_PIN_7;                                      /* 初始低 */
}

/* ════════════════ SysTick轮询延时 (不产生中断) ════════════════ */
static void simple_delay_ms(uint32_t ms) {
    SysTick->LOAD = 63999;
    SysTick->VAL  = 0;
    SysTick->CTRL = 5;
    while (ms--) { while (!(SysTick->CTRL & (1 << 16))) {} }
    SysTick->CTRL = 0;
}

/* ════════════════ Main ════════════════ */
int main(void) {
    /* 手动清零BSS (SRAM运行必须, Flash启动由启动码自动完成) */
    extern uint32_t __bss_start__, __bss_end__;
    for (uint32_t *p = &__bss_start__; p < &__bss_end__; p++) *p = 0;

    /* FPU */
    SCB_CPACR |= (3UL << 20) | (3UL << 22);

    /* 初始化LCD — 会正确开启所有GPIO时钟 (含GPIOB) */
    UI_Init();

    /* 初始化HX711 — 必须在LCD之后, PB6/PB7配置依赖GPIOB时钟已开 */
    hx711_gpio_init();
    simple_delay_ms(100);

    /* 零点标定 */
    int32_t offset = hx711_read();
    if (offset == 0x80000000) offset = 0;

    /* 切换到药品页面 — 显示 PILL WEIGHT */
    UI_SetPage(2);

    UI_Data_t ui = {0};
    uint16_t last_weight = 0xFFFF;

    while (1) {
        int32_t raw = hx711_read();
        if (raw != 0x80000000) {
            int32_t net = raw - offset;
            uint16_t w = (uint16_t)((net < 0) ? -net : net);
            ui.pill_weight_g = w;

            /* 仅重量变化时刷新屏幕, 避免闪烁 */
            if (w != last_weight) {
                last_weight = w;
                UI_Update(&ui);
                UI_Redraw();
            }
        }
        simple_delay_ms(150);
    }
    return 0;
}
