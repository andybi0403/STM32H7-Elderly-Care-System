/* Minimal device header for STM32H7R3L8HxH — verified against CubeMX DIE485 config */

#ifndef __STM32H7RSxx_H
#define __STM32H7RSxx_H

#include <stdint.h>

/* ============ Core Cortex-M7 ============ */
#define __CM7_REV                0x0100U
#define __MPU_PRESENT            1U
#define __FPU_PRESENT            1U
#define __NVIC_PRIO_BITS         4U
#define __ICACHE_PRESENT         1U
#define __DCACHE_PRESENT         1U

/* ============ Memory Map (from CubeMX STM32_DIE485_620_64.xml) ============ */
#define FLASH_BASE               0x08000000UL
#define DTCM_BASE                0x20000000UL
#define ITCM_BASE                0x00000000UL
#define AXI_SRAM_BASE            0x24000000UL
#define AHB_SRAM_BASE            0x30000000UL
#define BKPSRAM_BASE             0x38800000UL
#define PERIPH_BASE              0x40000000UL

/* ============ Bus base addresses (H7RS — verified via debugger) ============ */
#define APB1_PERIPH_BASE         (PERIPH_BASE)
#define APB2_PERIPH_BASE         0x40010000UL
#define AHB1_PERIPH_BASE         0x40020000UL
#define AHB2_PERIPH_BASE         0x40030000UL
#define AHB3_PERIPH_BASE         0x40040000UL
#define AHB4_PERIPH_BASE         0x42020000UL
#define APB3_PERIPH_BASE         0x50000000UL
#define APB4_PERIPH_BASE         0x58000000UL

/* ============ GPIO (APB4 — H7RS specific!) ============ */
#define GPIOA_BASE               0x58020000UL
#define GPIOB_BASE               0x58020400UL
#define GPIOC_BASE               0x58020800UL
#define GPIOD_BASE               0x58020C00UL
#define GPIOE_BASE               0x58021000UL
#define GPIOF_BASE               0x58021400UL
#define GPIOG_BASE               0x58021800UL
#define GPIOH_BASE               0x58021C00UL
#define GPIOI_BASE               0x58022000UL
#define GPIOJ_BASE               0x58022400UL
#define GPIOK_BASE               0x58022800UL

/* ============ RCC (APB4 — H7RS specific!) ============ */
#define RCC_BASE                 0x58024400UL

/* ============ PWR ============ */
#define PWR_BASE                 0x44025000UL

/* ============ FLASH ============ */
#define FLASH_R_BASE             0x44023000UL

/* ============ NVIC ============ */
#define NVIC_BASE                0xE000E100UL
#define SCB_BASE                 0xE000ED00UL
#define SysTick_BASE             0xE000E010UL

/* ============ GPIO Register Structure ============ */
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
    volatile uint32_t BRR;
    volatile uint32_t ASCR;
} GPIO_TypeDef;

#define GPIOA  ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB  ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC  ((GPIO_TypeDef *)GPIOC_BASE)
#define GPIOD  ((GPIO_TypeDef *)GPIOD_BASE)
#define GPIOE  ((GPIO_TypeDef *)GPIOE_BASE)
#define GPIOF  ((GPIO_TypeDef *)GPIOF_BASE)
#define GPIOG  ((GPIO_TypeDef *)GPIOG_BASE)
#define GPIOH  ((GPIO_TypeDef *)GPIOH_BASE)

/* GPIO Register bit definitions */
#define GPIO_MODER_INPUT          0x0UL
#define GPIO_MODER_OUTPUT         0x1UL
#define GPIO_MODER_ALT            0x2UL
#define GPIO_MODER_ANALOG         0x3UL

#define GPIO_OTYPER_PP            0x0UL
#define GPIO_OTYPER_OD            0x1UL

#define GPIO_OSPEEDR_LOW          0x0UL
#define GPIO_OSPEEDR_MEDIUM       0x1UL
#define GPIO_OSPEEDR_HIGH         0x2UL
#define GPIO_OSPEEDR_VERY_HIGH    0x3UL

#define GPIO_PUPDR_NONE           0x0UL
#define GPIO_PUPDR_PULLUP         0x1UL
#define GPIO_PUPDR_PULLDOWN       0x2UL

#define GPIO_PIN_0                0x0001U
#define GPIO_PIN_1                0x0002U
#define GPIO_PIN_2                0x0004U
#define GPIO_PIN_3                0x0008U
#define GPIO_PIN_4                0x0010U
#define GPIO_PIN_5                0x0020U
#define GPIO_PIN_6                0x0040U
#define GPIO_PIN_7                0x0080U
#define GPIO_PIN_8                0x0100U
#define GPIO_PIN_9                0x0200U
#define GPIO_PIN_10               0x0400U
#define GPIO_PIN_11               0x0800U
#define GPIO_PIN_12               0x1000U
#define GPIO_PIN_13               0x2000U
#define GPIO_PIN_14               0x4000U
#define GPIO_PIN_15               0x8000U
#define GPIO_PIN_ALL              0xFFFFU

#define GPIO_PIN_RESET            0U
#define GPIO_PIN_SET              1U

/* ============ RCC Register Structure (simplified) ============ */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t HSICFGR;
    volatile uint32_t CRRCR;
    volatile uint32_t CSICFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CFGR2;
    volatile uint32_t CFGR3;
    volatile uint32_t RES1[3];
    volatile uint32_t HSEFCR;
    volatile uint32_t D1CFGR;
    volatile uint32_t D2CFGR;
    volatile uint32_t D1CPRE;
    volatile uint32_t D1PPRE3;
    volatile uint32_t RES2[2];
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    volatile uint32_t AHB3ENR;
    volatile uint32_t AHB4ENR;
    volatile uint32_t AHB5ENR;
    volatile uint32_t RES3[1];
    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB4ENR;
    volatile uint32_t APB5ENR;
} RCC_TypeDef;

#define RCC  ((RCC_TypeDef *)RCC_BASE)

/* RCC AHB4ENR bit definitions */
#define RCC_AHB4ENR_GPIOAEN        (1UL << 0)
#define RCC_AHB4ENR_GPIOBEN        (1UL << 1)
#define RCC_AHB4ENR_GPIOCEN        (1UL << 2)
#define RCC_AHB4ENR_GPIODEN        (1UL << 3)
#define RCC_AHB4ENR_GPIOEEN        (1UL << 4)
#define RCC_AHB4ENR_GPIOFEN        (1UL << 5)
#define RCC_AHB4ENR_GPIOGEN        (1UL << 6)
#define RCC_AHB4ENR_GPIOHEN        (1UL << 7)

/* ============ PWR Register Structure (simplified) ============ */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t CPUCR;
    volatile uint32_t RES1;
    volatile uint32_t D3CR;
    volatile uint32_t RES2;
    volatile uint32_t WKUPCR;
    volatile uint32_t WKUPFR;
    volatile uint32_t CR5;
} PWR_TypeDef;

#define PWR  ((PWR_TypeDef *)PWR_BASE)

/* PWR CR1 bit definitions */
#define PWR_CR1_VOS_SHIFT         0U
#define PWR_CR1_VOS_MASK          0x3U

/* ============ SCB ============ */
typedef struct {
    volatile uint32_t CPUID;
    volatile uint32_t ICSR;
    volatile uint32_t VTOR;
    volatile uint32_t AIRCR;
    volatile uint32_t SCR;
    volatile uint32_t CCR;
    volatile uint32_t SHPR[3];
    volatile uint32_t SHCSR;
    volatile uint32_t CFSR;
    volatile uint32_t HFSR;
    volatile uint32_t RESERVED;
    volatile uint32_t MMAR;
    volatile uint32_t BFAR;
    volatile uint32_t AFSR;
} SCB_TypeDef;

#define SCB  ((SCB_TypeDef *)SCB_BASE)
#define SCB_CPACR (*(volatile uint32_t *)(SCB_BASE + 0x88))

/* ============ SysTick ============ */
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_TypeDef;

#define SysTick  ((SysTick_TypeDef *)SysTick_BASE)

/* ============ NVIC ============ */
typedef struct {
    volatile uint32_t ISER[8];
    volatile uint32_t RES0[24];
    volatile uint32_t ICER[8];
    volatile uint32_t RES1[24];
    volatile uint32_t ISPR[8];
    volatile uint32_t RES2[24];
    volatile uint32_t ICPR[8];
    volatile uint32_t RES3[24];
    volatile uint32_t IABR[8];
    volatile uint32_t RES4[56];
    volatile uint32_t IP[240];
} NVIC_TypeDef;

#define NVIC  ((NVIC_TypeDef *)NVIC_BASE)

/* ============ Helper macros ============ */
#define __SET_BIT(reg, bit)       ((reg) |= (bit))
#define __CLEAR_BIT(reg, bit)     ((reg) &= ~(bit))

#endif /* __STM32H7RSxx_H */
