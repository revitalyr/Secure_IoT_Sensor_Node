#include "bootloader.h"
#include "stm32f4xx_hal.h"

boot_error_t flash_erase_sector(uint32_t sector)
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

boot_error_t flash_write_word(uint32_t addr, uint32_t data)
{
    HAL_FLASH_Unlock();
    
    HAL_StatusTypeDef status = HAL_FLASH_Program(TYPEPROGRAM_WORD, addr, data);
    
    HAL_FLASH_Lock();
    
    return (status == HAL_OK) ? BOOT_OK : BOOT_FLASH_ERROR;
}

boot_error_t flash_read_word(uint32_t addr, uint32_t* data)
{
    *data = *(volatile uint32_t*)addr;
    return BOOT_OK;
}

boot_error_t metadata_read(ota_metadata_t* meta)
{
    uint32_t* src = (uint32_t*)METADATA_ADDR;
    uint32_t* dst = (uint32_t*)meta;
    
    for (uint32_t i = 0; i < sizeof(ota_metadata_t) / 4; i++) {
        dst[i] = src[i];
    }
    
    // Verify metadata integrity
    uint32_t crc = compute_crc32(METADATA_ADDR, sizeof(ota_metadata_t) - 4);
    if (crc != meta->reserved[0]) { // Using first reserved field for CRC
        return BOOT_METADATA_CORRUPT;
    }
    
    return BOOT_OK;
}

boot_error_t metadata_write(const ota_metadata_t* meta)
{
    // Erase metadata sector
    boot_error_t err = flash_erase_sector(FLASH_SECTOR_3);
    if (err != BOOT_OK) {
        return err;
    }
    
    // Write metadata
    const uint32_t* src = (const uint32_t*)meta;
    uint32_t dst = METADATA_ADDR;
    
    for (uint32_t i = 0; i < sizeof(ota_metadata_t) / 4; i++) {
        err = flash_write_word(dst + i * 4, src[i]);
        if (err != BOOT_OK) {
            return err;
        }
    }
    
    return BOOT_OK;
}

boot_error_t metadata_init(void)
{
    ota_metadata_t meta;
    boot_error_t err = metadata_read(&meta);
    
    // If metadata is invalid, initialize defaults
    if (err == BOOT_METADATA_CORRUPT) {
        meta.active_slot = 0;
        meta.update_pending = 0;
        meta.crc = 0;
        meta.size = 0;
        meta.version = 1;
        for (int i = 0; i < 3; i++) {
            meta.reserved[i] = 0;
        }
        
        err = metadata_write(&meta);
    }
    
    return err;
}
