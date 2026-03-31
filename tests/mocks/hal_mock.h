/**
 * @file hal_mock.h
 * @brief Hardware Abstraction Layer mock interface for testing
 * 
 * This header defines HAL mock interfaces, hardware simulation functions,
 * and test utilities for unit testing without actual hardware dependencies.
 * Provides comprehensive mocking of STM32F4 hardware peripherals.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef HAL_MOCK_H
#define HAL_MOCK_H

/* Include Standard Types */
#include <stdint.h>

/* Include Real Firmware Headers */
#include "../../config/system_config.h"
#include "../../config/memory_map.h"

/* Flash Storage Constants (if not defined) */
#ifndef STORAGE_FLASH_BASE
#define STORAGE_FLASH_BASE        0x08080000U  /**< Base address for storage */
#define STORAGE_SECTOR_START     0x08080000U  /**< Start of storage sector */
#endif

#ifndef STORAGE_SIZE
#define STORAGE_SIZE            (128 * 1024)  // 128KB
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for HAL types
typedef struct UART_HandleTypeDef UART_HandleTypeDef;
typedef struct I2C_HandleTypeDef I2C_HandleTypeDef;
typedef struct SPI_HandleTypeDef SPI_HandleTypeDef;
typedef struct GPIO_TypeDef GPIO_TypeDef;

// HAL return codes
typedef enum {
    HAL_OK       = 0x00U,
    HAL_ERROR    = 0x01U,
    HAL_BUSY     = 0x02U,
    HAL_TIMEOUT   = 0x03U
} HAL_StatusTypeDef;

// GPIO states
typedef enum {
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET   = 1
} GPIO_PinState;

// GPIO init structure
typedef struct {
    uint16_t Pin;
    uint32_t Mode;
    uint32_t Pull;
    uint32_t Speed;
    uint32_t Alternate;
} GPIO_InitTypeDef;

// Flash erase structure
typedef struct {
    uint32_t TypeErase;
    uint32_t VoltageRange;
    uint32_t Sector;
    uint32_t NbSectors;
} FLASH_EraseInitTypeDef;

// GPIO constants
#define GPIO_MODE_OUTPUT_PP     0x01U
#define GPIO_MODE_INPUT        0x00U
#define GPIO_MODE_AF_PP        0x02U
#define GPIO_MODE_IT_RISING    0x08U
#define GPIO_NOPULL           0x00U
#define GPIO_SPEED_FREQ_LOW     0x00U
#define GPIO_SPEED_FREQ_HIGH    0x03U

// Flash constants (only if not already defined)
#ifndef FLASH_TYPEERASE_SECTORS
#define FLASH_TYPEERASE_SECTORS  0x01U
#endif

#ifndef FLASH_VOLTAGE_RANGE_3
#define FLASH_VOLTAGE_RANGE_3   0x02U
#endif

#ifndef FLASH_SECTOR_5
#define FLASH_SECTOR_5          0x05U
#endif

#ifndef FLASH_SECTOR_SIZE
#define FLASH_SECTOR_SIZE       0x20000U  // 128KB sectors
#endif

// UART constants
#define UART_WORDLENGTH_8B      0x00U
#define UART_STOPBITS_1         0x00U
#define UART_PARITY_NONE        0x00U
#define UART_MODE_TX_RX         0x0CU
#define UART_HWCONTROL_NONE     0x00U
#define UART_OVERSAMPLING_16   0x00U
#define UART_IT_IDLE           0x1000U

// I2C constants
#define I2C_ADDRESSINGMODE_7BIT  0x00U

// SPI constants
#define SPI_MODE_MASTER          0x01U
#define SPI_DIRECTION_2LINES     0x00U
#define SPI_DATASIZE_8BIT       0x00U
#define SPI_POLARITY_LOW        0x00U
#define SPI_PHASE_1EDGE         0x01U
#define SPI_NSS_SOFT            0x01U
#define SPI_BAUDRATEPRESCALER_64 0x20U

// HAL constants
#define HAL_MAX_DELAY           0xFFFFFFFFU

// Mock hardware state
typedef struct {
    // GPIO mock state
    uint8_t gpio_state[16];  // Simplified GPIO state
    
    // UART mock state
    uint8_t uart_tx_buffer[2048];
    uint16_t uart_tx_len;
    uint8_t uart_rx_buffer[2048];
    uint16_t uart_rx_len;
    uint16_t uart_rx_pos;
    
    // I2C mock state
    uint8_t i2c_data[256];
    uint16_t i2c_data_len;
    
    // SPI mock state
    uint8_t spi_tx_buffer[1024];
    uint8_t spi_rx_buffer[1024];
    uint16_t spi_len;
    
    // Flash mock state
    uint8_t flash_memory[1024 * 1024];  // 1MB mock flash
    uint32_t flash_write_count;
    uint8_t flash_storage[STORAGE_SIZE];
    uint32_t flash_write_addr;
    uint8_t flash_init_done;
    
    // OTA metadata mock state
    ota_metadata_t ota_meta[2];  // Two slots
    uint8_t ota_init_done;
    
    // System mock state
    uint32_t system_tick;
    uint8_t system_error;
} mock_state_t;

// Global mock state
extern mock_state_t g_mock_state;

// HAL mock functions
HAL_StatusTypeDef HAL_Init(void);
void HAL_Delay(uint32_t delay);
uint32_t HAL_GetTick(void);

void HAL_GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin);
void HAL_GPIO_Init(GPIO_InitTypeDef* init);

HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size);
HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size);
void HAL_UART_IRQHandler(UART_HandleTypeDef* huart);

HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef* hi2c, uint16_t dev_addr, uint8_t* data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef* hi2c, uint16_t dev_addr, uint8_t* data, uint16_t size, uint32_t timeout);

HAL_StatusTypeDef HAL_SPI_TransmitReceive_DMA(SPI_HandleTypeDef* hspi, uint8_t* tx_data, uint8_t* rx_data, uint16_t size);

HAL_StatusTypeDef HAL_FLASH_Program(uint32_t type_program, uint32_t address, uint64_t data);
HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef* erase, uint32_t* sector_error);
HAL_StatusTypeDef HAL_FLASH_Unlock(void);
HAL_StatusTypeDef HAL_FLASH_Lock(void);

// Firmware functions that need mocking
void uart_init(void);
void uart_write(uint8_t* data, uint16_t len);
void uart_write_string(const char* str);
uint16_t uart_read(uint8_t* buffer, uint16_t max_len);

boot_error_t flash_storage_init(void);
boot_error_t flash_storage_write(uint32_t addr, uint8_t* data, uint32_t len);
boot_error_t flash_storage_read(uint32_t addr, uint8_t* data, uint32_t len);
boot_error_t flash_storage_erase_sector(uint32_t sector_start);

boot_error_t ota_metadata_init(void);
boot_error_t ota_metadata_read(ota_metadata_t* meta);
boot_error_t ota_metadata_write(const ota_metadata_t* meta);
boot_error_t ota_metadata_set_pending(uint32_t crc, uint32_t size, uint32_t version);
boot_error_t ota_metadata_commit(uint32_t slot);
boot_error_t ota_metadata_rollback(void);
uint32_t ota_metadata_get_active_slot(void);
uint8_t ota_metadata_is_update_pending(void);

// Wear leveling functions
boot_error_t wear_level_init(void);
boot_error_t wear_level_write(uint8_t* data, uint32_t len);
boot_error_t wear_level_read(uint32_t logical_addr, uint8_t* data, uint32_t len);
uint32_t wear_level_get_free_space(void);
uint32_t wear_level_get_write_count(uint32_t logical_addr);

// System functions
void system_init(void);

// Storage functions
boot_error_t storage_write_sensor_data(const sensor_data_t* data);
boot_error_t storage_read_sensor_data(uint32_t index, sensor_data_t* data);
boot_error_t storage_get_latest_data(sensor_data_t* data);
boot_error_t storage_clear_all(void);

/* Mock Helper Functions */
void mock_init(void);
void mock_reset(void);
void mock_uart_inject_data(uint8_t* data, uint16_t len);
void mock_uart_clear_tx(void);
void mock_i2c_set_response(uint8_t* data, uint16_t len);
void mock_spi_set_response(uint8_t* data, uint16_t len);
void mock_flash_write(uint32_t addr, uint8_t* data, uint32_t len);
void mock_flash_read(uint32_t addr, uint8_t* data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif // HAL_MOCK_H
