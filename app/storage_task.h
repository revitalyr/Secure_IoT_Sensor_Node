/**
 * @file storage_task.h
 * @brief Storage task interface for flash data management
 * 
 * This header defines the storage task interface, data structures,
 * and public API functions for managing sensor data storage in flash
 * memory with wear-leveling and circular buffer implementation.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef STORAGE_TASK_H
#define STORAGE_TASK_H

#include "system_config.h"
#include <stdint.h>

/**
 * @brief Storage task function
 * 
 * FreeRTOS task that handles flash storage operations, wear-leveling,
 * and circular buffer management for sensor data persistence.
 * 
 * @param arg Task argument (unused)
 */
void storage_task(void *arg);

/**
 * @brief Initialize storage subsystem
 * 
 * Sets up flash storage, initializes wear-leveling algorithm,
 * and prepares the storage task for operation.
 */
void storage_init(void);

/**
 * @brief Write sensor data entry to storage
 * 
 * Stores sensor data entry in flash with wear-leveling and
 * circular buffer management.
 * 
 * @param data Pointer to sensor data structure
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t storage_write_entry(const sensor_data_t* data);

/**
 * @brief Read sensor data entry from storage
 * 
 * Retrieves sensor data entry from flash storage by index.
 * 
 * @param index Entry index in storage
 * @param data Pointer to store retrieved data
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t storage_read_entry(uint32_t index, sensor_data_t* data);

/**
 * @brief Get latest sensor data entry
 * 
 * Retrieves the most recent sensor data entry from storage.
 * 
 * @param data Pointer to store retrieved data
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t storage_get_latest(sensor_data_t* data);

/**
 * @brief Get total stored entries count
 * 
 * Returns the total number of sensor data entries currently
 * stored in flash memory.
 * 
 * @return uint32_t Number of stored entries
 */
uint32_t storage_get_count(void);

#endif // STORAGE_TASK_H
