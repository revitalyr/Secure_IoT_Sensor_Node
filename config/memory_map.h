#pragma once

#include <stdint.h>

// OTA Metadata Structure
typedef struct {
    uint32_t active_slot;     // 0 = Slot A, 1 = Slot B
    uint32_t update_pending;  // 0 = no, 1 = yes
    uint32_t crc;            // CRC32 of the firmware
    uint32_t size;           // Firmware size in bytes
    uint32_t version;        // Firmware version
    uint32_t reserved[3];    // Reserved for future use
} __attribute__((packed)) ota_metadata_t;

// Communication Packet Structure
typedef struct {
    uint8_t header;
    uint8_t len;
    uint8_t payload[MAX_PACKET_SIZE];
    uint16_t crc;
} __attribute__((packed)) packet_t;

// Sensor Data Structure
typedef struct {
    float temperature;
    uint32_t timestamp;
    uint8_t status;
} __attribute__((packed)) sensor_data_t;

// Storage Entry Structure
typedef struct {
    sensor_data_t data;
    uint32_t crc;
} __attribute__((packed)) storage_entry_t;

// Flash Write Protection
typedef struct {
    uint32_t sector;
    uint32_t write_count;
    uint32_t last_erase;
} __attribute__((packed)) wear_entry_t;

// Function Pointers for Application Jump
typedef void (*pFunction)(void);

// Error Codes
typedef enum {
    BOOT_OK = 0,
    BOOT_NO_VALID_FIRMWARE,
    BOOT_SIGNATURE_INVALID,
    BOOT_CRC_INVALID,
    BOOT_FLASH_ERROR,
    BOOT_METADATA_CORRUPT
} boot_error_t;

// OTA Status
typedef enum {
    OTA_IDLE = 0,
    OTA_RECEIVING,
    OTA_VERIFYING,
    OTA_COMPLETE,
    OTA_ERROR
} ota_status_t;
