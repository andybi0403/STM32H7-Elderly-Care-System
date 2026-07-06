/**
 * HX711 称重模块测试 — STM32H7R3 裸机
 * ========================================
 *
 * 【接口定义】
 *   DOUT   → PB6  (输入, HX711数据输出)
 *   PD_SCK → PB7  (输出, HX711时钟)
 *   VCC    → 3.3V 或 5V (看模块, 通常3.3V兼容)
 *   GND    → GND (共地)
 *
 * 【HX711 模块接线】
 *   称重传感器 → HX711模块 (E+/E-/A+/A-)
 *   HX711模块 → 开发板 (DOUT→PB6, SCK→PB7, VCC, GND)
 *
 * 【协议】
 *   - DOUT拉低表示转换完成
 *   - 输出25个SCK脉冲读取24位数据 + 1位增益设置
 *   - 数据为二进制补码, 最高位为符号
 *
 * 【行为】
 *   每秒读取一次重量原始值, 可用于标定
 */

#include <stdint.h>

/* ===================== 寄存器地址 ===================== */
#define REG32(addr)         (*(volatile uint32_t *)(addr))

#define RCC_BASE            0x58024400UL
#define RCC_AHB4ENR         REG32(RCC_BASE + 0x140UL)   /* H7RS: +0x140 */

#define GPIOB_BASE          0x58020400UL
#define GPIOB_MODER         REG32(GPIOB_BASE + 0x00)
#define GPIOB_OTYPER        REG32(GPIOB_BASE + 0x04)
#define GPIOB_OSPEEDR       REG32(GPIOB_BASE + 0x08)
#define GPIOB_PUPDR         REG32(GPIOB_BASE + 0x0C)
#define GPIOB_IDR           REG32(GPIOB_BASE + 0x10)
#define GPIOB_BSRR          REG32(GPIOB_BASE + 0x18)
#define GPIOB_BRR           REG32(GPIOB_BASE + 0x28)

#define SysTick_BASE        0xE000E010UL
#define SysTick_CTRL        REG32(SysTick_BASE + 0x00)
#define SysTick_LOAD        REG32(SysTick_BASE + 0x04)
#define SysTick_VAL         REG32(SysTick_BASE + 0x08)

/* ===================== 引脚定义 ===================== */
#define HX711_DOUT_PIN      0x0040U                     /* PB6 */
#define HX711_SCK_PIN       0x0080U                     /* PB7 */

/* ===================== 延时 ===================== */

static void delay_us(uint32_t us) {
    for (volatile uint32_t i = 0; i < us * 10; i++)
        __asm__ volatile ("nop");
}

static void delay_ms(uint32_t ms) {
    SysTick_LOAD = 63999;
    SysTick_VAL  = 0;
    SysTick_CTRL = 5;
    while (ms--) { while (!(SysTick_CTRL & (1 << 16))) {} }
    SysTick_CTRL = 0;
}

/* ===================== GPIO 初始化 ===================== */
static void hx711_gpio_init(void) {
    RCC_AHB4ENR |= (1 << 1);                            /* GPIOB时钟 */
    (void)RCC_AHB4ENR;

    /* PB6 (DOUT): 输入, 上拉 */
    GPIOB_MODER &= ~(3U << 12);                         /* 00=输入 */
    GPIOB_PUPDR = (GPIOB_PUPDR & ~(3U << 12)) | (1U << 12); /* 上拉 */

    /* PB7 (SCK): 输出, 推挽, 初始低 */
    GPIOB_MODER   = (GPIOB_MODER & ~(3U << 14)) | (1U << 14); /* 输出 */
    GPIOB_OTYPER &= ~HX711_SCK_PIN;
    GPIOB_OSPEEDR = (GPIOB_OSPEEDR & ~(3U << 14)) | (2U << 14);
    GPIOB_PUPDR  &= ~(3U << 14);
    GPIOB_BRR     = HX711_SCK_PIN;                      /* SCK LOW */
}

/* ===================== HX711 读取 ===================== */

/** 读取一次HX711原始值, 返回24位有符号整数 */
static int32_t hx711_read(void) {
    /* 等待DOUT变低 (转换完成) */
    uint32_t timeout = 0;
    while (GPIOB_IDR & HX711_DOUT_PIN) {
        if (++timeout > 1000000) return 0x80000000;     /* 超时 */
    }

    int32_t val = 0;

    /* 读取24个数据位, MSB first */
    for (int i = 0; i < 24; i++) {
        GPIOB_BSRR = HX711_SCK_PIN;                     /* SCK HIGH */
        delay_us(1);                                    /* >0.2us */

        val = (val << 1) | ((GPIOB_IDR & HX711_DOUT_PIN) ? 1 : 0);

        GPIOB_BRR = HX711_SCK_PIN;                      /* SCK LOW */
        delay_us(1);                                    /* >0.2us */
    }

    /* 第25个脉冲: 设置下次增益 (1=chA_128, 3=chA_64) */
    GPIOB_BSRR = HX711_SCK_PIN;
    delay_us(1);
    GPIOB_BRR  = HX711_SCK_PIN;

    /* 符号扩展: 24位 → 32位 */
    if (val & 0x800000) val |= 0xFF000000;

    return val;
}

/* ===================== 数据缓冲 (SRAM, 调试器可读) ===================== */
static volatile int32_t g_raw_value = 0;        /* 最新原始值 */
static volatile int32_t g_offset     = 0;        /* 零点 */
static volatile int32_t g_net_value = 0;        /* 去零偏后的值 */
static volatile uint32_t g_error = 0;           /* 错误计数 */

/* ===================== 主程序 ===================== */

int main(void) {
    hx711_gpio_init();

    delay_ms(100);

    g_offset = hx711_read();
    if (g_offset == 0x80000000) g_offset = 0;

    while (1) {
        int32_t raw = hx711_read();

        if (raw == 0x80000000) {
            g_error++;
        } else {
            g_raw_value = raw;
            g_net_value = raw - g_offset;
        }

        delay_ms(200);
    }
    return 0;
}
