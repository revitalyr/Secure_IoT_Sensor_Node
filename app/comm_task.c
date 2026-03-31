/**
 * @file comm_task.c
 * @brief Communication task implementation for UART data transmission
 * 
 * This file implements the communication task functionality including
 * UART packet processing, CRC validation, and data transmission
 * with FreeRTOS integration.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#include "comm_task.h"
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>
#include <stdio.h>

extern QueueHandle_t sensor_queue;
extern SemaphoreHandle_t uart_mutex;

/**
 * @brief Communication packet structure
 * 
 * Defines the binary packet format for UART communication with
 * header, length, data payload, and CRC16 validation.
 */
typedef struct {
    uint8_t header;         /**< Packet header byte */
    uint16_t length;        /**< Data payload length */
    uint8_t data[256];      /**< Data payload buffer */
    uint16_t crc;          /**< CRC16 checksum */
} comm_packet_t;

// Packet types
#define PACKET_TYPE_SENSOR_DATA    0x01
#define PACKET_TYPE_STATUS         0x02
#define PACKET_TYPE_ERROR          0x03
#define PACKET_TYPE_ACK            0x04

void comm_init(void)
{
    // UART is already initialized in system_init()
    // Additional communication-specific initialization can go here
}

uint16_t comm_compute_crc16(uint8_t* data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (CRC16_POLY * (crc & 1));
        }
    }
    
    return crc;
}

void comm_send_packet(uint8_t* data, uint16_t len)
{
    if (xSemaphoreTake(uart_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        uart_write(data, len);
        xSemaphoreGive(uart_mutex);
    }
}

void comm_task(void *arg)
{
    float temperature;
    comm_packet_t packet;
    uint32_t packet_count = 0;
    
    // Initialize communication
    comm_init();
    
    for (;;)
    {
        // Wait for sensor data
        if (xQueueReceive(sensor_queue, &temperature, portMAX_DELAY) == pdTRUE) {
            // Build packet
            packet.header = PACKET_HEADER;
            packet.type = PACKET_TYPE_SENSOR_DATA;
            packet.len = sizeof(float) + sizeof(uint32_t);
            packet.payload = temperature;
            packet.timestamp = xTaskGetTickCount();
            
            // Compute CRC (excluding CRC field itself)
            packet.crc = comm_compute_crc16((uint8_t*)&packet, sizeof(packet) - 2);
            
            // Send packet
            comm_send_packet((uint8_t*)&packet, sizeof(packet));
            
            packet_count++;
            
            // Send status packet every 100 sensor readings
            if (packet_count % 100 == 0) {
                comm_packet_t status_packet = {
                    .header = PACKET_HEADER,
                    .type = PACKET_TYPE_STATUS,
                    .len = 4,
                    .payload = (float)packet_count,
                    .timestamp = xTaskGetTickCount(),
                    .crc = 0
                };
                
                status_packet.crc = comm_compute_crc16((uint8_t*)&status_packet, 
                                                      sizeof(status_packet) - 2);
                comm_send_packet((uint8_t*)&status_packet, sizeof(status_packet));
            }
        }
    }
}
