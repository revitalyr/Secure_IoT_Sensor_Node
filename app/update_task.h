/**
 * @file update_task.h
 * @brief OTA update task interface for firmware management
 * 
 * This header defines the OTA update task interface, status structures,
 * and public API functions for managing over-the-air firmware updates
 * with dual-bank implementation and rollback capability.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef UPDATE_TASK_H
#define UPDATE_TASK_H

#include "system_config.h"
#include <stdint.h>

/**
 * @brief OTA update status enumeration
 * 
 * Defines the possible states of the OTA update process
 * from idle through completion or failure.
 */
typedef enum {
    OTA_STATUS_IDLE,          /**< No update in progress */
    OTA_STATUS_DOWNLOADING,   /**< Firmware download in progress */
    OTA_STATUS_VERIFYING,     /**< Firmware verification in progress */
    OTA_STATUS_INSTALLING,    /**< Firmware installation in progress */
    OTA_STATUS_COMPLETE,      /**< Update completed successfully */
    OTA_STATUS_FAILED         /**< Update failed */
} ota_status_t;

/**
 * @brief Update task function
 * 
 * FreeRTOS task that handles OTA update operations including
 * firmware download, verification, installation, and rollback.
 * 
 * @param arg Task argument (unused)
 */
void update_task(void *arg);

/**
 * @brief Initialize update subsystem
 * 
 * Sets up OTA update infrastructure, initializes metadata,
 * and prepares the update task for operation.
 */
void update_init(void);

/**
 * @brief Set OTA update pending state
 * 
 * Marks firmware update as pending with CRC and size information
 * for subsequent download and installation.
 * 
 * @param crc Firmware CRC32 checksum
 * @param size Firmware size in bytes
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t ota_set_pending(uint32_t crc, uint32_t size);

/**
 * @brief Process OTA firmware chunk
 * 
 * Processes a chunk of firmware data during OTA download,
 * writing to appropriate flash bank with validation.
 * 
 * @param data Pointer to firmware chunk data
 * @param len Length of chunk in bytes
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t ota_process_chunk(uint8_t* data, uint16_t len);

/**
 * @brief Get current OTA update status
 * 
 * Returns the current status of the OTA update process
 * including progress and error information.
 * 
 * @return ota_status_t Current update status
 */
ota_status_t ota_get_status(void);

#endif // UPDATE_TASK_H
