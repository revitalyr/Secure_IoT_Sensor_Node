#pragma once

#include "system_config.h"
#include <stdint.h>

void update_task(void *arg);
void update_init(void);
boot_error_t ota_set_pending(uint32_t crc, uint32_t size);
boot_error_t ota_process_chunk(uint8_t* data, uint16_t len);
ota_status_t ota_get_status(void);
