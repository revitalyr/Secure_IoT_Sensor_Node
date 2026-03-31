#pragma once

#include "system_config.h"
#include <stdint.h>

void storage_task(void *arg);
void storage_init(void);
boot_error_t storage_write_entry(const sensor_data_t* data);
boot_error_t storage_read_entry(uint32_t index, sensor_data_t* data);
boot_error_t storage_get_latest(sensor_data_t* data);
uint32_t storage_get_count(void);
