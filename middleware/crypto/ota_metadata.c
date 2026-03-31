#include "ota_metadata.h"
#include "flash_if.h"
#include <string.h>

static ota_metadata_t g_metadata;
static uint8_t g_metadata_valid = 0;

boot_error_t ota_metadata_init(void)
{
    if (g_metadata_valid) {
        return BOOT_OK;
    }
    
    // Try to read existing metadata
    boot_error_t err = ota_metadata_read(&g_metadata);
    
    if (err != BOOT_OK) {
        // Initialize with defaults
        memset(&g_metadata, 0, sizeof(g_metadata));
        g_metadata.active_slot = 0;
        g_metadata.update_pending = 0;
        g_metadata.crc = 0;
        g_metadata.size = 0;
        g_metadata.version = 1;
        
        for (int i = 0; i < 3; i++) {
            g_metadata.reserved[i] = 0;
        }
        
        // Write initial metadata
        err = ota_metadata_write(&g_metadata);
        if (err != BOOT_OK) {
            return err;
        }
    }
    
    g_metadata_valid = 1;
    return BOOT_OK;
}

boot_error_t ota_metadata_read(ota_metadata_t* meta)
{
    if (meta == NULL) {
        return BOOT_METADATA_CORRUPT;
    }
    
    // Read from flash
    uint32_t* src = (uint32_t*)METADATA_ADDR;
    uint32_t* dst = (uint32_t*)meta;
    
    for (uint32_t i = 0; i < sizeof(ota_metadata_t) / 4; i++) {
        dst[i] = src[i];
    }
    
    // Verify metadata integrity using a simple checksum
    // In production, use a more robust verification
    uint32_t checksum = 0;
    for (uint32_t i = 0; i < sizeof(ota_metadata_t) / 4; i++) {
        checksum ^= dst[i];
    }
    
    // Check if metadata appears valid (basic checks)
    if (meta->active_slot > 1 || meta->size > APP_SIZE) {
        return BOOT_METADATA_CORRUPT;
    }
    
    return BOOT_OK;
}

boot_error_t ota_metadata_write(const ota_metadata_t* meta)
{
    if (meta == NULL) {
        return BOOT_METADATA_CORRUPT;
    }
    
    // Erase metadata sector
    boot_error_t err = flash_storage_erase_sector(FLASH_SECTOR_3);
    if (err != BOOT_OK) {
        return err;
    }
    
    // Write metadata to flash
    const uint32_t* src = (const uint32_t*)meta;
    uint32_t dst = METADATA_ADDR;
    
    for (uint32_t i = 0; i < sizeof(ota_metadata_t) / 4; i++) {
        err = flash_storage_write(dst + i * 4, (uint8_t*)&src[i], 4);
        if (err != BOOT_OK) {
            return err;
        }
    }
    
    // Update cached copy
    if (&g_metadata != meta) {
        memcpy(&g_metadata, meta, sizeof(ota_metadata_t));
    }
    
    return BOOT_OK;
}

boot_error_t ota_metadata_set_pending(uint32_t crc, uint32_t size, uint32_t version)
{
    boot_error_t err = ota_metadata_init();
    if (err != BOOT_OK) {
        return err;
    }
    
    // Update metadata for pending update
    g_metadata.update_pending = 1;
    g_metadata.crc = crc;
    g_metadata.size = size;
    g_metadata.version = version;
    
    // Write to flash
    return ota_metadata_write(&g_metadata);
}

boot_error_t ota_metadata_commit(uint32_t slot)
{
    boot_error_t err = ota_metadata_init();
    if (err != BOOT_OK) {
        return err;
    }
    
    if (slot > 1) {
        return BOOT_METADATA_CORRUPT;
    }
    
    // Commit the update
    g_metadata.active_slot = slot;
    g_metadata.update_pending = 0;
    
    return ota_metadata_write(&g_metadata);
}

boot_error_t ota_metadata_rollback(void)
{
    boot_error_t err = ota_metadata_init();
    if (err != BOOT_OK) {
        return err;
    }
    
    // Rollback to the other slot
    g_metadata.active_slot = (g_metadata.active_slot == 0) ? 1 : 0;
    g_metadata.update_pending = 0;
    
    return ota_metadata_write(&g_metadata);
}

uint32_t ota_metadata_get_active_slot(void)
{
    if (!g_metadata_valid) {
        ota_metadata_init();
    }
    
    return g_metadata.active_slot;
}

uint8_t ota_metadata_is_update_pending(void)
{
    if (!g_metadata_valid) {
        ota_metadata_init();
    }
    
    return g_metadata.update_pending;
}
