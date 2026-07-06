# 2026-07-06 晚间任务单

## 当前硬件结论

- DAPLink 已能通过 SWD 连接 STM32H7R3。
- OpenOCD 连接时需要使用 `AP_NUM=1`，`AP_NUM=0` 会读核失败。
- LCD 背光引脚确认是 `PD15`，已通过调试器手动拉高，屏幕能亮。
- 官方 `实验14 TFTLCD（MCU屏）实验` 直接烧录后没有显示内容，原因倾向于该例程不是纯片内 Flash 工程：片内 `0x08000000` 只有启动和 XSPI 初始化，主要 RO 段在外部 `0x90000000`，当前未成功写入外部 Flash。
- 因此现在优先判断为“背光和硬件供电基本正常，LCD 初始化/外部 Flash 下载/工程链接布局还没跑通”。

## 顾家铭任务

1. 先做底层硬件验证，不急着写完整业务。
2. 确认 2.8 寸 LCD 模块排线方向、压座是否压紧，触控/LCD 模块型号对应 `ATK-MD0280 V2.4.pdf`。
3. 配合软件完成三个最小测试：
   - 背光测试：`PD15` 输出高电平，屏幕应亮。
   - LCD 刷色测试：初始化 FMC + LCD 控制器，循环刷红/绿/蓝/白/黑。
   - 触摸测试：读 `T_PEN/T_CS/T_SCK/T_MISO/T_MOSI`，能输出触摸坐标即可。
4. 舵机 MG90S：
   - 使用外部 5V 供电，不从开发板 3.3V 直接带舵机。
   - 舵机 GND 必须和开发板 GND 共地。
   - PWM 频率 50Hz，脉宽先按 `0.5ms/1.5ms/2.5ms` 对应约 `0/90/180` 度测试。
   - 暂定用 `PA3 / PWM_DAC` 做信号脚；如果实物接线不方便，再换一个空闲 PWM 引脚。

## 许诺言任务

1. UI 先按 2.8 寸 LCD 设计，默认分辨率先按常见 `240x320` 竖屏处理，等 LCD ID 读出来后再确认是否需要调整方向。
2. 不直接写死传感器引脚，UI 只接收数据结构，底层负责采集。
3. 先做三个页面：
   - 首页：时间、老人状态、报警状态。
   - 健康页：心率、血氧、体温/姿态、跌倒状态。
   - 药盒页：服药提醒、药格状态、剩余重量、舵机开格状态。
4. 预留接口建议：

```c
typedef struct {
    uint8_t heart_rate;
    uint8_t spo2;
    float temperature;
    uint8_t fall_detected;
    uint8_t pillbox_slot;
    float pill_weight_g;
    uint8_t alarm_active;
} UI_Data_t;

void UI_Init(void);
void UI_Update(const UI_Data_t *data);
void UI_OnTouch(uint16_t x, uint16_t y, uint8_t pressed);
```

5. 如果用 LVGL，今晚只要求跑出静态页面和 `UI_Update()` 刷新假数据；触摸事件可以先留空。

## 今晚软件优先级

1. 不继续死磕官方 hex，先做一个纯片内 Flash 的最小工程。
2. 第一阶段只做：启动、时钟、串口、LED、`PD15` 背光。
3. 第二阶段移植官方 `lcd.c/lcd.h/lcd_ex.c/lcdfont.h`，把 LCD 刷色跑通。
4. 第三阶段再接 LVGL 或手写轻量 UI。

## 需要交给组员的资料

- `C:\Users\39909\Desktop\嵌赛\3，原理图\3，原理图\ATK-DNH7RX V1.0.pdf`
- `C:\Users\39909\Desktop\嵌赛\3，原理图\3，原理图\ATK-CNH7RX V1.0.pdf`
- `C:\Users\39909\Desktop\嵌赛\3，原理图\3，原理图\ATK-MD0280 V2.4.pdf`
- `C:\Users\39909\Desktop\嵌赛\3，原理图\3，原理图\H7RX开发板IO引脚分配表.xlsx`
- `C:\Users\39909\Desktop\嵌赛\H7RX硬件参考手册_V1.1.pdf`
- `C:\Users\39909\Desktop\嵌赛\STM32H7R3开发指南_V1.2.pdf`
- 如果许诺言用 LVGL：`C:\Users\39909\Desktop\嵌赛\LVGL开发指南_V1.5.pdf`

## 已确认的 LCD 关键引脚

| 功能 | STM32H7R3 引脚 |
| --- | --- |
| LCD_BL | PD15 |
| LCD_CS / FMC_NE1 | PD12 |
| LCD_RS / FMC_A15 | PE10 |
| LCD_WR / FMC_NWE | PB2 |
| LCD_RD / FMC_NOE | PA5 |
| Touch MISO | PE13 |
| Touch MOSI | PE14 |
| Touch PEN | PD4 |
| Touch CS | PE15 |
| Touch SCK | PE12 |

