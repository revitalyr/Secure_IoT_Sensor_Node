/**
 * @file update_task.c
 * @brief OTA update task implementation for firmware management
 * 
 * This file implements the OTA update task functionality including firmware
 * download, verification, installation, dual-bank management, and rollback
 * procedures with FreeRTOS integration.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#include "update_task.h"
#include "uart.h"
#include "flash_if.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>
#include <stdlib.h>

extern QueueHandle_t uart_rx_queue;
extern SemaphoreHandle_t flash_mutex;

/**
 * @brief OTA state machine enumeration
 * 
 * Internal state machine for managing the OTA update process
 * from initialization through completion or rollback.
 */
typedef enum {
    OTA_STATE_IDLE,
    OTA_STATE_WAITING_START,
    OTA_STATE_WAITING_SIZE,
    OTA_STATE_RECEIVING_DATA,
    OTA_STATE_VERIFYING,
    OTA_STATE_COMPLETE,
    OTA_STATE_ERROR
} ota_state_t;

static ota_state_t m_ota_state = OTA_STATE_IDLE;
static ota_status_t m_ota_status = OTA_IDLE;
static uint32_t m_expected_size = 0;
static uint32_t m_received_size = 0;
static uint32_t m_write_addr = APP_SLOT_B_ADDR;
static uint32_t m_firmware_crc = 0;
static uint8_t m_rx_buffer[OTA_CHUNK_SIZE];

// OTA commands
#define OTA_CMD_START    "START"
#define OTA_CMD_SIZE     "SIZE"
#define OTA_CMD_DATA     "DATA"
#define OTA_CMD_END      "END"
#define OTA_CMD_ABORT    "ABORT"

// OTA responses
#define OTA_RESP_OK      "OK\n"
#define OTA_RESP_ERROR   "ERROR\n"
#define OTA_RESP_READY   "READY\n"

void update_init(void)
{
    m_ota_state = OTA_STATE_IDLE;
    m_ota_status = OTA_IDLE;
    m_expected_size = 0;
    m_received_size = 0;
    m_write_addr = APP_SLOT_B_ADDR;
    m_firmware_crc = 0;
}

ota_status_t ota_get_status(void)
{
    return m_ota_status;
}

boot_error_t ota_set_pending(uint32_t crc, uint32_t size)
{
    ota_metadata_t meta;
    
    // Read current metadata
    boot_error_t err = metadata_read(&meta);
    if (err != BOOT_OK) {
        return err;
    }
    
    // Update metadata
    meta.update_pending = 1;
    meta.crc = crc;
    meta.size = size;
    meta.version++;
    
    // Write metadata
    err = metadata_write(&meta);
    if (err != BOOT_OK) {
        return err;
    }
    
    return BOOT_OK;
}

boot_error_t ota_process_chunk(uint8_t* data, uint16_t len)
{
    if (m_received_size + len > m_expected_size) {
        return BOOT_FLASH_ERROR;
    }
    
    if (xSemaphoreTake(flash_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        // Write chunk to flash
        uint32_t* src = (uint32_t*)data;
        for (uint32_t i = 0; i < len / 4; i++) {
            flash_write_word(m_write_addr + i * 4, src[i]);
        }
        
        // Handle remaining bytes
        for (uint32_t i = (len / 4) * 4; i < len; i++) {
            uint32_t word = data[i];
            flash_write_word(m_write_addr + i, word);
        }
        
        m_write_addr += len;
        m_received_size += len;
        
        xSemaphoreGive(flash_mutex);
        
        // Update CRC
        m_firmware_crc = compute_crc32(APP_SLOT_B_ADDR, m_received_size);
        
        return BOOT_OK;
    }
    
    return BOOT_FLASH_ERROR;
}

void update_task(void *arg)
{
    uint8_t byte;
    char cmd_buffer[32];
    uint8_t cmd_index = 0;
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // Check every 100ms
    
    // Initialize update system
    update_init();
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        // Process UART commands
        while (xQueueReceive(uart_rx_queue, &byte, 0) == pdTRUE) {
            if (byte == '\n' || byte == '\r') {
                if (cmd_index > 0) {
                    cmd_buffer[cmd_index] = '\0';
                    
                    // Process command
                    if (strncmp(cmd_buffer, OTA_CMD_START, 5) == 0) {
                        if (m_ota_state == OTA_STATE_IDLE) {
                            m_ota_state = OTA_STATE_WAITING_SIZE;
                            m_ota_status = OTA_RECEIVING;
                            m_received_size = 0;
                            m_write_addr = APP_SLOT_B_ADDR;
                            
                            // Erase application slot B
                            if (xSemaphoreTake(flash_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                                flash_erase_sector(FLASH_SECTOR_5);
                                flash_erase_sector(FLASH_SECTOR_6);
                                xSemaphoreGive(flash_mutex);
                            }
                            
                            uart_write((uint8_t*)OTA_RESP_READY, 6);
                        }
                    }
                    else if (strncmp(cmd_buffer, OTA_CMD_SIZE, 4) == 0) {
                        if (m_ota_state == OTA_STATE_WAITING_SIZE) {
                            m_expected_size = atoi(cmd_buffer + 5);
                            if (m_expected_size > 0 && m_expected_size <= APP_SIZE) {
                                m_ota_state = OTA_STATE_RECEIVING_DATA;
                                uart_write((uint8_t*)OTA_RESP_OK, 3);
                            } else {
                                m_ota_state = OTA_STATE_ERROR;
                                uart_write((uint8_t*)OTA_RESP_ERROR, 6);
                            }
                        }
                    }
                    else if (strncmp(cmd_buffer, OTA_CMD_DATA, 4) == 0) {
                        if (m_ota_state == OTA_STATE_RECEIVING_DATA) {
                            // Receive data chunk
                            uint16_t chunk_len = OTA_CHUNK_SIZE;
                            if (m_received_size + chunk_len > m_expected_size) {
                                chunk_len = m_expected_size - m_received_size;
                            }
                            
                            // Read chunk data
                            for (uint16_t i = 0; i < chunk_len; i++) {
                                if (xQueueReceive(uart_rx_queue, &m_rx_buffer[i], 
                                                 pdMS_TO_TICKS(100)) != pdTRUE) {
                                    m_ota_state = OTA_STATE_ERROR;
                                    uart_write((uint8_t*)OTA_RESP_ERROR, 6);
                                    break;
                                }
                            }
                            
                            if (m_ota_state != OTA_STATE_ERROR) {
                                if (ota_process_chunk(m_rx_buffer, chunk_len) == BOOT_OK) {
                                    uart_write((uint8_t*)OTA_RESP_OK, 3);
                                    
                                    if (m_received_size >= m_expected_size) {
                                        m_ota_state = OTA_STATE_VERIFYING;
                                    }
                                } else {
                                    m_ota_state = OTA_STATE_ERROR;
                                    uart_write((uint8_t*)OTA_RESP_ERROR, 6);
                                }
                            }
                        }
                    }
                    else if (strncmp(cmd_buffer, OTA_CMD_END, 3) == 0) {
                        if (m_ota_state == OTA_STATE_VERIFYING || 
                            m_ota_state == OTA_STATE_RECEIVING_DATA) {
                            
                            // Verify firmware
                            if (verify_firmware(APP_SLOT_B_ADDR, m_expected_size) == BOOT_OK) {
                                // Set pending flag for bootloader
                                if (ota_set_pending(m_firmware_crc, m_expected_size) == BOOT_OK) {
                                    m_ota_state = OTA_STATE_COMPLETE;
                                    m_ota_status = OTA_COMPLETE;
                                    uart_write((uint8_t*)OTA_RESP_OK, 3);
                                    
                                    // Wait a bit then reset
                                    vTaskDelay(pdMS_TO_TICKS(1000));
                                    system_reset();
                                } else {
                                    m_ota_state = OTA_STATE_ERROR;
                                    uart_write((uint8_t*)OTA_RESP_ERROR, 6);
                                }
                            } else {
                                m_ota_state = OTA_STATE_ERROR;
                                uart_write((uint8_t*)OTA_RESP_ERROR, 6);
                            }
                        }
                    }
                    else if (strncmp(cmd_buffer, OTA_CMD_ABORT, 5) == 0) {
                        m_ota_state = OTA_STATE_IDLE;
                        m_ota_status = OTA_IDLE;
                        uart_write((uint8_t*)OTA_RESP_OK, 3);
                    }
                    
                    cmd_index = 0;
                }
            } else if (cmd_index < sizeof(cmd_buffer) - 1) {
                cmd_buffer[cmd_index++] = byte;
            }
        }
        
        // Periodic maintenance
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        
        // Timeout handling
        if (m_ota_state != OTA_STATE_IDLE && m_ota_state != OTA_STATE_ERROR) {
            static uint32_t idle_count = 0;
            idle_count++;
            
            if (idle_count > 100) { // 10 seconds timeout
                m_ota_state = OTA_STATE_IDLE;
                m_ota_status = OTA_IDLE;
                idle_count = 0;
            }
        } else {
            idle_count = 0;
        }
    }
}
