#include "hal_mock.h"
#include <string.h>

// Global mock state
mock_state_t g_mock_state;

void mock_init(void)
{
    memset(&g_mock_state, 0, sizeof(g_mock_state));
    
    // Initialize flash with 0xFF (erased state)
    memset(g_mock_state.flash_memory, 0xFF, sizeof(g_mock_state.flash_memory));
}

void mock_reset(void)
{
    mock_init();
}

// GPIO mock functions
void HAL_GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state)
{
    // Simplified - just store the state
    uint16_t pin_num = 0;
    while (pin > 1) {
        pin >>= 1;
        pin_num++;
    }
    
    if (pin_num < 16) {
        g_mock_state.gpio_state[pin_num] = state;
    }
}

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin)
{
    uint16_t pin_num = 0;
    while (pin > 1) {
        pin >>= 1;
        pin_num++;
    }
    
    if (pin_num < 16) {
        return g_mock_state.gpio_state[pin_num];
    }
    
    return GPIO_PIN_RESET;
}

// UART mock functions
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (g_mock_state.uart_tx_len + size > sizeof(g_mock_state.uart_tx_buffer)) {
        return HAL_ERROR;
    }
    
    memcpy(&g_mock_state.uart_tx_buffer[g_mock_state.uart_tx_len], data, size);
    g_mock_state.uart_tx_len += size;
    
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (g_mock_state.uart_rx_pos + size > g_mock_state.uart_rx_len) {
        return HAL_ERROR;
    }
    
    memcpy(data, &g_mock_state.uart_rx_buffer[g_mock_state.uart_rx_pos], size);
    g_mock_state.uart_rx_pos += size;
    
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size)
{
    return HAL_UART_Transmit(huart, data, size, HAL_MAX_DELAY);
}

HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size)
{
    return HAL_UART_Receive(huart, data, size, HAL_MAX_DELAY);
}

// I2C mock functions
HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef* hi2c, uint16_t dev_addr, uint8_t* data, uint16_t size, uint32_t timeout)
{
    // Store transmitted data for verification
    if (g_mock_state.i2c_data_len + size > sizeof(g_mock_state.i2c_data)) {
        return HAL_ERROR;
    }
    
    memcpy(&g_mock_state.i2c_data[g_mock_state.i2c_data_len], data, size);
    g_mock_state.i2c_data_len += size;
    
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef* hi2c, uint16_t dev_addr, uint8_t* data, uint16_t size, uint32_t timeout)
{
    // Return mock data
    if (size > sizeof(g_mock_state.i2c_data)) {
        return HAL_ERROR;
    }
    
    memcpy(data, g_mock_state.i2c_data, size);
    return HAL_OK;
}

// SPI mock functions
HAL_StatusTypeDef HAL_SPI_TransmitReceive_DMA(SPI_HandleTypeDef* hspi, uint8_t* tx_data, uint8_t* rx_data, uint16_t size)
{
    if (size > sizeof(g_mock_state.spi_tx_buffer) || size > sizeof(g_mock_state.spi_rx_buffer)) {
        return HAL_ERROR;
    }
    
    memcpy(g_mock_state.spi_tx_buffer, tx_data, size);
    memcpy(rx_data, g_mock_state.spi_rx_buffer, size);
    g_mock_state.spi_len = size;
    
    return HAL_OK;
}

// Flash mock functions
HAL_StatusTypeDef HAL_FLASH_Program(uint32_t type_program, uint32_t address, uint64_t data)
{
    if (address >= sizeof(g_mock_state.flash_memory)) {
        return HAL_ERROR;
    }
    
    // Simulate flash programming (can only change 1 to 0)
    uint32_t current = *(uint32_t*)&g_mock_state.flash_memory[address];
    uint32_t new_data = (uint32_t)data;
    
    if ((current & new_data) != new_data) {
        return HAL_ERROR; // Trying to set 0 to 1
    }
    
    *(uint32_t*)&g_mock_state.flash_memory[address] = new_data;
    g_mock_state.flash_write_count++;
    
    return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef* erase, uint32_t* sector_error)
{
    uint32_t start_addr = erase->Sector * FLASH_SECTOR_SIZE;
    uint32_t end_addr = start_addr + (erase->NbSectors * FLASH_SECTOR_SIZE);
    
    if (end_addr > sizeof(g_mock_state.flash_memory)) {
        return HAL_ERROR;
    }
    
    // Simulate flash erase (set all bytes to 0xFF)
    memset(&g_mock_state.flash_memory[start_addr], 0xFF, end_addr - start_addr);
    
    return HAL_OK;
}

// System mock functions
HAL_StatusTypeDef HAL_Init(void)
{
    return HAL_OK;
}

void HAL_Delay(uint32_t delay)
{
    g_mock_state.system_tick += delay;
}

uint32_t HAL_GetTick(void)
{
    return g_mock_state.system_tick;
}

// Mock data injection functions
void mock_uart_inject_data(uint8_t* data, uint16_t len)
{
    if (g_mock_state.uart_rx_len + len > sizeof(g_mock_state.uart_rx_buffer)) {
        return;
    }
    
    memcpy(&g_mock_state.uart_rx_buffer[g_mock_state.uart_rx_len], data, len);
    g_mock_state.uart_rx_len += len;
}

void mock_uart_clear_tx(void)
{
    g_mock_state.uart_tx_len = 0;
}

void mock_i2c_set_response(uint8_t* data, uint16_t len)
{
    if (len > sizeof(g_mock_state.i2c_data)) {
        return;
    }
    
    memcpy(g_mock_state.i2c_data, data, len);
    g_mock_state.i2c_data_len = len;
}

void mock_spi_set_response(uint8_t* data, uint16_t len)
{
    if (len > sizeof(g_mock_state.spi_rx_buffer)) {
        return;
    }
    
    memcpy(g_mock_state.spi_rx_buffer, data, len);
}

void mock_flash_write(uint32_t addr, uint8_t* data, uint32_t len)
{
    if (addr + len > sizeof(g_mock_state.flash_memory)) {
        return;
    }
    
    memcpy(&g_mock_state.flash_memory[addr], data, len);
}

void mock_flash_read(uint32_t addr, uint8_t* data, uint32_t len)
{
    if (addr + len > sizeof(g_mock_state.flash_memory)) {
        return;
    }
    
    memcpy(data, &g_mock_state.flash_memory[addr], len);
}
