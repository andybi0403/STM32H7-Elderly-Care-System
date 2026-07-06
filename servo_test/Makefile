# Makefile for SG90 Servo Test — STM32H7R3
PREFIX  ?= C:/arm-gcc/bin/arm-none-eabi-
CC   = $(PREFIX)gcc
AS   = $(PREFIX)gcc -x assembler-with-cpp
SZ   = $(PREFIX)size
OBJCOPY = $(PREFIX)objcopy

OPENOCD ?= C:/Users/15730/AppData/Local/Microsoft/WinGet/Packages/xpack-dev-tools.openocd-xpack_Microsoft.Winget.Source_8wekyb3d8bbwe/xpack-openocd-0.12.0-7/bin/openocd
OPENOCD_SCRIPTS ?= C:/Users/15730/AppData/Local/Microsoft/WinGet/Packages/xpack-dev-tools.openocd-xpack_Microsoft.Winget.Source_8wekyb3d8bbwe/xpack-openocd-0.12.0-7/openocd/scripts

TARGET    = servo_test
BUILD_DIR = build

AS_SRC  = ../virtual\ UI/Drivers/CMSIS/Device/ST/STM32H7RSxx/Source/Templates/gcc/startup_stm32h7r3xx.s
C_SRCS  = servo_test.c Boot/Core/Src/system_stm32h7rsxx.c

MCU      = -mcpu=cortex-m7 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard
C_DEFS   = -DSTM32H7R3xx
INC      = -I. -IBoot/Core/Inc
OPT      = -Og
CFLAGS   = $(MCU) $(C_DEFS) $(INC) $(OPT) -Wall -fdata-sections -ffunction-sections -std=gnu11
LDFLAGS  = $(MCU) -specs=nosys.specs -specs=nano.specs -T STM32H7R3L8_FLASH.ld -lc -lm -lnosys -Wl,--gc-sections

OBJS = $(BUILD_DIR)/servo_test.o $(BUILD_DIR)/system_stm32h7rsxx.o $(BUILD_DIR)/startup_stm32h7r3xx.o

.PHONY: all clean flash

all: $(BUILD_DIR)/$(TARGET).elf
	@echo "=== Build Complete ==="
	$(SZ) $<

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS) | $(BUILD_DIR)
	$(CC) $^ $(LDFLAGS) -o $@

$(BUILD_DIR)/servo_test.o: servo_test.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/system_stm32h7rsxx.o: Boot/Core/Src/system_stm32h7rsxx.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/startup_stm32h7r3xx.o: $(AS_SRC) | $(BUILD_DIR)
	$(AS) -c $(MCU) "$<" -o $@

clean:
	rm -rf $(BUILD_DIR)

flash: all
	$(OBJCOPY) -O binary $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin
	"$(OPENOCD)" -s "$(OPENOCD_SCRIPTS)" -f interface/cmsis-dap.cfg -f stm32h7rx.cfg \
		-c "adapter speed 100" -c "init" -c "halt" \
		-c "flash write_image erase $(BUILD_DIR)/$(TARGET).bin 0x08000000" \
		-c "reset run" -c "shutdown"
