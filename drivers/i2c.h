#pragma once

#include "system_config.h"
#include <stdint.h>

void i2c_init(void);
uint8_t i2c_read_byte(uint8_t dev_addr, uint8_t reg_addr);
uint16_t i2c_read_word(uint8_t dev_addr, uint8_t reg_addr);
boot_error_t i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint16_t len);
boot_error_t i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
boot_error_t i2c_write_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint16_t len);
float i2c_read_temperature(void);
