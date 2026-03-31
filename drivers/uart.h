#pragma once

#include "system_config.h"
#include <stdint.h>

void uart_init(void);
void uart_dma_init(void);
void uart_write(uint8_t* data, uint16_t len);
void uart_write_string(const char* str);
uint16_t uart_read(uint8_t* buffer, uint16_t max_len);
uint16_t uart_read_line(char* buffer, uint16_t max_len);
uint16_t uart_dma_get_rx(uint8_t* out, uint16_t* len);
void uart_process_rx_data(uint8_t* data, uint16_t len);

// IRQ handlers
void USART2_IRQHandler(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
