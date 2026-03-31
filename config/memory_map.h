/**
 * @file memory_map.h
 * @brief Memory map definitions and data structures for Secure IoT Sensor Node
 * 
 * This header defines memory layout, data structures, and address mappings
 * for the STM32F4-based IoT sensor node including flash sectors, OTA metadata,
 * and communication packet formats.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include <stdint.h>

/**
 * @brief OTA metadata structure
 * 
 * Structure containing OTA update metadata including active slot,
 * update status, firmware information, and version tracking.
 * Uses packed attribute for flash storage compatibility.
 */
typedef struct {
    uint32_t active_slot;     /**< Active firmware slot (0=Slot A, 1=Slot B) */
    uint32_t update_pending;  /**< Update pending flag (0=no, 1=yes) */
    uint32_t crc;            /**< CRC32 checksum of firmware image */
    uint32_t size;           /**< Firmware size in bytes */
    uint32_t version;        /**< Firmware version number */
    uint32_t reserved[3];    /**< Reserved fields for future use */
} __attribute__((packed)) ota_metadata_t;

/**
 * @brief Communication packet structure
 * 
 * Binary packet format for UART communication with header,
 * length, payload, and CRC16 validation.
 */
typedef struct {
    uint8_t header;                         /**< Packet header byte */
    uint8_t len;                            /**< Payload length */
    uint8_t payload[MAX_PACKET_SIZE];      /**< Data payload buffer */
    uint16_t crc;                          /**< CRC16 checksum */
} __attribute__((packed)) packet_t;

/**
 * @brief Sensor data structure
 * 
 * Compact sensor data structure with temperature reading,
 * timestamp, and status information.
 */
typedef struct {
    float temperature;      /**< Temperature value in Celsius */
    uint32_t timestamp;    /**< Unix timestamp */
    uint8_t status;        /**< Sensor status flags */
} __attribute__((packed)) sensor_data_t;

/**
 * @brief Storage entry structure
 * 
 * Flash storage entry combining sensor data with CRC
 * validation for data integrity.
 */
typedef struct {
    sensor_data_t data;    /**< Sensor data */
    uint32_t crc;         /**< CRC32 checksum */
} __attribute__((packed)) storage_entry_t;

/**
 * @brief Wear-leveling entry structure
 * 
 * Structure for tracking flash wear statistics including
 * sector usage, write counts, and erase cycles.
 */
typedef struct {
    uint32_t sector;       /**< Flash sector number */
    uint32_t write_count;  /**< Number of write operations */
    uint32_t last_erase;   /**< Last erase timestamp */
} __attribute__((packed)) wear_entry_t;

/**
 * @brief Application jump table structure
 * 
 * Function pointer structure for application jumping
 * and bootloader-to-application transition.
 */
typedef struct {
    uint32_t stack_ptr;    /**< Initial stack pointer */
    uint32_t reset_vec;    /**< Reset vector address */
} __attribute__((packed)) app_jump_table_t;

/**
 * @brief Function pointer type
 * 
 * Generic function pointer type for application jumping.
 */
typedef void (*pFunction)(void);

/**
 * @brief Boot error codes enumeration
 * 
 * Error codes for bootloader operations and firmware verification.
 */
typedef enum {
    BOOT_OK = 0,                    /**< Operation successful */
    BOOT_NO_VALID_FIRMWARE,        /**< No valid firmware found */
    BOOT_SIGNATURE_INVALID,        /**< Digital signature invalid */
    BOOT_CRC_INVALID,             /**< CRC checksum invalid */
    BOOT_FLASH_ERROR,             /**< Flash operation error */
    BOOT_METADATA_CORRUPT         /**< Metadata corruption detected */
} boot_error_t;

/**
 * @brief OTA status enumeration
 * 
 * Status codes for OTA update process tracking.
 */
typedef enum {
    OTA_IDLE = 0,        /**< No update in progress */
    OTA_RECEIVING,       /**< Receiving firmware data */
    OTA_VERIFYING,       /**< Verifying firmware integrity */
    OTA_COMPLETE,        /**< Update completed successfully */
    OTA_ERROR           /**< Update failed */
} ota_status_t;

#endif // MEMORY_MAP_H
