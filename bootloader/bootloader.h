#pragma once

#include "system_config.h"
#include "memory_map.h"
#include <stdint.h>

// Bootloader functions
boot_error_t bootloader_init(void);
boot_error_t bootloader_main(void);
boot_error_t verify_firmware(uint32_t addr, uint32_t size);
boot_error_t verify_signature(uint8_t* hash, uint8_t* signature);
void jump_to_application(uint32_t addr);
void system_reset(void);

// Flash interface functions
boot_error_t flash_erase_sector(uint32_t sector);
boot_error_t flash_write_word(uint32_t addr, uint32_t data);
boot_error_t flash_read_word(uint32_t addr, uint32_t* data);

// Cryptographic functions
void compute_sha256(uint8_t* data, uint32_t size, uint8_t* hash);
uint32_t compute_crc32(uint32_t addr, uint32_t size);
uint16_t compute_crc16(uint8_t* data, uint16_t len);

// Metadata functions
boot_error_t metadata_read(ota_metadata_t* meta);
boot_error_t metadata_write(const ota_metadata_t* meta);
boot_error_t metadata_init(void);

// Utility functions
void delay_ms(uint32_t ms);
void led_set_status(uint8_t on);
void led_blink(uint8_t count, uint32_t delay_ms);
