#pragma once

#include "system_config.h"
#include <stdint.h>

void comm_task(void *arg);
void comm_init(void);
void comm_send_packet(uint8_t* data, uint16_t len);
uint16_t comm_compute_crc16(uint8_t* data, uint16_t len);
