# HX711 称重模块 + LCD 显示

基于 STM32H7R3 裸机，GPIO模拟HX711协议读取称重传感器，TFT LCD实时显示。

## 接口定义

### HX711 → 开发板

| HX711引脚 | 开发板 | 说明 |
|-----------|--------|------|
| DOUT | **PB6** (P2) | 数据输出 |
| PD_SCK | **PB7** (P2) | 时钟 |
| VCC | **DAPLink 3.3V** | 供电 |
| GND | **DAPLink GND** | 共地 |

### 称重传感器 → HX711 绿端子

| 传感器 | HX711 | 说明 |
|--------|-------|------|
| 红线 (激励+) | **E+** | 桥式传感器供电 |
| 黑线 (激励-) | **E-** | |
| 白线 (信号-) | **A-** | 差分信号 |
| 绿线 (信号+) | **A+** | |

> 颜色以实物标注为准

## 屏幕显示

- 复用原项目的 LCD UI 框架 (ILI9341, GPIO并口)
- 自动显示在**药品页面** (PILL WEIGHT 字段)
- 每秒读取 HX711，仅数值变化时刷新屏幕

## 编译

```bash
cd firmware/module_tests/hx711_test
make -f Makefile_hx711 clean all
```

## 加载运行 (SRAM方式)

```bash
# 转BIN + 通过pyOCD加载到SRAM
arm-none-eabi-objcopy -O binary build/hx711_lcd.elf build/hx711_lcd.bin
python load_to_sram.py
```

> Flash烧录暂不支持(H7RS芯片安全限制)，断电后需重新加载。

## H7RS 关键注意事项

1. **RCC_AHB4ENR 地址偏移为 +0x140** (标准H7是 +0x50)，本项目已正确处理
2. **SRAM运行需手动清零BSS段**，主函数开头已处理
3. **HX711必须在LCD初始化之后配置**，因为LCD init正确开启了GPIOB时钟
4. DAPLink调试器**不能直接写GPIO寄存器**，需通过CPU代码操作

## 行为

1. 上电/加载后，屏幕显示药品页面
2. 零点自动标定 (取上电后第一次读数)
3. 重量变化时 PILL WEIGHT 数字自动更新
4. 秤盘空载时读数接近0，放上砝码/药品后读数增大

## 文件结构

```
hx711_test/
├── main_hx711.c          # 主程序 (HX711读取 + LCD显示)
├── lcd_ui.c/h            # LCD驱动 (复用原项目)
├── stm32h7rsxx.h         # H7RS寄存器定义
├── Makefile_hx711        # 编译脚本
├── STM32H7R3L8_SRAM.ld   # SRAM链接脚本
├── load_to_sram.py       # SRAM加载工具
└── README.md
```
