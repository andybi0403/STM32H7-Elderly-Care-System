# Makefile for HX711 Test — STM32H7R3
PREFIX  ?= C:/arm-gcc/bin/arm-none-eabi-
CC   = $(PREFIX)gcc
AS   = $(PREFIX)gcc -x assembler-with-cpp
SZ   = $(PREFIX)size
OBJCOPY = $(PREFIX)objcopy

TARGET    = hx711_test
BUILD_DIR = build

AS_SRC  = ../servo_test/../virtual\ UI/Drivers/CMSIS/Device/ST/STM32H7RSxx/Source/Templates/gcc/startup_stm32h7r3xx.s
C_SRCS  = hx711_test.c Boot/Core/Src/system_stm32h7rsxx.c

MCU      = -mcpu=cortex-m7 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard
C_DEFS   = -DSTM32H7R3xx
INC      = -I. -IBoot/Core/Inc
OPT      = -Og
CFLAGS   = $(MCU) $(C_DEFS) $(INC) $(OPT) -Wall -fdata-sections -ffunction-sections -std=gnu11
LDFLAGS  = $(MCU) -specs=nosys.specs -specs=nano.specs -T STM32H7R3L8_SRAM.ld -lc -lm -lnosys -Wl,--gc-sections

OBJS = $(BUILD_DIR)/hx711_test.o $(BUILD_DIR)/system_stm32h7rsxx.o $(BUILD_DIR)/startup_stm32h7r3xx.o

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET).elf
	@echo "=== Build Complete ==="
	$(SZ) $<

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS) | $(BUILD_DIR)
	$(CC) $^ $(LDFLAGS) -o $@

$(BUILD_DIR)/hx711_test.o: hx711_test.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/system_stm32h7rsxx.o: Boot/Core/Src/system_stm32h7rsxx.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/startup_stm32h7r3xx.o: $(AS_SRC) | $(BUILD_DIR)
	$(AS) -c $(MCU) "$<" -o $@

clean:
	rm -rf $(BUILD_DIR)
