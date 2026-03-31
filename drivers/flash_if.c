#include "flash_if.h"
#include "stm32f4xx_hal.h"
#include <string.h>

// Flash storage configuration
#define STORAGE_FLASH_BASE    0x08080000  // Second half of Flash
#define STORAGE_SECTOR_START  FLASH_SECTOR_7
#define STORAGE_SECTOR_SIZE   0x20000     // 128KB
#define STORAGE_TOTAL_SIZE    STORAGE_SECTOR_SIZE

// Wear leveling configuration
#define WEAR_BLOCK_SIZE       1024        // 1KB blocks
#define WEAR_BLOCKS_TOTAL     (STORAGE_SECTOR_SIZE / WEAR_BLOCK_SIZE)
#define WEAR_MAX_CYCLES       10000       // Maximum erase cycles per block

// Wear leveling metadata
typedef struct {
    uint32_t logical_block;    // Logical block number
    uint32_t physical_block;   // Physical block number
    uint32_t erase_count;      // Number of erase cycles
    uint32_t is_valid;         // Block validity flag
    uint32_t crc;             // Metadata CRC
} __attribute__((packed)) wear_block_meta_t;

// Wear leveling state
static wear_block_meta_t wear_table[WEAR_BLOCKS_TOTAL];
static uint32_t wear_initialized = 0;
static uint32_t current_write_block = 0;
static uint32_t write_offset = 0;

// Metadata storage location
#define WEAR_META_ADDR        (STORAGE_FLASH_BASE + STORAGE_SECTOR_SIZE - sizeof(wear_table))

boot_error_t flash_storage_init(void)
{
    // No specific initialization needed for STM32F4 flash
    return BOOT_OK;
}

boot_error_t flash_storage_write(uint32_t addr, uint8_t* data, uint32_t len)
{
    if (addr < STORAGE_FLASH_BASE || 
        addr + len > STORAGE_FLASH_BASE + STORAGE_TOTAL_SIZE ||
        len == 0 || data == NULL) {
        return BOOT_FLASH_ERROR;
    }
    
    HAL_FLASH_Unlock();
    
    // Check if we need to erase first
    uint32_t end_addr = addr + len;
    for (uint32_t check_addr = addr; check_addr < end_addr; check_addr += 4) {
        uint32_t current_word = *(volatile uint32_t*)check_addr;
        uint32_t new_word = *(uint32_t*)(data + (check_addr - addr));
        
        // If any bit needs to change from 1 to 0, we need to erase
        if ((current_word & new_word) != new_word) {
            // This is a simplified check - in practice, you'd erase by sector
            break;
        }
    }
    
    // Write data word by word
    for (uint32_t i = 0; i < len; i += 4) {
        uint32_t word = 0;
        
        // Handle non-aligned end
        uint32_t remaining = len - i;
        if (remaining < 4) {
            memcpy(&word, data + i, remaining);
        } else {
            word = *(uint32_t*)(data + i);
        }
        
        HAL_StatusTypeDef status = HAL_FLASH_Program(TYPEPROGRAM_WORD, addr + i, word);
        if (status != HAL_OK) {
            HAL_FLASH_Lock();
            return BOOT_FLASH_ERROR;
        }
    }
    
    HAL_FLASH_Lock();
    return BOOT_OK;
}

boot_error_t flash_storage_read(uint32_t addr, uint8_t* data, uint32_t len)
{
    if (addr < STORAGE_FLASH_BASE || 
        addr + len > STORAGE_FLASH_BASE + STORAGE_TOTAL_SIZE ||
        len == 0 || data == NULL) {
        return BOOT_FLASH_ERROR;
    }
    
    memcpy(data, (uint8_t*)addr, len);
    return BOOT_OK;
}

boot_error_t flash_storage_erase_sector(uint32_t sector)
{
    HAL_FLASH_Unlock();
    
    FLASH_EraseInitTypeDef erase = {0};
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector = sector;
    erase.NbSectors = 1;
    
    uint32_t sector_error;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &sector_error);
    
    HAL_FLASH_Lock();
    
    return (status == HAL_OK) ? BOOT_OK : BOOT_FLASH_ERROR;
}

// Wear leveling implementation
boot_error_t wear_level_init(void)
{
    if (wear_initialized) {
        return BOOT_OK;
    }
    
    // Load wear table from flash
    boot_error_t err = flash_storage_read(WEAR_META_ADDR, (uint8_t*)wear_table, sizeof(wear_table));
    if (err != BOOT_OK) {
        // Initialize empty wear table
        memset(wear_table, 0, sizeof(wear_table));
        for (uint32_t i = 0; i < WEAR_BLOCKS_TOTAL; i++) {
            wear_table[i].logical_block = i;
            wear_table[i].physical_block = i;
            wear_table[i].erase_count = 0;
            wear_table[i].is_valid = 1;
            wear_table[i].crc = compute_crc16((uint8_t*)&wear_table[i], 
                                             sizeof(wear_block_meta_t) - 2);
        }
    }
    
    // Find block with minimum erase count for next write
    uint32_t min_erase = UINT32_MAX;
    for (uint32_t i = 0; i < WEAR_BLOCKS_TOTAL; i++) {
        if (wear_table[i].is_valid && wear_table[i].erase_count < min_erase) {
            min_erase = wear_table[i].erase_count;
            current_write_block = i;
        }
    }
    
    write_offset = 0;
    wear_initialized = 1;
    
    return BOOT_OK;
}

boot_error_t wear_level_write(uint8_t* data, uint32_t len)
{
    if (!wear_initialized) {
        wear_level_init();
    }
    
    if (len > WEAR_BLOCK_SIZE) {
        return BOOT_FLASH_ERROR;
    }
    
    // Check if current block has space
    if (write_offset + len > WEAR_BLOCK_SIZE) {
        // Move to next block with minimum wear
        uint32_t min_erase = UINT32_MAX;
        for (uint32_t i = 0; i < WEAR_BLOCKS_TOTAL; i++) {
            if (wear_table[i].is_valid && wear_table[i].erase_count < min_erase) {
                min_erase = wear_table[i].erase_count;
                current_write_block = i;
            }
        }
        
        // Erase the physical block
        uint32_t physical_addr = STORAGE_FLASH_BASE + 
                                wear_table[current_write_block].physical_block * WEAR_BLOCK_SIZE;
        
        // Calculate sector from physical address
        uint32_t sector = STORAGE_SECTOR_START + 
                         (wear_table[current_write_block].physical_block * WEAR_BLOCK_SIZE) / 
                         FLASH_SECTOR_SIZE;
        
        boot_error_t err = flash_storage_erase_sector(sector);
        if (err != BOOT_OK) {
            return err;
        }
        
        // Update erase count
        wear_table[current_write_block].erase_count++;
        write_offset = 0;
        
        // Save wear table
        boot_error_t save_err = flash_storage_write(WEAR_META_ADDR, (uint8_t*)wear_table, 
                                                    sizeof(wear_table));
        if (save_err != BOOT_OK) {
            return save_err;
        }
    }
    
    // Write data to current block
    uint32_t physical_addr = STORAGE_FLASH_BASE + 
                            wear_table[current_write_block].physical_block * WEAR_BLOCK_SIZE + 
                            write_offset;
    
    boot_error_t err = flash_storage_write(physical_addr, data, len);
    if (err == BOOT_OK) {
        write_offset += len;
    }
    
    return err;
}

boot_error_t wear_level_read(uint32_t logical_addr, uint8_t* data, uint32_t len)
{
    if (!wear_initialized) {
        wear_level_init();
    }
    
    uint32_t logical_block = logical_addr / WEAR_BLOCK_SIZE;
    uint32_t block_offset = logical_addr % WEAR_BLOCK_SIZE;
    
    if (logical_block >= WEAR_BLOCKS_TOTAL || block_offset + len > WEAR_BLOCK_SIZE) {
        return BOOT_FLASH_ERROR;
    }
    
    // Find physical block for this logical block
    uint32_t physical_block = UINT32_MAX;
    for (uint32_t i = 0; i < WEAR_BLOCKS_TOTAL; i++) {
        if (wear_table[i].logical_block == logical_block && wear_table[i].is_valid) {
            physical_block = wear_table[i].physical_block;
            break;
        }
    }
    
    if (physical_block == UINT32_MAX) {
        return BOOT_FLASH_ERROR;
    }
    
    uint32_t physical_addr = STORAGE_FLASH_BASE + physical_block * WEAR_BLOCK_SIZE + block_offset;
    return flash_storage_read(physical_addr, data, len);
}

uint32_t wear_level_get_free_space(void)
{
    if (!wear_initialized) {
        wear_level_init();
    }
    
    return WEAR_BLOCK_SIZE - write_offset;
}

uint32_t wear_level_get_write_count(uint32_t logical_addr)
{
    if (!wear_initialized) {
        wear_level_init();
    }
    
    uint32_t logical_block = logical_addr / WEAR_BLOCK_SIZE;
    
    for (uint32_t i = 0; i < WEAR_BLOCKS_TOTAL; i++) {
        if (wear_table[i].logical_block == logical_block && wear_table[i].is_valid) {
            return wear_table[i].erase_count;
        }
    }
    
    return 0;
}

// High-level storage functions
boot_error_t storage_write_sensor_data(const sensor_data_t* data)
{
    return wear_level_write((uint8_t*)data, sizeof(sensor_data_t));
}

boot_error_t storage_read_sensor_data(uint32_t index, sensor_data_t* data)
{
    return wear_level_read(index * sizeof(sensor_data_t), (uint8_t*)data, sizeof(sensor_data_t));
}

boot_error_t storage_get_latest_data(sensor_data_t* data)
{
    // This is simplified - in practice, you'd maintain an index
    return storage_read_sensor_data(0, data);
}

boot_error_t storage_clear_all(void)
{
    boot_error_t err = flash_storage_erase_sector(STORAGE_SECTOR_START);
    if (err != BOOT_OK) {
        return err;
    }
    
    // Reinitialize wear leveling
    wear_initialized = 0;
    return wear_level_init();
}
