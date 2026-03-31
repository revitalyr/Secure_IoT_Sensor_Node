#include "stm32f4xx_hal.h"

// Simple stub implementations

HAL_StatusTypeDef HAL_Init(void) {
    return HAL_OK;
}

void HAL_Delay(uint32_t delay) {
    // Simple delay stub
    volatile uint32_t i;
    for (i = 0; i < delay * 1000; i++);
}

uint32_t HAL_GetTick(void) {
    static uint32_t tick = 0;
    return ++tick;
}

void HAL_GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state) {
    // GPIO write stub
}

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin) {
    return GPIO_PIN_SET;
}

void HAL_GPIO_Init(GPIO_InitTypeDef* init) {
    // GPIO init stub
}

void __HAL_RCC_GPIOC_CLK_ENABLE(void) {}
void __HAL_RCC_USART2_CLK_ENABLE(void) {}
void __HAL_RCC_I2C1_CLK_ENABLE(void) {}
void __HAL_RCC_SPI1_CLK_ENABLE(void) {}
void __HAL_RCC_DMA1_CLK_ENABLE(void) {}
void __HAL_RCC_GPIOA_CLK_ENABLE(void) {}

HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef* huart) {
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size, uint32_t timeout) {
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size, uint32_t timeout) {
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size) {
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size) {
    return HAL_OK;
}

void HAL_UART_IRQHandler(UART_HandleTypeDef* huart) {}

HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef* hi2c) {
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef* hi2c, uint16_t dev_addr, uint8_t* data, uint16_t size, uint32_t timeout) {
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef* hi2c, uint16_t dev_addr, uint8_t* data, uint16_t size, uint32_t timeout) {
    return HAL_OK;
}

HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef* hspi) {
    return HAL_OK;
}

HAL_StatusTypeDef HAL_SPI_TransmitReceive_DMA(SPI_HandleTypeDef* hspi, uint8_t* tx_data, uint8_t* rx_data, uint16_t size) {
    return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASH_Unlock(void) {
    return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASH_Lock(void) {
    return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASH_Program(uint32_t type_program, uint32_t address, uint64_t data) {
    return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef* erase, uint32_t* sector_error) {
    return HAL_OK;
}

void SystemClock_Config(void) {
    // System clock configuration stub
}

void Error_Handler(void) {
    while (1) {}
}
