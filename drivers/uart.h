/**
 * @file uart.h
 * @brief UART driver interface for STM32F4 communication
 * 
 * This header defines the UART driver interface, DMA operations,
 * and communication functions for high-speed serial communication
 * with interrupt-driven and DMA-based data transfer.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef UART_H
#define UART_H

#include "system_config.h"
#include <stdint.h>

/**
 * @brief Initialize UART peripheral
 * 
 * Configures UART hardware, sets up communication parameters,
 * and prepares for data transmission and reception.
 */
void uart_init(void);

/**
 * @brief Initialize UART DMA channels
 * 
 * Sets up DMA channels for high-speed UART data transfer
 * with minimal CPU intervention.
 */
void uart_dma_init(void);

/**
 * @brief Write data to UART
 * 
 * Transmits data buffer via UART using blocking or DMA mode.
 * 
 * @param data Pointer to data buffer
 * @param len Length of data in bytes
 */
void uart_write(uint8_t* data, uint16_t len);

/**
 * @brief Write string to UART
 * 
 * Transmits null-terminated string via UART.
 * 
 * @param str Pointer to string
 */
void uart_write_string(const char* str);

/**
 * @brief Read data from UART
 * 
 * Receives data from UART into buffer with timeout.
 * 
 * @param buffer Pointer to receive buffer
 * @param max_len Maximum buffer size
 * @return uint16_t Number of bytes received
 */
uint16_t uart_read(uint8_t* buffer, uint16_t max_len);

/**
 * @brief Read line from UART
 * 
 * Receives text line ending with newline character.
 * 
 * @param buffer Pointer to receive buffer
 * @param max_len Maximum buffer size
 * @return uint16_t Number of bytes received
 */
uint16_t uart_read_line(char* buffer, uint16_t max_len);

/**
 * @brief Get DMA received data
 * 
 * Retrieves data received via DMA channel.
 * 
 * @param out Output buffer pointer
 * @param len Pointer to store data length
 * @return uint16_t Status flag
 */
uint16_t uart_dma_get_rx(uint8_t* out, uint16_t* len);

/**
 * @brief Process received UART data
 * 
 * Handles incoming UART data processing and protocol parsing.
 * 
 * @param data Pointer to received data
 * @param len Length of data
 */
void uart_process_rx_data(uint8_t* data, uint16_t len);

#endif // UART_H
void USART2_IRQHandler(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
