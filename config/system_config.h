#pragma once

#include <stdint.h>

// Memory Map
#define BOOTLOADER_ADDR       0x08000000
#define APP_SLOT_A_ADDR       0x08010000
#define APP_SLOT_B_ADDR       0x08050000
#define METADATA_ADDR         0x0800F000

#define APP_SIZE              0x40000  // 256KB
#define BOOTLOADER_SIZE       0x10000  // 64KB
#define METADATA_SIZE         0x1000   // 4KB

// Flash Sectors (STM32F4)
#define FLASH_SECTOR_0        0x00000000
#define FLASH_SECTOR_1        0x00004000
#define FLASH_SECTOR_2        0x00008000
#define FLASH_SECTOR_3        0x0000C000
#define FLASH_SECTOR_4        0x00010000
#define FLASH_SECTOR_5        0x00020000
#define FLASH_SECTOR_6        0x00040000
#define FLASH_SECTOR_7        0x00060000

// System Configuration
#define SYSTEM_CLOCK_HZ       168000000
#define UART_BAUDRATE         115200
#define I2C_CLOCK_HZ          100000
#define SPI_CLOCK_HZ          1000000

// FreeRTOS Configuration
#define SENSOR_TASK_PRIORITY  1
#define COMM_TASK_PRIORITY    2
#define STORAGE_TASK_PRIORITY 2
#define UPDATE_TASK_PRIORITY  3
#define IDLE_TASK_PRIORITY    0

#define TASK_STACK_SIZE       256
#define SENSOR_QUEUE_SIZE     16
#define UART_RX_QUEUE_SIZE    128

// Communication Protocol
#define PACKET_HEADER         0xAA
#define MAX_PACKET_SIZE       256
#define CRC16_POLY            0xA001

// OTA Configuration
#define OTA_CHUNK_SIZE        256
#define OTA_TIMEOUT_MS        5000
#define MAX_RETRIES           3

// Storage Configuration
#define STORAGE_SIZE          128
#define FLASH_SECTOR_SIZE     0x20000
#define WEAR_LEVEL_THRESHOLD  1000

// Hardware Configuration
#define LED_STATUS_PIN        GPIO_PIN_13
#define LED_STATUS_PORT       GPIOC
#define BUTTON_USER_PIN       GPIO_PIN_0
#define BUTTON_USER_PORT      GPIOA

// Debug Configuration
#define DEBUG_ENABLED         1
#define ASSERT_ENABLED        1
