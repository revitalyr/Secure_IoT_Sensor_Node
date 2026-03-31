/**
 * @file bootloader.h
 * @brief Secure bootloader interface for firmware verification and loading
 * 
 * This header defines the bootloader interface, firmware verification functions,
 * and secure boot procedures for the Secure IoT Sensor Node with SHA256
 * hash verification and digital signature validation.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "system_config.h"
#include "memory_map.h"
#include <stdint.h>

/* Bootloader Functions */

/**
 * @brief Initialize bootloader subsystem
 * 
 * Sets up bootloader hardware, initializes verification systems,
 * and prepares for firmware loading operations.
 * 
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t bootloader_init(void);

/**
 * @brief Main bootloader routine
 * 
 * Executes the main bootloader logic including firmware verification,
 * OTA update processing, and application jumping.
 * 
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t bootloader_main(void);

/**
 * @brief Verify firmware integrity
 * 
 * Verifies firmware image using SHA256 hash validation and
 * checks for corruption or tampering.
 * 
 * @param addr Firmware start address
 * @param size Firmware size in bytes
 * @return boot_error_t BOOT_OK if valid, error code on failure
 */
boot_error_t verify_firmware(uint32_t addr, uint32_t size);

/**
 * @brief Verify digital signature
 * 
 * Verifies firmware digital signature using embedded public key
 * to ensure firmware authenticity.
 * 
 * @param hash SHA256 hash of firmware
 * @param signature Digital signature to verify
 * @return boot_error_t BOOT_OK if valid, error code on failure
 */
boot_error_t verify_signature(uint8_t* hash, uint8_t* signature);

/**
 * @brief Jump to application
 * 
 * Transfers control to application firmware at specified address
 * after successful verification.
 * 
 * @param addr Application start address
 */
void jump_to_application(uint32_t addr);

/**
 * @brief Reset the system
 * 
 * Performs a system reset to restart the bootloader or application.
 */
void system_reset(void);

/* Flash Interface Functions */

/**
 * @brief Erase flash sector
 * 
 * Erases a specified flash sector for firmware programming.
 * 
 * @param sector Flash sector number
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t flash_erase_sector(uint32_t sector);

/**
 * @brief Write word to flash
 * 
 * Writes a 32-bit word to flash memory at specified address.
 * 
 * @param addr Flash address
 * @param data 32-bit data to write
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t flash_write_word(uint32_t addr, uint32_t data);

/**
 * @brief Read word from flash
 * 
 * Reads a 32-bit word from flash memory at specified address.
 * 
 * @param addr Flash address
 * @param data Pointer to store read data
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t flash_read_word(uint32_t addr, uint32_t* data);

/* Cryptographic Functions */

/**
 * @brief Compute SHA256 hash
 * 
 * Computes SHA256 hash of data for firmware verification.
 * 
 * @param data Pointer to input data
 * @param size Size of data in bytes
 * @param hash Output hash buffer (32 bytes)
 */
void compute_sha256(uint8_t* data, uint32_t size, uint8_t* hash);

/**
 * @brief Compute CRC32 checksum
 * 
 * Computes CRC32 checksum of flash region for validation.
 * 
 * @param addr Start address
 * @param size Size in bytes
 * @return uint32_t Computed CRC32 value
 */
uint32_t compute_crc32(uint32_t addr, uint32_t size);

/**
 * @brief Compute CRC16 checksum
 * 
 * Computes CRC16 checksum of data buffer.
 * 
 * @param data Pointer to data buffer
 * @param len Length in bytes
 * @return uint16_t Computed CRC16 value
 */
uint16_t compute_crc16(uint8_t* data, uint16_t len);

/* Metadata Functions */

/**
 * @brief Read OTA metadata
 * 
 * Reads OTA update metadata from flash storage.
 * 
 * @param meta Pointer to store metadata
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t metadata_read(ota_metadata_t* meta);

/**
 * @brief Write OTA metadata
 * 
 * Writes OTA update metadata to flash storage.
 * 
 * @param meta Pointer to metadata structure
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t metadata_write(const ota_metadata_t* meta);

/**
 * @brief Initialize metadata subsystem
 * 
 * Initializes OTA metadata management system.
 * 
 * @return boot_error_t BOOT_OK on success, error code on failure
 */
boot_error_t metadata_init(void);

/* Utility Functions */

/**
 * @brief Millisecond delay
 * 
 * Provides blocking delay in milliseconds.
 * 
 * @param ms Delay duration in milliseconds
 */
void delay_ms(uint32_t ms);

/**
 * @brief Set LED status
 * 
 * Controls system status LED.
 * 
 * @param on LED state (1=on, 0=off)
 */
void led_set_status(uint8_t on);

/**
 * @brief Blink LED
 * 
 * Blinks LED specified number of times.
 * 
 * @param count Number of blinks
 * @param delay_ms Delay between blinks
 */
void led_blink(uint8_t count, uint32_t delay_ms);

#endif // BOOTLOADER_H
