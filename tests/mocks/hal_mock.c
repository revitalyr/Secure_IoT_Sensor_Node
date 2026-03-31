#include "hal_mock.h"
#include "freertos_mock.h"
#include <string.h>

// Global mock state
mock_state_t g_mock_state;

void mock_init(void)
{
    memset(&g_mock_state, 0, sizeof(g_mock_state));
    
    // Initialize flash with 0xFF (erased state)
    memset(g_mock_state.flash_memory, 0xFF, sizeof(g_mock_state.flash_memory));
    memset(g_mock_state.flash_storage, 0xFF, sizeof(g_mock_state.flash_storage));
}

void mock_reset(void)
{
    mock_init();
}

// GPIO mock functions
void HAL_GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state)
{
    (void)port;
    (void)pin;
    (void)state;
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
    (void)port;
    (void)pin;
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
    (void)huart;
    (void)timeout;
    if (g_mock_state.uart_tx_len + size > sizeof(g_mock_state.uart_tx_buffer)) {
        return HAL_ERROR;
    }
    
    memcpy(&g_mock_state.uart_tx_buffer[g_mock_state.uart_tx_len], data, size);
    g_mock_state.uart_tx_len += size;
    
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size, uint32_t timeout)
{
    (void)huart;
    (void)timeout;
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
    (void)hi2c;
    (void)dev_addr;
    (void)timeout;
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
    (void)hi2c;
    (void)dev_addr;
    (void)timeout;
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
    (void)hspi;
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
    (void)type_program;
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
    (void)erase;
    (void)sector_error;
    uint32_t start_addr = erase->Sector * FLASH_SECTOR_SIZE;
    uint32_t end_addr = start_addr + (erase->NbSectors * FLASH_SECTOR_SIZE);
    
    if (end_addr > sizeof(g_mock_state.flash_memory)) {
        return HAL_ERROR;
    }
    
    // Simulate flash erase (set all bytes to 0xFF)
    memset(&g_mock_state.flash_memory[start_addr], 0xFF, end_addr - start_addr);
    
    return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASH_Unlock(void)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASH_Lock(void)
{
    return HAL_OK;
}

void HAL_GPIO_Init(GPIO_InitTypeDef* init)
{
    (void)init;
    // Mock implementation - does nothing
}

void HAL_UART_IRQHandler(UART_HandleTypeDef* huart)
{
    (void)huart;
    // Mock implementation - does nothing
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

// Firmware function mocks
void uart_init(void)
{
    g_mock_state.uart_tx_len = 0;
    g_mock_state.uart_rx_len = 0;
    g_mock_state.uart_rx_pos = 0;
}

void uart_write(uint8_t* data, uint16_t len)
{
    if (g_mock_state.uart_tx_len + len > sizeof(g_mock_state.uart_tx_buffer)) {
        return;
    }
    
    memcpy(&g_mock_state.uart_tx_buffer[g_mock_state.uart_tx_len], data, len);
    g_mock_state.uart_tx_len += len;
}

void uart_write_string(const char* str)
{
    uint16_t len = strlen(str);
    uart_write((uint8_t*)str, len);
}

uint16_t uart_read(uint8_t* buffer, uint16_t max_len)
{
    uint16_t available = g_mock_state.uart_rx_len - g_mock_state.uart_rx_pos;
    uint16_t to_read = (available < max_len) ? available : max_len;
    
    if (to_read > 0) {
        memcpy(buffer, &g_mock_state.uart_rx_buffer[g_mock_state.uart_rx_pos], to_read);
        g_mock_state.uart_rx_pos += to_read;
    }
    
    return to_read;
}

boot_error_t flash_storage_init(void)
{
    g_mock_state.flash_init_done = 1;
    return BOOT_OK;
}

boot_error_t flash_storage_write(uint32_t addr, uint8_t* data, uint32_t len)
{
    if (!g_mock_state.flash_init_done) {
        return BOOT_FLASH_ERROR;
    }
    
    // Use the larger flash_memory array for storage (1MB)
    uint32_t offset = addr - STORAGE_FLASH_BASE;
    if (offset + len > sizeof(g_mock_state.flash_memory)) {
        return BOOT_METADATA_CORRUPT;
    }
    
    memcpy(&g_mock_state.flash_memory[offset], data, len);
    
    return BOOT_OK;
}

boot_error_t flash_storage_read(uint32_t addr, uint8_t* data, uint32_t len)
{
    if (!g_mock_state.flash_init_done) {
        return BOOT_FLASH_ERROR;
    }
    
    // Use the larger flash_memory array for storage (1MB)
    uint32_t offset = addr - STORAGE_FLASH_BASE;
    if (offset + len > sizeof(g_mock_state.flash_memory)) {
        return BOOT_METADATA_CORRUPT;
    }
    
    memcpy(data, &g_mock_state.flash_memory[offset], len);
    
    return BOOT_OK;
}

boot_error_t flash_storage_erase_sector(uint32_t sector_start)
{
    if (!g_mock_state.flash_init_done) {
        return BOOT_FLASH_ERROR;
    }
    
    // Use the larger flash_memory array for bounds checking
    uint32_t offset = sector_start - STORAGE_FLASH_BASE;
    uint32_t sector_size = 4096; // Typical sector size
    if (offset + sector_size > sizeof(g_mock_state.flash_memory)) {
        sector_size = sizeof(g_mock_state.flash_memory) - offset;
    }
    
    // Simulate sector erase (set to 0xFF)
    memset(&g_mock_state.flash_memory[offset], 0xFF, sector_size);
    
    return BOOT_OK;
}

boot_error_t ota_metadata_init(void)
{
    g_mock_state.ota_init_done = 1;
    
    // Initialize both slots with default values
    for (int i = 0; i < 2; i++) {
        memset(&g_mock_state.ota_meta[i], 0, sizeof(ota_metadata_t));
        g_mock_state.ota_meta[i].active_slot = 0; // Default to slot 0
        g_mock_state.ota_meta[i].update_pending = 0;
    }
    
    return BOOT_OK;
}

boot_error_t ota_metadata_read(ota_metadata_t* meta)
{
    if (!g_mock_state.ota_init_done || meta == NULL) {
        return BOOT_METADATA_CORRUPT;
    }
    
    // Read from the currently active slot
    uint8_t active_slot = g_mock_state.ota_meta[0].active_slot;
    memcpy(meta, &g_mock_state.ota_meta[active_slot], sizeof(ota_metadata_t));
    return BOOT_OK;
}

boot_error_t ota_metadata_write(const ota_metadata_t* meta)
{
    if (!g_mock_state.ota_init_done || meta == NULL) {
        return BOOT_METADATA_CORRUPT;
    }
    
    // Write to the specified slot from the metadata itself
    uint8_t target_slot = meta->active_slot;
    if (target_slot >= 2) {
        target_slot = 0; // Default to slot 0 if invalid
    }
    
    memcpy(&g_mock_state.ota_meta[target_slot], meta, sizeof(ota_metadata_t));
    
    // Update the active slot reference in both metadata entries
    g_mock_state.ota_meta[0].active_slot = target_slot;
    g_mock_state.ota_meta[1].active_slot = target_slot;
    
    return BOOT_OK;
}

boot_error_t ota_metadata_set_pending(uint32_t crc, uint32_t size, uint32_t version)
{
    if (!g_mock_state.ota_init_done) {
        return BOOT_METADATA_CORRUPT;
    }
    
    // Set pending on the inactive slot
    uint8_t inactive_slot = (g_mock_state.ota_meta[0].active_slot == 0) ? 1 : 0;
    
    g_mock_state.ota_meta[inactive_slot].crc = crc;
    g_mock_state.ota_meta[inactive_slot].size = size;
    g_mock_state.ota_meta[inactive_slot].version = version;
    g_mock_state.ota_meta[inactive_slot].update_pending = 1;
    
    return BOOT_OK;
}

boot_error_t ota_metadata_commit(uint32_t slot)
{
    if (!g_mock_state.ota_init_done || slot >= 2) {
        return BOOT_METADATA_CORRUPT;
    }
    
    // Clear pending flag and set as active
    g_mock_state.ota_meta[slot].update_pending = 0;
    
    // Update active slot in both metadata entries
    g_mock_state.ota_meta[0].active_slot = slot;
    g_mock_state.ota_meta[1].active_slot = slot;
    
    // Clear pending flag in the other slot too
    uint8_t other_slot = (slot == 0) ? 1 : 0;
    g_mock_state.ota_meta[other_slot].update_pending = 0;
    
    return BOOT_OK;
}

boot_error_t ota_metadata_rollback(void)
{
    if (!g_mock_state.ota_init_done) {
        return BOOT_METADATA_CORRUPT;
    }
    
    // Switch to the other slot
    uint8_t current_slot = g_mock_state.ota_meta[0].active_slot;
    uint8_t new_slot = (current_slot == 0) ? 1 : 0;
    
    return ota_metadata_commit(new_slot);
}

uint32_t ota_metadata_get_active_slot(void)
{
    if (!g_mock_state.ota_init_done) {
        return 0;
    }
    
    return g_mock_state.ota_meta[0].active_slot;
}

uint8_t ota_metadata_is_update_pending(void)
{
    if (!g_mock_state.ota_init_done) {
        return 0;
    }
    
    // Check if either slot has pending update
    return (g_mock_state.ota_meta[0].update_pending || g_mock_state.ota_meta[1].update_pending) ? 1 : 0;
}

// Wear leveling functions
boot_error_t wear_level_init(void)
{
    // Mock implementation - ensure flash storage is initialized
    if (!g_mock_state.flash_init_done) {
        flash_storage_init();
    }
    return BOOT_OK;
}

boot_error_t wear_level_write(uint8_t* data, uint32_t len)
{
    // Mock implementation - write to flash storage at a fixed address
    uint32_t addr = STORAGE_FLASH_BASE + 0x1000; // Offset for wear leveling area
    
    // Increment write count
    g_mock_state.flash_write_count++;
    
    // For testing, skip bounds checking and just write
    return flash_storage_write(addr, data, len);
}

boot_error_t wear_level_read(uint32_t logical_addr, uint8_t* data, uint32_t len)
{
    // Mock implementation - read from wear leveling area
    uint32_t addr = STORAGE_FLASH_BASE + 0x1000 + logical_addr;
    return flash_storage_read(addr, data, len);
}

uint32_t wear_level_get_free_space(void)
{
    // Mock implementation - return some free space
    return STORAGE_SIZE / 2;
}

uint32_t wear_level_get_write_count(uint32_t logical_addr)
{
    (void)logical_addr;
    // Mock implementation - return flash write count
    return g_mock_state.flash_write_count;
}

// System functions
void system_init(void)
{
    // Mock implementation - does nothing
}

// Storage functions
boot_error_t storage_write_sensor_data(const sensor_data_t* data)
{
    if (data == NULL) {
        return BOOT_METADATA_CORRUPT;
    }
    
    // Ensure flash storage is initialized
    if (!g_mock_state.flash_init_done) {
        flash_storage_init();
    }
    
    // Mock implementation - write to flash storage
    uint32_t addr = STORAGE_FLASH_BASE + 0x2000; // Offset for sensor data
    return flash_storage_write(addr, (uint8_t*)data, sizeof(sensor_data_t));
}

boot_error_t storage_read_sensor_data(uint32_t index, sensor_data_t* data)
{
    if (data == NULL) {
        return BOOT_METADATA_CORRUPT;
    }
    
    // Ensure flash storage is initialized
    if (!g_mock_state.flash_init_done) {
        flash_storage_init();
    }
    
    // Mock implementation - read from flash storage
    uint32_t addr = STORAGE_FLASH_BASE + 0x2000 + (index * sizeof(sensor_data_t));
    return flash_storage_read(addr, (uint8_t*)data, sizeof(sensor_data_t));
}

boot_error_t storage_get_latest_data(sensor_data_t* data)
{
    if (data == NULL) {
        return BOOT_METADATA_CORRUPT;
    }
    
    // Mock implementation - just read index 0
    return storage_read_sensor_data(0, data);
}

boot_error_t storage_clear_all(void)
{
    // Mock implementation - erase the storage area
    return flash_storage_erase_sector(STORAGE_FLASH_BASE + 0x2000);
}

// Global variables definitions (already declared in freertos_mock.h)
// These are defined here to avoid multiple definition errors
QueueHandle_t sensor_queue;
QueueHandle_t uart_rx_queue;
QueueHandle_t data_queue;
SemaphoreHandle_t flash_mutex;
SemaphoreHandle_t uart_mutex;

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

void mock_flash_write(uint32_t addr, uint8_t* data, uint32_t len)
{
    if (addr < STORAGE_FLASH_BASE || addr + len > STORAGE_FLASH_BASE + sizeof(g_mock_state.flash_memory)) {
        return;
    }
    
    uint32_t offset = addr - STORAGE_FLASH_BASE;
    memcpy(&g_mock_state.flash_memory[offset], data, len);
}

void mock_flash_read(uint32_t addr, uint8_t* data, uint32_t len)
{
    if (addr < STORAGE_FLASH_BASE || addr + len > STORAGE_FLASH_BASE + sizeof(g_mock_state.flash_memory)) {
        return;
    }
    
    uint32_t offset = addr - STORAGE_FLASH_BASE;
    memcpy(data, &g_mock_state.flash_memory[offset], len);
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
