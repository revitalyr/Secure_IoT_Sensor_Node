#include "spi.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f4xx_hal.h"

extern QueueHandle_t data_queue;

// SPI handle
SPI_HandleTypeDef hspi1;

// Double buffering configuration
#define BUF_SIZE        256
static uint16_t dma_buffer[BUF_SIZE * 2]; // A + B buffers
static volatile uint8_t current_buffer = 0; // 0 = A, 1 = B
static volatile uint8_t spi_busy = 0;

// Task handle
TaskHandle_t spi_task_handle = NULL;

// SPI device commands (example for Winbond Flash)
#define FLASH_CMD_READ_ID    0x9F
#define FLASH_CMD_READ       0x03
#define FLASH_CMD_WRITE_EN   0x06
#define FLASH_CMD_PAGE_PROG  0x02
#define FLASH_CMD_SECTOR_ERASE 0x20

void spi_init(void)
{
    // GPIO Configuration for SPI1 (PA5: SCK, PA6: MISO, PA7: MOSI)
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    
    GPIO_InitTypeDef gpio = {0};
    
    // SPI1 SCK (PA5)
    gpio.Pin = GPIO_PIN_5;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &gpio);
    
    // SPI1 MISO (PA6)
    gpio.Pin = GPIO_PIN_6;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &gpio);
    
    // SPI1 MOSI (PA7)
    gpio.Pin = GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &gpio);
    
    // SPI1 Configuration
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64; // ~1.3 MHz at 84MHz
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    
    HAL_SPI_Init(&hspi1);
}

void spi_dma_init(void)
{
    // DMA is configured per transaction
    // Enable half and full transfer interrupts for double buffering
    __HAL_DMA_ENABLE_IT(hspi1.hdmarx, DMA_IT_HT | DMA_IT_TC);
    __HAL_DMA_ENABLE_IT(hspi1.hdmatx, DMA_IT_TC);
}

boot_error_t spi_dma_read(uint8_t* tx_data, uint8_t* rx_data, uint16_t len)
{
    if (spi_busy) {
        return BOOT_FLASH_ERROR;
    }
    
    spi_busy = 1;
    spi_set_cs(0); // CS low
    
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive_DMA(&hspi1, tx_data, rx_data, len);
    return (status == HAL_OK) ? BOOT_OK : BOOT_FLASH_ERROR;
}

boot_error_t spi_dma_write(uint8_t* data, uint16_t len)
{
    if (spi_busy) {
        return BOOT_FLASH_ERROR;
    }
    
    spi_busy = 1;
    spi_set_cs(0); // CS low
    
    HAL_StatusTypeDef status = HAL_SPI_Transmit_DMA(&hspi1, data, len);
    return (status == HAL_OK) ? BOOT_OK : BOOT_FLASH_ERROR;
}

uint8_t spi_is_busy(void)
{
    return spi_busy;
}

void spi_set_cs(uint8_t state)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// SPI task for continuous data acquisition (double buffering example)
void spi_task(void *arg)
{
    uint8_t tx_cmd[4] = {FLASH_CMD_READ_ID, 0x00, 0x00, 0x00};
    uint8_t rx_data[4];
    uint16_t* buffer_ptr;
    
    // Create task handle for external notifications
    spi_task_handle = xTaskGetCurrentTaskHandle();
    
    // Start continuous DMA reception (simulating ADC data)
    HAL_SPI_Receive_DMA(&hspi1, (uint8_t*)dma_buffer, BUF_SIZE * 2);
    
    for (;;)
    {
        // Wait for notification from ISR (half/full transfer)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        // Process the completed buffer
        if (current_buffer == 0) {
            // Buffer A completed
            buffer_ptr = &dma_buffer[0];
        } else {
            // Buffer B completed
            buffer_ptr = &dma_buffer[BUF_SIZE];
        }
        
        // Send buffer pointer to processing task
        xQueueSend(data_queue, &buffer_ptr, portMAX_DELAY);
        
        // Toggle current buffer
        current_buffer = !current_buffer;
    }
}

// HAL SPI Callbacks
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1) {
        spi_set_cs(1); // CS high
        spi_busy = 0;
        
        // Notify task of completion
        if (spi_task_handle != NULL) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            vTaskNotifyGiveFromISR(spi_task_handle, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1) {
        spi_set_cs(1); // CS high
        spi_busy = 0;
        
        // Notify task of completion
        if (spi_task_handle != NULL) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            vTaskNotifyGiveFromISR(spi_task_handle, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1) {
        // Full transfer complete - process buffer B
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        // Send buffer pointer to queue
        uint16_t* buffer_ptr = &dma_buffer[BUF_SIZE];
        xQueueSendFromISR(data_queue, &buffer_ptr, &xHigherPriorityTaskWoken);
        
        current_buffer = 1; // Next buffer to process will be A
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// Half transfer callback (for double buffering)
void HAL_SPI_RxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1) {
        // Half transfer complete - process buffer A
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        // Send buffer pointer to queue
        uint16_t* buffer_ptr = &dma_buffer[0];
        xQueueSendFromISR(data_queue, &buffer_ptr, &xHigherPriorityTaskWoken);
        
        current_buffer = 0; // Next buffer to process will be B
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// DMA IRQ Handlers
void DMA2_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(hspi1.hdmarx);
}

void DMA2_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(hspi1.hdmatx);
}

void DMA2_Stream2_IRQHandler(void)
{
    HAL_DMA_IRQHandler(hspi1.hdmarx);
}
