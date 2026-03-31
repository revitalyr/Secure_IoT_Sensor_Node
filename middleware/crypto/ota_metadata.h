#pragma once

#include "system_config.h"
#include "memory_map.h"
#include <stdint.h>

boot_error_t ota_metadata_init(void);
boot_error_t ota_metadata_read(ota_metadata_t* meta);
boot_error_t ota_metadata_write(const ota_metadata_t* meta);
boot_error_t ota_metadata_set_pending(uint32_t crc, uint32_t size, uint32_t version);
boot_error_t ota_metadata_commit(uint32_t slot);
boot_error_t ota_metadata_rollback(void);
uint32_t ota_metadata_get_active_slot(void);
uint8_t ota_metadata_is_update_pending(void);
