# Makefile for STM32H7R3L8HxH — Bare-Metal (no HAL)
# Auto-generated project for 东南大学 嵌赛 智能养老监护系统

# Toolchain (absolute paths — installed via winget)
PREFIX  = "/c/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/14.2 rel1/bin/arm-none-eabi-"
MAKE    = "/c/Users/15730/AppData/Local/Microsoft/WinGet/Packages/ezwinports.make_Microsoft.Winget.Source_8wekyb3d8bbwe/bin/make.exe"
OPENOCD = "/c/Users/15730/AppData/Local/Microsoft/WinGet/Packages/xpack-dev-tools.openocd-xpack_Microsoft.Winget.Source_8wekyb3d8bbwe/xpack-openocd-0.12.0-7/bin/openocd.exe"

CC   = $(PREFIX)gcc
AS   = $(PREFIX)gcc -x assembler-with-cpp
SZ   = $(PREFIX)size

OPENOCD_SCRIPTS = "/c/Users/15730/AppData/Local/Microsoft/WinGet/Packages/xpack-dev-tools.openocd-xpack_Microsoft.Winget.Source_8wekyb3d8bbwe/xpack-openocd-0.12.0-7/openocd/scripts"

# OpenOCD config
OPENOCD_IFACE  = interface/stlink.cfg
OPENOCD_TARGET = stm32h7rx.cfg

# Project
TARGET    = Template
BUILD_DIR = build

# MCU flags (Cortex-M7 + FPv5-SP-D16)
CPU       = -mcpu=cortex-m7
FPU       = -mfpu=fpv5-sp-d16
FLOAT_ABI = -mfloat-abi=hard
MCU       = $(CPU) -mthumb $(FPU) $(FLOAT_ABI)

# C defines
C_DEFS = \
	-DUSE_HAL_DRIVER \
	-DSTM32H7R3xx \
	-DHSE_VALUE=24000000

# Optimization
OPT = -Og

# Include paths
INC = \
	-I. \
	-IBoot/Core/Inc \
	-IDrivers/CMSIS/Include

# Common flags
CFLAGS  = $(MCU) $(C_DEFS) $(INC) $(OPT) -Wall -fdata-sections -ffunction-sections -std=gnu11
LDFLAGS = $(MCU) -specs=nosys.specs -specs=nano.specs -T STM32H7R3L8_FLASH.ld -lc -lm -lnosys -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map

# Source files
C_SRCS = \
	Boot/Core/Src/main.c \
	Boot/Core/Src/stm32h7rsxx_hal_msp.c \
	Boot/Core/Src/stm32h7rsxx_it.c \
	Boot/Core/Src/system_stm32h7rsxx.c

AS_SRCS = Drivers/CMSIS/Device/ST/STM32H7RSxx/Source/Templates/gcc/startup_stm32h7r3xx.s

# Build objects
C_OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(C_SRCS:.c=.o)))
AS_OBJS = $(BUILD_DIR)/startup_stm32h7r3xx.o
OBJECTS = $(AS_OBJS) $(C_OBJS)

# VPATH
VPATH = Boot/Core/Src:Drivers/CMSIS/Device/ST/STM32H7RSxx/Source/Templates/gcc

# Rules
.PHONY: all clean flash

all: $(BUILD_DIR)/$(TARGET).elf
	@echo "=== Build Complete: $(TARGET).elf ==="
	$(SZ) $(BUILD_DIR)/$(TARGET).elf

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(AS) -c $(MCU) $< -o $@

clean:
	rm -rf $(BUILD_DIR)

CUBEPROG = "/c/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe"

flash: all
	$(CUBEPROG) -c port=SWD freq=4000 mode=UR -w $(BUILD_DIR)/$(TARGET).elf -v
	$(OPENOCD) -s $(OPENOCD_SCRIPTS) -f $(OPENOCD_IFACE) -c "transport select swd" -f $(OPENOCD_TARGET) \
		-c "init" -c "halt" -c "reg sp 0x20010000" -c "reg pc 0x080001ad" -c "resume" -c "sleep 100" -c "exit"
