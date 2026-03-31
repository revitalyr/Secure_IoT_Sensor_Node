/**
 * @file ota_metadata.h
 * @brief OTA metadata management interface
 * 
 * This header defines the OTA metadata management interface for
 * handling firmware update state, slot management, and update
 * tracking with persistent storage in flash memory.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef OTA_METADATA_H
#define OTA_METADATA_H

#include "system_config.h"
#include "memory_map.h"
#include <stdint.h>

/**
 * @brief Initialize OTA metadata subsystem
 * 
 * Sets up OTA metadata management system, reads existing metadata
 * from flash, and initializes internal state.
 * 
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t ota_metadata_init(void);

/**
 * @brief Read OTA metadata from flash
 * 
 * Retrieves OTA metadata from persistent flash storage.
 * 
 * @param meta Pointer to store metadata structure
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t ota_metadata_read(ota_metadata_t* meta);

/**
 * @brief Write OTA metadata to flash
 * 
 * Stores OTA metadata to persistent flash storage with validation.
 * 
 * @param meta Pointer to metadata structure
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t ota_metadata_write(const ota_metadata_t* meta);

/**
 * @brief Set OTA update pending state
 * 
 * Marks firmware update as pending with specified parameters.
 * 
 * @param crc Firmware CRC32 checksum
 * @param size Firmware size in bytes
 * @param version Firmware version number
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t ota_metadata_set_pending(uint32_t crc, uint32_t size, uint32_t version);

/**
 * @brief Commit OTA update to specified slot
 * 
 * Commits firmware update to active slot and clears pending state.
 * 
 * @param slot Target firmware slot (0 or 1)
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t ota_metadata_commit(uint32_t slot);

/**
 * @brief Rollback to previous firmware slot
 * 
 * Switches active firmware slot to previous version for recovery.
 * 
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t ota_metadata_rollback(void);

/**
 * @brief Get currently active firmware slot
 * 
 * Returns the index of the currently active firmware slot.
 * 
 * @return uint32_t Active slot number (0 or 1)
 */
uint32_t ota_metadata_get_active_slot(void);

/**
 * @brief Check if update is pending
 * 
 * Returns status of pending firmware update.
 * 
 * @return uint8_t 1 if update pending, 0 otherwise
 */
uint8_t ota_metadata_is_update_pending(void);

#endif // OTA_METADATA_H
