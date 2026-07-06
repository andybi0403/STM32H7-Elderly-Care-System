#include "lcd_ui.h"
#include "stm32h7rsxx.h"
#include <stdint.h>

#if defined(__GNUC__)
#define LCD_UI_UNUSED __attribute__((unused))
#else
#define LCD_UI_UNUSED
#endif

#define REG32(addr) (*(volatile uint32_t *)(addr))

#define RCC_AHB5ENR_REG REG32(RCC_BASE + 0x134UL)
#define RCC_AHB4ENR_REG REG32(RCC_BASE + 0x140UL)
#define RCC_AHB5ENR_FMCEN (1UL << 4)

#define MPU_TYPE_REG REG32(0xE000ED90UL)
#define MPU_CTRL_REG REG32(0xE000ED94UL)
#define MPU_RNR_REG  REG32(0xE000ED98UL)
#define MPU_RBAR_REG REG32(0xE000ED9CUL)
#define MPU_RASR_REG REG32(0xE000EDA0UL)
#define SCB_SHCSR_REG REG32(0xE000ED24UL)

#define MPU_CTRL_ENABLE     (1UL << 0)
#define MPU_CTRL_PRIVDEFENA (1UL << 2)
#define MPU_RASR_XN         (1UL << 28)
#define MPU_RASR_AP_FULL    (3UL << 24)
#define MPU_RASR_S          (1UL << 18)
#define MPU_RASR_SIZE_64MB  (25UL << 1)
#define MPU_RASR_ENABLE     (1UL << 0)

#define FMC_Bank1_R_BASE  0x52004000UL
#define FMC_Bank1E_R_BASE 0x52004104UL

#define FMC_BCR1_FMCEN (1UL << 31)
#define FMC_BCRx_MBKEN (1UL << 0)
#define FMC_BCRx_MWID_16BIT (1UL << 4)
#define FMC_BCRx_WREN  (1UL << 12)
#define FMC_BCRx_EXTMOD (1UL << 14)
#define FMC_BCR1_CCLKEN (1UL << 20)

typedef struct {
    volatile uint32_t BTCR[8];
} FMC_Bank1_TypeDef;

typedef struct {
    volatile uint32_t BWTR[7];
} FMC_Bank1E_TypeDef;

#define FMC_Bank1_R  ((FMC_Bank1_TypeDef *)FMC_Bank1_R_BASE)
#define FMC_Bank1E_R ((FMC_Bank1E_TypeDef *)FMC_Bank1E_R_BASE)

typedef struct {
    volatile uint16_t LCD_REG;
    volatile uint16_t LCD_RAM;
} LCD_TypeDef;

#define LCD ((LCD_TypeDef *)0x6000FFFEUL)

#define LCD_W 320U
#define LCD_H 240U

#define LCD_CTRL_CS GPIO_PIN_12
#define LCD_CTRL_RD GPIO_PIN_5
#define LCD_CTRL_WR GPIO_PIN_2
#define LCD_CTRL_RS GPIO_PIN_10

#define TOUCH_PEN GPIO_PIN_4
#define TOUCH_SCK GPIO_PIN_12
#define TOUCH_MISO GPIO_PIN_13
#define TOUCH_MOSI GPIO_PIN_14
#define TOUCH_CS GPIO_PIN_15

#define LCD_DATA_MASK_A (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_8 | \
                         GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15)
#define LCD_DATA_MASK_B (GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_10 | \
                         GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13)

#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_BG      0xEF7D
#define COLOR_PANEL   0xFFFF
#define COLOR_HEADER  0x02B2
#define COLOR_BLUE    0x049F
#define COLOR_GREEN   0x07E0
#define COLOR_RED     0xF800
#define COLOR_YELLOW  0xFFE0
#define COLOR_GRAY    0x8410
#define COLOR_DARK    0x2104
#define COLOR_NAV     0x3186
#define COLOR_ORANGE  0xFC80

#define TOUCH_RAW_MIN 200U
#define TOUCH_RAW_MAX 3900U
#define TOUCH_SWAP_XY 1U
#define TOUCH_INV_X   1U
#define TOUCH_INV_Y   0U

static uint16_t g_lcd_id;
static UI_Data_t g_ui_data = {
    72, 98, 365, 2, -1, 0, 1, 126, 0, 0, 1
};
static uint8_t g_current_page;
static uint8_t g_last_pressed;
static uint16_t g_last_x;
static uint16_t g_last_y;
static uint8_t g_needs_redraw = 1;

static void LCD_UI_UNUSED lcd_mpu_init(void)
{
    if ((MPU_TYPE_REG & 0xFFUL) == 0U) {
        return;
    }

    __asm volatile("dsb");
    __asm volatile("isb");
    MPU_CTRL_REG = 0;

    MPU_RNR_REG = 0;
    MPU_RBAR_REG = 0x60000000UL;
    MPU_RASR_REG = MPU_RASR_XN | MPU_RASR_AP_FULL | MPU_RASR_S |
                   MPU_RASR_SIZE_64MB | MPU_RASR_ENABLE;

    SCB_SHCSR_REG |= (1UL << 16);
    MPU_CTRL_REG = MPU_CTRL_PRIVDEFENA | MPU_CTRL_ENABLE;
    __asm volatile("dsb");
    __asm volatile("isb");
}

static void delay_cycles(volatile uint32_t cycles)
{
    while (cycles--) {
        __asm volatile("nop");
    }
}

static void delay_ms(uint32_t ms)
{
    while (ms--) {
        delay_cycles(12000);
    }
}

static void LCD_UI_UNUSED gpio_pin_af(GPIO_TypeDef *gpio, uint32_t pin, uint32_t af)
{
    uint32_t shift = pin * 2U;
    gpio->MODER = (gpio->MODER & ~(3UL << shift)) | (GPIO_MODER_ALT << shift);
    gpio->OTYPER &= ~(1UL << pin);
    gpio->OSPEEDR = (gpio->OSPEEDR & ~(3UL << shift)) | (GPIO_OSPEEDR_VERY_HIGH << shift);
    gpio->PUPDR = (gpio->PUPDR & ~(3UL << shift)) | (GPIO_PUPDR_PULLUP << shift);

    uint32_t afr_index = pin >> 3U;
    uint32_t afr_shift = (pin & 7U) * 4U;
    gpio->AFR[afr_index] = (gpio->AFR[afr_index] & ~(0xFUL << afr_shift)) | (af << afr_shift);
}

static void gpio_pin_output(GPIO_TypeDef *gpio, uint32_t pin)
{
    uint32_t shift = pin * 2U;
    gpio->MODER = (gpio->MODER & ~(3UL << shift)) | (GPIO_MODER_OUTPUT << shift);
    gpio->OTYPER &= ~(1UL << pin);
    gpio->OSPEEDR = (gpio->OSPEEDR & ~(3UL << shift)) | (GPIO_OSPEEDR_HIGH << shift);
    gpio->PUPDR &= ~(3UL << shift);
}

static void gpio_pin_input_pullup(GPIO_TypeDef *gpio, uint32_t pin)
{
    uint32_t shift = pin * 2U;
    gpio->MODER &= ~(3UL << shift);
    gpio->PUPDR = (gpio->PUPDR & ~(3UL << shift)) | (GPIO_PUPDR_PULLUP << shift);
}

static void lcd_bus_init(void)
{
    RCC_AHB4ENR_REG |= RCC_AHB4ENR_GPIOAEN | RCC_AHB4ENR_GPIOBEN |
                       RCC_AHB4ENR_GPIOCEN | RCC_AHB4ENR_GPIODEN |
                       RCC_AHB4ENR_GPIOEEN;
    (void)RCC_AHB4ENR_REG;

    gpio_pin_output(GPIOA, 0);
    gpio_pin_output(GPIOA, 1);
    gpio_pin_output(GPIOA, 2);
    gpio_pin_output(GPIOA, 5);
    gpio_pin_output(GPIOA, 8);
    gpio_pin_output(GPIOA, 9);
    gpio_pin_output(GPIOA, 10);
    gpio_pin_output(GPIOA, 11);
    gpio_pin_output(GPIOA, 12);
    gpio_pin_output(GPIOA, 15);

    gpio_pin_output(GPIOB, 2);
    gpio_pin_output(GPIOB, 3);
    gpio_pin_output(GPIOB, 4);
    gpio_pin_output(GPIOB, 5);
    gpio_pin_output(GPIOB, 10);
    gpio_pin_output(GPIOB, 11);
    gpio_pin_output(GPIOB, 12);
    gpio_pin_output(GPIOB, 13);

    gpio_pin_output(GPIOD, 12);
    gpio_pin_output(GPIOE, 10);
    gpio_pin_output(GPIOD, 15);
    gpio_pin_output(GPIOD, 14);
    gpio_pin_output(GPIOC, 0);

    gpio_pin_input_pullup(GPIOD, 4);
    gpio_pin_output(GPIOE, 12);
    gpio_pin_input_pullup(GPIOE, 13);
    gpio_pin_output(GPIOE, 14);
    gpio_pin_output(GPIOE, 15);

    GPIOD->BSRR = GPIO_PIN_15 | LCD_CTRL_CS;
    GPIOA->BSRR = LCD_CTRL_RD;
    GPIOB->BSRR = LCD_CTRL_WR;
    GPIOE->BSRR = LCD_CTRL_RS;
    GPIOD->BSRR = (uint32_t)LCD_CTRL_CS << 16;
    GPIOE->BSRR = TOUCH_CS;
    GPIOE->BSRR = (uint32_t)TOUCH_SCK << 16;
    GPIOE->BSRR = (uint32_t)TOUCH_MOSI << 16;
}

static void lcd_gpio_write_bus(uint16_t data)
{
    uint32_t porta =
        ((data & 0x0001U) << 12) |
        ((data & 0x0002U) << 10) |
        ((data & 0x0004U) << 8) |
        ((data & 0x0008U) << 6) |
        ((data & 0x0010U) << 4) |
        ((data & 0x0020U) >> 3) |
        ((data & 0x0040U) >> 5) |
        ((data & 0x0080U) >> 7) |
        ((data & 0x8000U) << 0);
    uint32_t portb =
        ((data & 0x0100U) << 5) |
        ((data & 0x0200U) << 3) |
        ((data & 0x0400U) << 1) |
        ((data & 0x0800U) >> 1) |
        ((data & 0x1000U) >> 7) |
        ((data & 0x2000U) >> 9) |
        ((data & 0x4000U) >> 11);

    GPIOA->ODR = (GPIOA->ODR & ~LCD_DATA_MASK_A) | porta;
    GPIOB->ODR = (GPIOB->ODR & ~LCD_DATA_MASK_B) | portb;
}

static void lcd_gpio_write(uint16_t value, uint8_t is_data)
{
    if (is_data) {
        GPIOE->BSRR = LCD_CTRL_RS;
    } else {
        GPIOE->BSRR = (uint32_t)LCD_CTRL_RS << 16;
    }

    lcd_gpio_write_bus(value);
    delay_cycles(20);
    GPIOB->BSRR = (uint32_t)LCD_CTRL_WR << 16;
    delay_cycles(20);
    GPIOB->BSRR = LCD_CTRL_WR;
    delay_cycles(20);
}

static void lcd_write_cmd(uint16_t cmd)
{
    lcd_gpio_write(cmd, 0);
}

static void lcd_write_data(uint16_t data)
{
    lcd_gpio_write(data, 1);
}

static uint16_t lcd_read_data(void)
{
    return 0xFFFF;
}

static void lcd_write_reg(uint16_t cmd, uint16_t data)
{
    lcd_write_cmd(cmd);
    lcd_write_data(data);
}

static uint16_t LCD_UI_UNUSED lcd_read_id(void)
{
    uint16_t id;

    lcd_write_cmd(0xD3);
    (void)lcd_read_data();
    (void)lcd_read_data();
    id = (uint16_t)(lcd_read_data() << 8);
    id |= lcd_read_data();
    if (id == 0x9341) {
        return id;
    }

    lcd_write_cmd(0x04);
    (void)lcd_read_data();
    (void)lcd_read_data();
    id = (uint16_t)(lcd_read_data() << 8);
    id |= lcd_read_data();
    if (id == 0x8552 || id == 0x7789) {
        return 0x7789;
    }

    return id;
}

static void lcd_ili9341_init(void)
{
    lcd_write_cmd(0x01);
    delay_ms(20);
    lcd_write_cmd(0xCF); lcd_write_data(0x00); lcd_write_data(0xC1); lcd_write_data(0x30);
    lcd_write_cmd(0xED); lcd_write_data(0x64); lcd_write_data(0x03); lcd_write_data(0x12); lcd_write_data(0x81);
    lcd_write_cmd(0xE8); lcd_write_data(0x85); lcd_write_data(0x10); lcd_write_data(0x7A);
    lcd_write_cmd(0xCB); lcd_write_data(0x39); lcd_write_data(0x2C); lcd_write_data(0x00); lcd_write_data(0x34); lcd_write_data(0x02);
    lcd_write_cmd(0xF7); lcd_write_data(0x20);
    lcd_write_cmd(0xEA); lcd_write_data(0x00); lcd_write_data(0x00);
    lcd_write_reg(0xC0, 0x1B);
    lcd_write_reg(0xC1, 0x01);
    lcd_write_cmd(0xC5); lcd_write_data(0x30); lcd_write_data(0x30);
    lcd_write_cmd(0xC7); lcd_write_data(0xB7);
    lcd_write_reg(0x36, 0xA8);
    lcd_write_reg(0x3A, 0x55);
    lcd_write_cmd(0xB1); lcd_write_data(0x00); lcd_write_data(0x1A);
    lcd_write_cmd(0xB6); lcd_write_data(0x0A); lcd_write_data(0xA2);
    lcd_write_reg(0xF2, 0x00);
    lcd_write_reg(0x26, 0x01);
    lcd_write_cmd(0xE0);
    lcd_write_data(0x0F); lcd_write_data(0x2A); lcd_write_data(0x28); lcd_write_data(0x08); lcd_write_data(0x0E);
    lcd_write_data(0x08); lcd_write_data(0x54); lcd_write_data(0xA9); lcd_write_data(0x43); lcd_write_data(0x0A);
    lcd_write_data(0x0F); lcd_write_data(0x00); lcd_write_data(0x00); lcd_write_data(0x00); lcd_write_data(0x00);
    lcd_write_cmd(0xE1);
    lcd_write_data(0x00); lcd_write_data(0x15); lcd_write_data(0x17); lcd_write_data(0x07); lcd_write_data(0x11);
    lcd_write_data(0x06); lcd_write_data(0x2B); lcd_write_data(0x56); lcd_write_data(0x3C); lcd_write_data(0x05);
    lcd_write_data(0x10); lcd_write_data(0x0F); lcd_write_data(0x3F); lcd_write_data(0x3F); lcd_write_data(0x0F);
    lcd_write_cmd(0x11);
    delay_ms(120);
    lcd_write_cmd(0x29);
}

static void LCD_UI_UNUSED lcd_st7789_init(void)
{
    lcd_write_cmd(0x11);
    delay_ms(120);
    lcd_write_reg(0x36, 0x00);
    lcd_write_reg(0x3A, 0x05);
    lcd_write_cmd(0xB2); lcd_write_data(0x0C); lcd_write_data(0x0C); lcd_write_data(0x00); lcd_write_data(0x33); lcd_write_data(0x33);
    lcd_write_reg(0xB7, 0x35);
    lcd_write_reg(0xBB, 0x32);
    lcd_write_reg(0xC0, 0x0C);
    lcd_write_reg(0xC2, 0x01);
    lcd_write_reg(0xC3, 0x10);
    lcd_write_reg(0xC4, 0x20);
    lcd_write_reg(0xC6, 0x0F);
    lcd_write_cmd(0xD0); lcd_write_data(0xA4); lcd_write_data(0xA1);
    lcd_write_cmd(0x21);
    lcd_write_cmd(0x29);
}

static void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    lcd_write_cmd(0x2A);
    lcd_write_data(x0 >> 8); lcd_write_data(x0 & 0xFF);
    lcd_write_data(x1 >> 8); lcd_write_data(x1 & 0xFF);
    lcd_write_cmd(0x2B);
    lcd_write_data(y0 >> 8); lcd_write_data(y0 & 0xFF);
    lcd_write_data(y1 >> 8); lcd_write_data(y1 & 0xFF);
    lcd_write_cmd(0x2C);
}

static void lcd_fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (x >= LCD_W || y >= LCD_H) return;
    if ((uint32_t)x + w > LCD_W) w = LCD_W - x;
    if ((uint32_t)y + h > LCD_H) h = LCD_H - y;

    lcd_set_window(x, y, (uint16_t)(x + w - 1U), (uint16_t)(y + h - 1U));
    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        lcd_write_data(color);
    }
}

static uint8_t glyph_row(char c, uint8_t row)
{
    static const uint8_t digits[10][7] = {
        {14,17,19,21,25,17,14}, {4,12,4,4,4,4,14}, {14,17,1,2,4,8,31},
        {30,1,1,14,1,1,30}, {2,6,10,18,31,2,2}, {31,16,30,1,1,17,14},
        {6,8,16,30,17,17,14}, {31,1,2,4,8,8,8}, {14,17,17,14,17,17,14},
        {14,17,17,15,1,2,12}
    };
    static const uint8_t letters[26][7] = {
        {14,17,17,31,17,17,17}, {30,17,17,30,17,17,30}, {14,17,16,16,16,17,14},
        {30,17,17,17,17,17,30}, {31,16,16,30,16,16,31}, {31,16,16,30,16,16,16},
        {14,17,16,23,17,17,15}, {17,17,17,31,17,17,17}, {14,4,4,4,4,4,14},
        {7,2,2,2,18,18,12}, {17,18,20,24,20,18,17}, {16,16,16,16,16,16,31},
        {17,27,21,21,17,17,17}, {17,25,21,19,17,17,17}, {14,17,17,17,17,17,14},
        {30,17,17,30,16,16,16}, {14,17,17,17,21,18,13}, {30,17,17,30,20,18,17},
        {15,16,16,14,1,1,30}, {31,4,4,4,4,4,4}, {17,17,17,17,17,17,14},
        {17,17,17,17,17,10,4}, {17,17,17,21,21,21,10}, {17,17,10,4,10,17,17},
        {17,17,10,4,4,4,4}, {31,1,2,4,8,16,31}
    };

    if (c >= '0' && c <= '9') return digits[c - '0'][row];
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    if (c >= 'A' && c <= 'Z') return letters[c - 'A'][row];
    switch (c) {
        case ' ': return 0;
        case '%': { static const uint8_t p[7] = {25,25,2,4,8,19,19}; return p[row]; }
        case '.': return row == 6 ? 4 : 0;
        case ':': return (row == 2 || row == 4) ? 4 : 0;
        case '-': return row == 3 ? 31 : 0;
        case '/': return (uint8_t)(1U << (4U - (row > 4 ? 4 : row)));
        default: return 0;
    }
}

static void draw_char(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale)
{
    for (uint8_t row = 0; row < 7; row++) {
        uint8_t bits = glyph_row(c, row);
        for (uint8_t col = 0; col < 5; col++) {
            uint16_t color = (bits & (1U << (4U - col))) ? fg : bg;
            lcd_fill((uint16_t)(x + col * scale), (uint16_t)(y + row * scale), scale, scale, color);
        }
    }
}

static void draw_text(uint16_t x, uint16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t scale)
{
    while (*s) {
        draw_char(x, y, *s++, fg, bg, scale);
        x = (uint16_t)(x + 6U * scale);
    }
}

static void u16_to_dec(uint16_t value, char *out)
{
    char temp[6];
    uint8_t i = 0;

    if (value == 0) {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    while (value && i < sizeof(temp)) {
        temp[i++] = (char)('0' + (value % 10U));
        value /= 10U;
    }

    for (uint8_t j = 0; j < i; j++) {
        out[j] = temp[i - 1U - j];
    }
    out[i] = '\0';
}

static void draw_panel(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    lcd_fill(x, y, w, h, color);
    lcd_fill(x, y, w, 2, COLOR_GRAY);
    lcd_fill(x, (uint16_t)(y + h - 2), w, 2, COLOR_GRAY);
    lcd_fill(x, y, 2, h, COLOR_GRAY);
    lcd_fill((uint16_t)(x + w - 2), y, 2, h, COLOR_GRAY);
}

static void LCD_UI_UNUSED draw_bar(uint16_t x, uint16_t y, uint16_t w, uint16_t fill, uint16_t color)
{
    lcd_fill(x, y, w, 8, 0xDEFB);
    lcd_fill(x, y, fill, 8, color);
}

static void draw_button(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *label, uint8_t active)
{
    uint16_t bg = active ? COLOR_HEADER : COLOR_NAV;
    lcd_fill(x, y, w, h, bg);
    lcd_fill(x, y, w, 1, COLOR_WHITE);
    draw_text((uint16_t)(x + 8U), (uint16_t)(y + 8U), label, COLOR_WHITE, bg, 1);
}

static void draw_num(uint16_t x, uint16_t y, uint16_t value, uint16_t fg, uint16_t bg, uint8_t scale)
{
    char text[6];
    u16_to_dec(value, text);
    draw_text(x, y, text, fg, bg, scale);
}

static void draw_temp(uint16_t x, uint16_t y, int16_t c10, uint16_t fg, uint16_t bg, uint8_t scale)
{
    char text[8];
    uint16_t v = c10 < 0 ? (uint16_t)-c10 : (uint16_t)c10;
    char whole[5];
    char frac[2];

    u16_to_dec((uint16_t)(v / 10U), whole);
    u16_to_dec((uint16_t)(v % 10U), frac);
    uint8_t p = 0;
    if (c10 < 0) text[p++] = '-';
    for (uint8_t i = 0; whole[i] != '\0'; i++) text[p++] = whole[i];
    text[p++] = '.';
    text[p++] = frac[0];
    text[p++] = 'C';
    text[p] = '\0';
    draw_text(x, y, text, fg, bg, scale);
}

static void draw_header(const char *title)
{
    lcd_fill(0, 0, LCD_W, LCD_H, COLOR_BG);
    lcd_fill(0, 0, LCD_W, 30, COLOR_HEADER);
    draw_text(8, 8, title, COLOR_WHITE, COLOR_HEADER, 2);
    draw_text(218, 10, g_ui_data.wifi_connected ? "LINK OK" : "NO LINK", COLOR_WHITE, COLOR_HEADER, 1);
    if (g_ui_data.alarm_active) {
        lcd_fill(280, 4, 34, 22, COLOR_RED);
        draw_text(286, 11, "ALM", COLOR_WHITE, COLOR_RED, 1);
    }
}

static void draw_nav(void)
{
    draw_button(0, 206, 80, 34, "HOME", g_current_page == 0);
    draw_button(80, 206, 80, 34, "HEALTH", g_current_page == 1);
    draw_button(160, 206, 80, 34, "PILL", g_current_page == 2);
    draw_button(240, 206, 80, 34, "ALERT", g_current_page == 3);
}

static void draw_home(void)
{
    draw_header("ELDER CARE");

    draw_panel(8, 40, 96, 72, COLOR_PANEL);
    draw_text(18, 50, "HEALTH", COLOR_DARK, COLOR_PANEL, 1);
    draw_num(18, 68, g_ui_data.heart_rate, COLOR_RED, COLOR_PANEL, 2);
    draw_text(62, 76, "BPM", COLOR_DARK, COLOR_PANEL, 1);
    draw_text(18, 96, g_ui_data.fall_detected ? "FALL" : "STABLE", g_ui_data.fall_detected ? COLOR_RED : COLOR_GREEN, COLOR_PANEL, 1);

    draw_panel(112, 40, 96, 72, COLOR_PANEL);
    draw_text(122, 50, "SPO2", COLOR_DARK, COLOR_PANEL, 1);
    draw_num(122, 70, g_ui_data.spo2, COLOR_BLUE, COLOR_PANEL, 2);
    draw_text(166, 78, "%", COLOR_DARK, COLOR_PANEL, 1);
    draw_temp(122, 96, g_ui_data.temperature_c10, COLOR_GREEN, COLOR_PANEL, 1);

    draw_panel(216, 40, 96, 72, COLOR_PANEL);
    draw_text(226, 50, "PILL BOX", COLOR_DARK, COLOR_PANEL, 1);
    draw_text(226, 70, "SLOT", COLOR_DARK, COLOR_PANEL, 1);
    draw_num(268, 70, g_ui_data.pillbox_slot, COLOR_ORANGE, COLOR_PANEL, 2);
    draw_text(226, 96, g_ui_data.servo_open ? "OPEN" : "READY", g_ui_data.servo_open ? COLOR_RED : COLOR_GREEN, COLOR_PANEL, 1);

    draw_panel(8, 122, 304, 72, COLOR_PANEL);
    draw_text(20, 134, "SYSTEM FLOW", COLOR_DARK, COLOR_PANEL, 1);
    draw_text(20, 154, "RTC PILL  HX711  MAX30102  IMU", COLOR_BLUE, COLOR_PANEL, 1);
    draw_text(20, 174, g_ui_data.alarm_active ? "ALARM ACTIVE - CHECK DETAIL" : "NORMAL MONITORING", g_ui_data.alarm_active ? COLOR_RED : COLOR_GREEN, COLOR_PANEL, 1);

    draw_nav();
}

static void draw_health(void)
{
    draw_header("HEALTH DETAIL");
    draw_panel(8, 40, 148, 70, COLOR_PANEL);
    draw_text(20, 50, "HEART RATE", COLOR_DARK, COLOR_PANEL, 1);
    draw_num(20, 70, g_ui_data.heart_rate, COLOR_RED, COLOR_PANEL, 2);
    draw_text(72, 78, "BPM", COLOR_DARK, COLOR_PANEL, 1);

    draw_panel(164, 40, 148, 70, COLOR_PANEL);
    draw_text(176, 50, "BLOOD OXYGEN", COLOR_DARK, COLOR_PANEL, 1);
    draw_num(176, 70, g_ui_data.spo2, COLOR_BLUE, COLOR_PANEL, 2);
    draw_text(220, 78, "%", COLOR_DARK, COLOR_PANEL, 1);

    draw_panel(8, 120, 148, 70, COLOR_PANEL);
    draw_text(20, 130, "TEMP", COLOR_DARK, COLOR_PANEL, 1);
    draw_temp(20, 152, g_ui_data.temperature_c10, COLOR_GREEN, COLOR_PANEL, 2);

    draw_panel(164, 120, 148, 70, COLOR_PANEL);
    draw_text(176, 130, "POSTURE", COLOR_DARK, COLOR_PANEL, 1);
    draw_text(176, 150, g_ui_data.fall_detected ? "FALL RISK" : "STABLE", g_ui_data.fall_detected ? COLOR_RED : COLOR_GREEN, COLOR_PANEL, 2);
    draw_text(176, 176, "PITCH ROLL", COLOR_DARK, COLOR_PANEL, 1);
    draw_nav();
}

static void draw_pill(void)
{
    draw_header("PILL DETAIL");
    draw_panel(8, 40, 148, 70, COLOR_PANEL);
    draw_text(20, 50, "NEXT SLOT", COLOR_DARK, COLOR_PANEL, 1);
    draw_num(20, 70, g_ui_data.pillbox_slot, COLOR_ORANGE, COLOR_PANEL, 3);
    draw_text(78, 82, g_ui_data.servo_open ? "OPEN" : "READY", g_ui_data.servo_open ? COLOR_RED : COLOR_GREEN, COLOR_PANEL, 1);

    draw_panel(164, 40, 148, 70, COLOR_PANEL);
    draw_text(176, 50, "WEIGHT", COLOR_DARK, COLOR_PANEL, 1);
    draw_num(176, 70, g_ui_data.pill_weight_g, COLOR_BLUE, COLOR_PANEL, 2);
    draw_text(236, 78, "G", COLOR_DARK, COLOR_PANEL, 1);

    draw_panel(8, 120, 304, 70, COLOR_PANEL);
    draw_text(20, 132, "SERVO MG90S", COLOR_DARK, COLOR_PANEL, 1);
    draw_text(20, 152, "PWM 50HZ  0.5/1.5/2.5MS", COLOR_BLUE, COLOR_PANEL, 1);
    draw_text(20, 172, "OPEN SLOT AFTER RTC REMIND", COLOR_GREEN, COLOR_PANEL, 1);
    draw_nav();
}

static void draw_alert(void)
{
    draw_header("ALERT AND DATA");
    draw_panel(8, 40, 304, 70, COLOR_PANEL);
    draw_text(20, 52, "ALARM", COLOR_DARK, COLOR_PANEL, 1);
    draw_text(90, 50, g_ui_data.alarm_active ? "ACTIVE" : "CLEAR", g_ui_data.alarm_active ? COLOR_RED : COLOR_GREEN, COLOR_PANEL, 2);
    draw_text(20, 84, "FALL HR SPO2 PILL LOW", COLOR_BLUE, COLOR_PANEL, 1);

    draw_panel(8, 120, 304, 70, COLOR_PANEL);
    draw_text(20, 132, "CAPTURE PORT", COLOR_DARK, COLOR_PANEL, 1);
    draw_text(20, 152, "UI_GETCAPTUREFRAME", COLOR_BLUE, COLOR_PANEL, 1);
    draw_text(20, 172, "READY FOR UART WIFI PACKET", COLOR_GREEN, COLOR_PANEL, 1);
    draw_nav();
}

static void draw_page(void)
{
    switch (g_current_page) {
        case 1: draw_health(); break;
        case 2: draw_pill(); break;
        case 3: draw_alert(); break;
        default: draw_home(); break;
    }
    g_needs_redraw = 0;
}

static void touch_write_byte(uint8_t data)
{
    for (uint8_t i = 0; i < 8U; i++) {
        if (data & 0x80U) {
            GPIOE->BSRR = TOUCH_MOSI;
        } else {
            GPIOE->BSRR = (uint32_t)TOUCH_MOSI << 16;
        }

        data <<= 1;
        GPIOE->BSRR = (uint32_t)TOUCH_SCK << 16;
        delay_cycles(80);
        GPIOE->BSRR = TOUCH_SCK;
        delay_cycles(80);
    }
}

static uint16_t touch_read_ad(uint8_t cmd)
{
    uint16_t value = 0;

    GPIOE->BSRR = (uint32_t)TOUCH_SCK << 16;
    GPIOE->BSRR = (uint32_t)TOUCH_MOSI << 16;
    GPIOE->BSRR = (uint32_t)TOUCH_CS << 16;
    touch_write_byte(cmd);
    delay_cycles(400);

    GPIOE->BSRR = (uint32_t)TOUCH_SCK << 16;
    delay_cycles(80);
    GPIOE->BSRR = TOUCH_SCK;
    delay_cycles(80);
    GPIOE->BSRR = (uint32_t)TOUCH_SCK << 16;

    for (uint8_t i = 0; i < 16U; i++) {
        value <<= 1;
        GPIOE->BSRR = (uint32_t)TOUCH_SCK << 16;
        delay_cycles(80);
        GPIOE->BSRR = TOUCH_SCK;
        if (GPIOE->IDR & TOUCH_MISO) {
            value++;
        }
        delay_cycles(80);
    }

    GPIOE->BSRR = TOUCH_CS;
    return (uint16_t)(value >> 4);
}

static uint16_t touch_read_filtered(uint8_t cmd)
{
    uint16_t min = 0xFFFF;
    uint16_t max = 0;
    uint32_t sum = 0;

    for (uint8_t i = 0; i < 5U; i++) {
        uint16_t v = touch_read_ad(cmd);
        if (v < min) min = v;
        if (v > max) max = v;
        sum += v;
    }

    return (uint16_t)((sum - min - max) / 3U);
}

static uint8_t touch_is_down(void)
{
    return (GPIOD->IDR & TOUCH_PEN) == 0U;
}

static uint8_t touch_read_raw(uint16_t *x, uint16_t *y)
{
    if (!touch_is_down()) {
        return 0;
    }

    *x = touch_read_filtered(0xD0);
    *y = touch_read_filtered(0x90);
    return touch_is_down();
}

static uint16_t clamp_map(uint16_t raw, uint16_t out_max, uint8_t invert)
{
    uint32_t value;

    if (raw < TOUCH_RAW_MIN) raw = TOUCH_RAW_MIN;
    if (raw > TOUCH_RAW_MAX) raw = TOUCH_RAW_MAX;
    value = ((uint32_t)(raw - TOUCH_RAW_MIN) * out_max) / (TOUCH_RAW_MAX - TOUCH_RAW_MIN);
    if (invert) value = out_max - value;
    return (uint16_t)value;
}

static uint8_t touch_read_screen(uint16_t *x, uint16_t *y)
{
    uint16_t raw_x;
    uint16_t raw_y;
    uint16_t sx;
    uint16_t sy;

    if (!touch_read_raw(&raw_x, &raw_y)) {
        return 0;
    }

#if TOUCH_SWAP_XY
    sx = clamp_map(raw_y, LCD_W - 1U, TOUCH_INV_X);
    sy = clamp_map(raw_x, LCD_H - 1U, TOUCH_INV_Y);
#else
    sx = clamp_map(raw_x, LCD_W - 1U, TOUCH_INV_X);
    sy = clamp_map(raw_y, LCD_H - 1U, TOUCH_INV_Y);
#endif

    *x = sx;
    *y = sy;
    return 1;
}

void UI_OnTouch(uint16_t x, uint16_t y, uint8_t pressed)
{
    if (!pressed) {
        return;
    }

    if (y >= 206U) {
        g_current_page = (uint8_t)(x / 80U);
        if (g_current_page > 3U) g_current_page = 3U;
        g_needs_redraw = 1;
        return;
    }

    if (g_current_page == 0U) {
        if (y >= 40U && y <= 112U) {
            if (x < 108U) g_current_page = 1U;
            else if (x < 212U) g_current_page = 1U;
            else g_current_page = 2U;
            g_needs_redraw = 1;
            return;
        }

        if (y >= 122U && y <= 194U) {
            g_current_page = 3U;
            g_needs_redraw = 1;
        }
    } else if (y < 34U) {
        g_current_page = 0U;
        g_needs_redraw = 1;
    }
}

void UI_Update(const UI_Data_t *data)
{
    if (!data) {
        return;
    }

    g_ui_data = *data;
    g_needs_redraw = 1;
}

uint16_t UI_GetCaptureFrame(uint8_t *buffer, uint16_t buffer_size)
{
    UI_CaptureFrame_t frame;
    const uint8_t *p;
    uint8_t sum = 0;

    if (!buffer || buffer_size < sizeof(frame)) {
        return 0;
    }

    frame.magic = 0xA5;
    frame.version = 1;
    frame.data = g_ui_data;
    frame.checksum = 0;
    p = (const uint8_t *)&frame;
    for (uint16_t i = 0; i < sizeof(frame) - 1U; i++) {
        sum = (uint8_t)(sum + p[i]);
    }
    frame.checksum = sum;

    p = (const uint8_t *)&frame;
    for (uint16_t i = 0; i < sizeof(frame); i++) {
        buffer[i] = p[i];
    }

    return (uint16_t)sizeof(frame);
}

void UI_Init(void)
{
    lcd_bus_init();
    delay_ms(50);

    g_lcd_id = 0x9341;
    lcd_ili9341_init();
    GPIOD->BSRR = GPIO_PIN_15;

    g_current_page = 0;
    g_last_pressed = 0;
    g_needs_redraw = 1;
    draw_page();
}

void UI_FirmwareRun(void)
{
    UI_Init();

    while (1) {
        uint16_t tx = 0;
        uint16_t ty = 0;
        uint8_t pressed = touch_read_screen(&tx, &ty);

        if (pressed) {
            g_last_x = tx;
            g_last_y = ty;
        }

        if (g_last_pressed && !pressed) {
            UI_OnTouch(g_last_x, g_last_y, 1);
        }
        g_last_pressed = pressed;

        if (g_needs_redraw) {
            draw_page();
        }

        GPIOD->BSRR = (uint32_t)GPIO_PIN_14 << 16;
        GPIOC->BSRR = GPIO_PIN_0;
        delay_ms(80);
        GPIOD->BSRR = GPIO_PIN_14;
        GPIOC->BSRR = (uint32_t)GPIO_PIN_0 << 16;
        delay_ms(80);
    }
}
