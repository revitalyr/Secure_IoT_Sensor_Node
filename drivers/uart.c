#include "uart.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "stm32f4xx_hal.h"

extern QueueHandle_t uart_rx_queue;

// UART handle
UART_HandleTypeDef huart2;

// DMA buffers
#define RX_BUFFER_SIZE  512
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint16_t old_pos = 0;
static volatile uint16_t new_pos = 0;

// TX buffer for DMA
static uint8_t tx_buffer[256];
static volatile uint8_t tx_busy = 0;

void uart_init(void)
{
    // GPIO Configuration for USART2 (PA2: TX, PA3: RX)
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
    
    GPIO_InitTypeDef gpio = {0};
    
    // USART2 TX (PA2)
    gpio.Pin = GPIO_PIN_2;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &gpio);
    
    // USART2 RX (PA3)
    gpio.Pin = GPIO_PIN_3;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &gpio);
    
    // USART2 Configuration
    huart2.Instance = USART2;
    huart2.Init.BaudRate = UART_BAUDRATE;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
    
    // Initialize DMA
    uart_dma_init();
}

void uart_dma_init(void)
{
    // Start DMA reception in circular mode
    HAL_UART_Receive_DMA(&huart2, rx_buffer, RX_BUFFER_SIZE);
    
    // Enable UART idle line interrupt
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
    
    // Configure NVIC for USART2
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
}

void uart_write(uint8_t* data, uint16_t len)
{
    if (len == 0 || data == NULL) {
        return;
    }
    
    // Wait for previous transmission to complete
    while (tx_busy) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    // Copy data to TX buffer
    if (len <= sizeof(tx_buffer)) {
        memcpy(tx_buffer, data, len);
        tx_busy = 1;
        HAL_UART_Transmit_DMA(&huart2, tx_buffer, len);
    } else {
        // Data too large, use blocking transmission
        HAL_UART_Transmit(&huart2, data, len, HAL_MAX_DELAY);
    }
}

void uart_write_string(const char* str)
{
    if (str != NULL) {
        uart_write((uint8_t*)str, strlen(str));
    }
}

uint16_t uart_read(uint8_t* buffer, uint16_t max_len)
{
    uint16_t available = 0;
    uart_dma_get_rx(buffer, &available);
    return (available > max_len) ? max_len : available;
}

uint16_t uart_read_line(char* buffer, uint16_t max_len)
{
    uint16_t len = 0;
    uint8_t byte;
    
    while (len < max_len - 1) {
        if (xQueueReceive(uart_rx_queue, &byte, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (byte == '\n' || byte == '\r') {
                break;
            }
            buffer[len++] = byte;
        } else {
            break;
        }
    }
    
    buffer[len] = '\0';
    return len;
}

uint16_t uart_dma_get_rx(uint8_t* out, uint16_t* len)
{
    uint16_t received = 0;
    
    // Get current DMA position
    new_pos = RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx);
    
    if (new_pos != old_pos) {
        if (new_pos > old_pos) {
            // Simple case: no wrap around
            received = new_pos - old_pos;
            if (out != NULL) {
                memcpy(out, &rx_buffer[old_pos], received);
            }
        } else {
            // Wrap around case
            uint16_t first_part = RX_BUFFER_SIZE - old_pos;
            received = first_part + new_pos;
            
            if (out != NULL) {
                memcpy(out, &rx_buffer[old_pos], first_part);
                memcpy(out + first_part, rx_buffer, new_pos);
            }
        }
        
        old_pos = new_pos;
    }
    
    if (len != NULL) {
        *len = received;
    }
    
    return received;
}

void uart_process_rx_data(uint8_t* data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        // Send each byte to the queue for processing by tasks
        xQueueSendFromISR(uart_rx_queue, &data[i], NULL);
    }
}

// USART2 IRQ Handler
void USART2_IRQHandler(void)
{
    // Check for idle line detection
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE)) {
        __HAL_UART_CLEAR_IDLEFLAG(&huart2);
        
        // Process received data
        uint8_t temp_buffer[64];
        uint16_t received_len;
        
        uint16_t total_received = uart_dma_get_rx(temp_buffer, &received_len);
        if (total_received > 0) {
            uart_process_rx_data(temp_buffer, received_len);
        }
    }
    
    // Call HAL UART IRQ handler
    HAL_UART_IRQHandler(&huart2);
}

// HAL UART Callbacks
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        // DMA reception complete (circular mode restarts automatically)
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        tx_busy = 0;
    }
}

// DMA IRQ Handlers (if needed)
void DMA1_Stream6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(huart2.hdmatx);
}

void DMA1_Stream5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(huart2.hdmarx);
}
