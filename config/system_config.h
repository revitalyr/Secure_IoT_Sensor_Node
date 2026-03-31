/**
 * @file system_config.h
 * @brief System configuration constants and memory layout definitions
 * 
 * This header defines system-wide configuration constants, memory addresses,
 * flash sector mappings, and hardware-specific settings for the STM32F4-based
 * Secure IoT Sensor Node.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <stdint.h>

/* Memory Map Definitions */
#define BOOTLOADER_ADDR       0x08000000  /**< Bootloader start address */
#define APP_SLOT_A_ADDR       0x08010000  /**< Application slot A address */
#define APP_SLOT_B_ADDR       0x08050000  /**< Application slot B address */
#define METADATA_ADDR         0x0800F000  /**< OTA metadata address */

/* Size Definitions */
#define APP_SIZE              0x40000     /**< Application size (256KB) */
#define BOOTLOADER_SIZE       0x10000     /**< Bootloader size (64KB) */
#define METADATA_SIZE         0x1000      /**< Metadata size (4KB) */
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
#define WEB_TASK_PRIORITY     2
#define LED_TASK_PRIORITY     1
#define PERF_TASK_PRIORITY    1
#define IDLE_TASK_PRIORITY    0

// Stack Sizes
#define SENSOR_TASK_STACK_SIZE 256
#define COMM_TASK_STACK_SIZE   512
#define STORAGE_TASK_STACK_SIZE 256
#define UPDATE_TASK_STACK_SIZE 512
#define WEB_TASK_STACK_SIZE    1024
#define LED_TASK_STACK_SIZE    128
#define PERF_TASK_STACK_SIZE   256

// Queue Depths
#define SENSOR_QUEUE_DEPTH    5
#define UART_QUEUE_DEPTH      10
#define DATA_QUEUE_DEPTH      8

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

/* Hardware Configuration */
#define LED_STATUS_PIN        GPIO_PIN_13
#define LED_STATUS_PORT       GPIOC
#define BUTTON_USER_PIN       GPIO_PIN_0
#define BUTTON_USER_PORT      GPIOA

/* Debug Configuration */
#define DEBUG_ENABLED         1
#define ASSERT_ENABLED        1

#endif // SYSTEM_CONFIG_H
