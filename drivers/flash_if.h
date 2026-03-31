#pragma once

#include "system_config.h"
#include "memory_map.h"
#include <stdint.h>

// Flash storage interface
boot_error_t flash_storage_init(void);
boot_error_t flash_storage_write(uint32_t addr, uint8_t* data, uint32_t len);
boot_error_t flash_storage_read(uint32_t addr, uint8_t* data, uint32_t len);
boot_error_t flash_storage_erase_sector(uint32_t sector);

// Wear leveling functions
boot_error_t wear_level_init(void);
boot_error_t wear_level_write(uint8_t* data, uint32_t len);
boot_error_t wear_level_read(uint32_t logical_addr, uint8_t* data, uint32_t len);
uint32_t wear_level_get_free_space(void);
uint32_t wear_level_get_write_count(uint32_t logical_addr);

// High-level storage functions
boot_error_t storage_write_sensor_data(const sensor_data_t* data);
boot_error_t storage_read_sensor_data(uint32_t index, sensor_data_t* data);
boot_error_t storage_get_latest_data(sensor_data_t* data);
boot_error_t storage_clear_all(void);
