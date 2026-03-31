#pragma once

#include "system_config.h"
#include <stdint.h>

void spi_init(void);
void spi_dma_init(void);
boot_error_t spi_dma_read(uint8_t* tx_data, uint8_t* rx_data, uint16_t len);
boot_error_t spi_dma_write(uint8_t* data, uint16_t len);
uint8_t spi_is_busy(void);
void spi_set_cs(uint8_t state);
void spi_task(void *arg);

// Callbacks
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);

// External task handle
extern TaskHandle_t spi_task_handle;
