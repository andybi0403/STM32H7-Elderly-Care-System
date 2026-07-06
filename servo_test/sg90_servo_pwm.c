/**
 * SG90/MG90S 舵机测试 — STM32H7R3 裸机
 * ========================================
 *
 * 【接口定义】
 *   信号引脚:  PA6  (GPIOA Pin 6, 开发板P2排针)
 *   电源:      5V 独立供电 (舵机红线)
 *   地:        GND 共地 (舵机棕线接电源负极 + 开发板GND)
 *
 * 【PWM参数】  软件模拟, 50Hz (20ms周期)
 *   0°   →  700us 脉冲
 *   120° → 1633us 脉冲
 *   180° → 2100us 脉冲 (理论值, 本测试未使用)
 *
 * 【H7RS关键寄存器】 (不同于标准H7!)
 *   RCC_AHB4ENR = RCC_BASE + 0x140 = 0x58024540
 *   GPIOA_BASE   = 0x58020000
 *
 * 【行为】 0° → 120° 来回循环摆动
 */

#include <stdint.h>

/* ===================== 寄存器地址 (H7RS专用) ===================== */
#define REG32(addr)         (*(volatile uint32_t *)(addr))

#define RCC_BASE            0x58024400UL
#define RCC_AHB4ENR         REG32(RCC_BASE + 0x140UL)   /* H7RS: +0x140 */

#define GPIOA_BASE          0x58020000UL
#define GPIOA_MODER         REG32(GPIOA_BASE + 0x00)    /* 模式 */
#define GPIOA_OTYPER        REG32(GPIOA_BASE + 0x04)    /* 输出类型 */
#define GPIOA_OSPEEDR       REG32(GPIOA_BASE + 0x08)    /* 速度 */
#define GPIOA_PUPDR         REG32(GPIOA_BASE + 0x0C)    /* 上下拉 */
#define GPIOA_BSRR          REG32(GPIOA_BASE + 0x18)    /* 置位 */
#define GPIOA_BRR           REG32(GPIOA_BASE + 0x28)    /* 复位 */

#define SysTick_BASE        0xE000E010UL
#define SysTick_CTRL        REG32(SysTick_BASE + 0x00)
#define SysTick_LOAD        REG32(SysTick_BASE + 0x04)
#define SysTick_VAL         REG32(SysTick_BASE + 0x08)

/* ===================== 引脚宏 ===================== */
#define SERVO_PIN           0x0040U                    /* PA6 */
#define SERVO_PORT_BASE     GPIOA_BASE

/* ===================== 舵机参数 ===================== */
#define PWM_FREQ_HZ         50                          /* PWM频率 */
#define PWM_PERIOD_US       20000                       /* 周期 20ms */
#define PULSE_MIN_US        700                         /* 0° 脉宽 */
#define PULSE_MAX_DEG_US    2100                        /* 180° 理论脉宽 */
#define SWEEP_ANGLE_MAX     120                         /* 最大摆动角度 */

/* ===================== 延时函数 ===================== */

static void delay_us(uint32_t us) {
    for (volatile uint32_t i = 0; i < us * 10; i++)
        __asm__ volatile ("nop");
}

static void delay_ms(uint32_t ms) {
    SysTick_LOAD = 63999;                               /* 64MHz / 1000 */
    SysTick_VAL  = 0;
    SysTick_CTRL = 5;                                   /* 轮询模式 */
    while (ms--) {
        while (!(SysTick_CTRL & (1 << 16))) {}
    }
    SysTick_CTRL = 0;
}

/* ===================== GPIO初始化 ===================== */
static void servo_gpio_init(void) {
    RCC_AHB4ENR |= 1;                                  /* GPIOA时钟 */
    (void)RCC_AHB4ENR;                                  /* 确保写入生效 */

    GPIOA_MODER   = (GPIOA_MODER & ~(3U << 12)) | (1U << 12);   /* PA6→输出 */
    GPIOA_OTYPER &= ~SERVO_PIN;                         /* 推挽 */
    GPIOA_OSPEEDR = (GPIOA_OSPEEDR & ~(3U << 12)) | (2U << 12); /* 高速 */
    GPIOA_PUPDR  &= ~(3U << 12);                        /* 无上下拉 */
    GPIOA_BRR     = SERVO_PIN;                          /* 初始低电平 */
}

/* ===================== 舵机PWM ===================== */

/** 设置舵机角度 (0 ~ SWEEP_ANGLE_MAX) */
static void servo_set_angle(uint32_t angle_deg) {
    if (angle_deg > SWEEP_ANGLE_MAX) angle_deg = SWEEP_ANGLE_MAX;

    uint32_t pulse_us = PULSE_MIN_US + (angle_deg * (PULSE_MAX_DEG_US - PULSE_MIN_US)) / 180;

    GPIOA_BSRR = SERVO_PIN;                             /* 高电平 */
    delay_us(pulse_us);
    GPIOA_BRR  = SERVO_PIN;                             /* 低电平 */
    delay_us(PWM_PERIOD_US - pulse_us);                 /* 补足20ms周期 */
}

/* ===================== 主程序 ===================== */

extern void SystemInit(void);
void SystemInit(void) {
    REG32(0xE000ED88) |= (3U << 20) | (3U << 22);      /* 开启FPU */
}

int main(void) {
    servo_gpio_init();

    while (1) {
        /* 0° → 120° */
        for (int a = 0; a <= SWEEP_ANGLE_MAX; a += 2) {
            servo_set_angle(a);
            delay_ms(15);
        }
        delay_ms(500);

        /* 120° → 0° */
        for (int a = SWEEP_ANGLE_MAX; a >= 0; a -= 2) {
            servo_set_angle(a);
            delay_ms(15);
        }
        delay_ms(500);
    }
    return 0;
}
