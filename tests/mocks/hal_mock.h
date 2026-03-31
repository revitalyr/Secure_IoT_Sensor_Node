#pragma once

#include "../config/system_config.h"
#include "../config/memory_map.h"
#include <stdint.h>

// Mock hardware state
typedef struct {
    // GPIO mock state
    uint8_t gpio_state[16];  // Simplified GPIO state
    
    // UART mock state
    uint8_t uart_tx_buffer[1024];
    uint16_t uart_tx_len;
    uint8_t uart_rx_buffer[1024];
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
    
    // System mock state
    uint32_t system_tick;
    uint8_t system_error;
} mock_state_t;

// Global mock state
extern mock_state_t g_mock_state;

// Mock functions
void mock_init(void);
void mock_reset(void);

// GPIO mock functions
void HAL_GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin);

// UART mock functions
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size);
HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size);

// I2C mock functions
HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef* hi2c, uint16_t dev_addr, uint8_t* data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef* hi2c, uint16_t dev_addr, uint8_t* data, uint16_t size, uint32_t timeout);

// SPI mock functions
HAL_StatusTypeDef HAL_SPI_TransmitReceive_DMA(SPI_HandleTypeDef* hspi, uint8_t* tx_data, uint8_t* rx_data, uint16_t size);

// Flash mock functions
HAL_StatusTypeDef HAL_FLASH_Program(uint32_t type_program, uint32_t address, uint64_t data);
HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef* erase, uint32_t* sector_error);

// System mock functions
HAL_StatusTypeDef HAL_Init(void);
void HAL_Delay(uint32_t delay);
uint32_t HAL_GetTick(void);

// Mock data injection functions
void mock_uart_inject_data(uint8_t* data, uint16_t len);
void mock_uart_clear_tx(void);
void mock_i2c_set_response(uint8_t* data, uint16_t len);
void mock_spi_set_response(uint8_t* data, uint16_t len);
void mock_flash_write(uint32_t addr, uint8_t* data, uint32_t len);
void mock_flash_read(uint32_t addr, uint8_t* data, uint32_t len);
