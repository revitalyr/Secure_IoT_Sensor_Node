/**
 * @file comm_task.h
 * @brief Communication task interface for UART data transmission
 * 
 * This header defines the communication task interface, packet structures,
 * and public API functions for handling UART communication with CRC-protected
 * binary protocol.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef COMM_TASK_H
#define COMM_TASK_H

#include "system_config.h"
#include <stdint.h>

/**
 * @brief Communication task function
 * 
 * FreeRTOS task that handles UART communication, packet processing,
 * and data transmission with CRC validation.
 * 
 * @param arg Task argument (unused)
 */
void comm_task(void *arg);

/**
 * @brief Initialize communication subsystem
 * 
 * Sets up UART hardware, initializes communication buffers,
 * and prepares the communication task for operation.
 */
void comm_init(void);

/**
 * @brief Send data packet via UART
 * 
 * Transmits a data packet with CRC16 validation over UART.
 * The function handles packet framing and error detection.
 * 
 * @param data Pointer to data buffer
 * @param len Length of data in bytes
 */
void comm_send_packet(uint8_t* data, uint16_t len);

/**
 * @brief Compute CRC16 checksum
 * 
 * Calculates CRC16 checksum for data validation using
 * standard polynomial for error detection in communication.
 * 
 * @param data Pointer to data buffer
 * @param len Length of data in bytes
 * @return uint16_t Computed CRC16 value
 */
uint16_t comm_compute_crc16(uint8_t* data, uint16_t len);

#endif // COMM_TASK_H
