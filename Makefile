# Secure IoT Sensor Node - Makefile
# Target: STM32F4xx with FreeRTOS

# Project name
PROJECT = Secure_IoT_Sensor_Node

# Toolchain
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size

# Directories
BOOTLOADER_DIR = bootloader
APP_DIR = app
DRIVERS_DIR = drivers
MIDDLEWARE_DIR = middleware
CONFIG_DIR = config
BUILD_DIR = build

# MCU settings
MCU = cortex-m4
ARCH = armv7e-m
FLOAT_ABI = hard
FPU = fpv4-sp-d16

# Compiler flags
CFLAGS = -mcpu=$(MCU) -mthumb -mfloat-abi=$(FLOAT_ABI) -mfpu=$(FPU)
CFLAGS += -DSTM32F407xx -DUSE_HAL_DRIVER
CFLAGS += -Wall -Wextra -Wno-unused-parameter
CFLAGS += -Os -g
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -std=c99

# Linker flags
LDFLAGS = -mcpu=$(MCU) -mthumb -mfloat-abi=$(FLOAT_ABI) -mfpu=$(FPU)
LDFLAGS += -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map
LDFLAGS += -T $(BOOTLOADER_DIR)/bootloader.ld
LDFLAGS += --specs=nano.specs --specs=nosys.specs

# Include directories
INCLUDES = -I$(CONFIG_DIR)
INCLUDES += -I$(BOOTLOADER_DIR)
INCLUDES += -I$(APP_DIR)
INCLUDES += -I$(DRIVERS_DIR)
INCLUDES += -I$(MIDDLEWARE_DIR)/freertos/include
INCLUDES += -I$(MIDDLEWARE_DIR)/crypto

# Source files
BOOTLOADER_SOURCES = $(BOOTLOADER_DIR)/bootloader.c
BOOTLOADER_SOURCES += $(BOOTLOADER_DIR)/flash_if.c
BOOTLOADER_SOURCES += $(BOOTLOADER_DIR)/signature.c
BOOTLOADER_SOURCES += $(BOOTLOADER_DIR)/startup_stm32f4xx.c

APP_SOURCES = $(APP_DIR)/main.c
APP_SOURCES += $(APP_DIR)/sensor_task.c
APP_SOURCES += $(APP_DIR)/comm_task.c
APP_SOURCES += $(APP_DIR)/storage_task.c
APP_SOURCES += $(APP_DIR)/update_task.c

DRIVER_SOURCES = $(DRIVERS_DIR)/uart.c
DRIVER_SOURCES += $(DRIVERS_DIR)/i2c.c
DRIVER_SOURCES += $(DRIVERS_DIR)/gpio.c
DRIVER_SOURCES += $(DRIVERS_DIR)/spi.c
DRIVER_SOURCES += $(DRIVERS_DIR)/flash_if.c

MIDDLEWARE_SOURCES = $(MIDDLEWARE_DIR)/crypto/ota_metadata.c

# FreeRTOS sources (simplified - in practice, you'd use the full FreeRTOS)
FREERTOS_SOURCES = $(MIDDLEWARE_DIR)/freertos/port.c
FREERTOS_SOURCES += $(MIDDLEWARE_DIR)/freertos/list.c
FREERTOS_SOURCES += $(MIDDLEWARE_DIR)/freertos/queue.c
FREERTOS_SOURCES += $(MIDDLEWARE_DIR)/freertos/tasks.c

# HAL sources (simplified - in practice, you'd use STM32 HAL library)
HAL_SOURCES = $(MIDDLEWARE_DIR)/stm32f4xx_hal.c
HAL_SOURCES += $(MIDDLEWARE_DIR)/stm32f4xx_hal_uart.c
HAL_SOURCES += $(MIDDLEWARE_DIR)/stm32f4xx_hal_i2c.c
HAL_SOURCES += $(MIDDLEWARE_DIR)/stm32f4xx_hal_spi.c
HAL_SOURCES += $(MIDDLEWARE_DIR)/stm32f4xx_hal_flash.c

# Object files
BOOTLOADER_OBJECTS = $(BOOTLOADER_SOURCES:%.c=$(BUILD_DIR)/bootloader_%.o)
APP_OBJECTS = $(APP_SOURCES:%.c=$(BUILD_DIR)/app_%.o)
DRIVER_OBJECTS = $(DRIVER_SOURCES:%.c=$(BUILD_DIR)/driver_%.o)
MIDDLEWARE_OBJECTS = $(MIDDLEWARE_SOURCES:%.c=$(BUILD_DIR)/middleware_%.o)
FREERTOS_OBJECTS = $(FREERTOS_SOURCES:%.c=$(BUILD_DIR)/freertos_%.o)
HAL_OBJECTS = $(HAL_SOURCES:%.c=$(BUILD_DIR)/hal_%.o)

ALL_OBJECTS = $(BOOTLOADER_OBJECTS) $(APP_OBJECTS) $(DRIVER_OBJECTS) \
              $(MIDDLEWARE_OBJECTS) $(FREERTOS_OBJECTS) $(HAL_OBJECTS)

# Targets
.PHONY: all bootloader app clean

all: bootloader app

bootloader: $(BUILD_DIR)/bootloader.bin

app: $(BUILD_DIR)/app.bin

$(BUILD_DIR)/bootloader.elf: $(BOOTLOADER_OBJECTS) $(DRIVER_OBJECTS) $(HAL_OBJECTS)
	@echo "Linking bootloader..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^
	$(SIZE) $@

$(BUILD_DIR)/app.elf: $(APP_OBJECTS) $(DRIVER_OBJECTS) $(MIDDLEWARE_OBJECTS) $(FREERTOS_OBJECTS) $(HAL_OBJECTS)
	@echo "Linking application..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(LDFLAGS) -T $(APP_DIR)/app.ld -o $@ $^
	$(SIZE) $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	@echo "Creating binary file..."
	$(OBJCOPY) -O binary $< $@

# Compile C files
$(BUILD_DIR)/bootloader_%.o: $(BOOTLOADER_DIR)/%.c
	@echo "Compiling $<..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/app_%.o: $(APP_DIR)/%.c
	@echo "Compiling $<..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/driver_%.o: $(DRIVERS_DIR)/%.c
	@echo "Compiling $<..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/middleware_%.o: $(MIDDLEWARE_DIR)/%.c
	@echo "Compiling $<..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/freertos_%.o: $(MIDDLEWARE_DIR)/freertos/%.c
	@echo "Compiling $<..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/hal_%.o: $(MIDDLEWARE_DIR)/%.c
	@echo "Compiling $<..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Flash targets (requires st-link or J-Link)
flash-bootloader: $(BUILD_DIR)/bootloader.bin
	@echo "Flashing bootloader..."
	st-flash write $(BUILD_DIR)/bootloader.bin 0x08000000

flash-app: $(BUILD_DIR)/app.bin
	@echo "Flashing application..."
	st-flash write $(BUILD_DIR)/app.bin 0x08010000

# Debug
debug: $(BUILD_DIR)/bootloader.elf
	@echo "Starting GDB server..."
	st-util &
	arm-none-eabi-gdb $(BUILD_DIR)/bootloader.elf -ex "target remote localhost:4242"

# Clean
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)

# Info
info:
	@echo "Project: $(PROJECT)"
	@echo "MCU: STM32F4xx (Cortex-M4)"
	@echo "Build directory: $(BUILD_DIR)"
	@echo "Source files:"
	@echo "  Bootloader: $(words $(BOOTLOADER_SOURCES)) files"
	@echo "  Application: $(words $(APP_SOURCES)) files"
	@echo "  Drivers: $(words $(DRIVER_SOURCES)) files"
	@echo "  Middleware: $(words $(MIDDLEWARE_SOURCES)) files"

help:
	@echo "Available targets:"
	@echo "  all           - Build bootloader and application"
	@echo "  bootloader    - Build bootloader only"
	@echo "  app           - Build application only"
	@echo "  flash-bootloader - Flash bootloader to device"
	@echo "  flash-app     - Flash application to device"
	@echo "  debug         - Start debugging session"
	@echo "  clean         - Clean build directory"
	@echo "  info          - Show project information"
	@echo "  help          - Show this help"
