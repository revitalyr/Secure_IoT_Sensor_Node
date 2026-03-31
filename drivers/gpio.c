#include "gpio.h"
#include "stm32f4xx_hal.h"

void gpio_init(void)
{
    // Enable GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitTypeDef gpio = {0};
    
    // Status LED (PC13 - typically onboard LED on STM32F4 discovery)
    gpio.Pin = LED_STATUS_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_STATUS_PORT, &gpio);
    
    // User button (PA0)
    gpio.Pin = BUTTON_USER_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLDOWN;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUTTON_USER_PORT, &gpio);
    
    // Additional GPIO for SPI chip select (PA4)
    gpio.Pin = GPIO_PIN_4;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // CS high (inactive)
    
    // GPIO for external interrupt (PA1 - for SPI device ready signal)
    gpio.Pin = GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_IT_RISING;
    gpio.Pull = GPIO_PULLDOWN;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);
    
    // Configure NVIC for external interrupt
    HAL_NVIC_SetPriority(EXTI1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

void led_set_status(uint8_t on)
{
    HAL_GPIO_WritePin(LED_STATUS_PORT, LED_STATUS_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void led_blink(uint8_t count, uint32_t delay_ms)
{
    for (uint8_t i = 0; i < count; i++) {
        led_set_status(1);
        HAL_Delay(delay_ms);
        led_set_status(0);
        HAL_Delay(delay_ms);
    }
}

uint8_t button_is_pressed(void)
{
    return HAL_GPIO_ReadPin(BUTTON_USER_PORT, BUTTON_USER_PIN) == GPIO_PIN_SET;
}

void gpio_set_output(uint16_t pin, GPIO_TypeDef* port, uint8_t state)
{
    HAL_GPIO_WritePin(port, pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

uint8_t gpio_get_input(uint16_t pin, GPIO_TypeDef* port)
{
    return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET;
}

// External interrupt handler
void EXTI1_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1)) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
        
        // Handle SPI device ready signal
        // This could set a flag or notify a task
        extern TaskHandle_t spi_task_handle;
        if (spi_task_handle != NULL) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            vTaskNotifyGiveFromISR(spi_task_handle, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
