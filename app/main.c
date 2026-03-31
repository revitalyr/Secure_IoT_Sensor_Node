#include "main.h"
#include "uart.h"
#include "i2c.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stm32f4xx_hal.h"

// Global handles
QueueHandle_t sensor_queue;
QueueHandle_t uart_rx_queue;
QueueHandle_t data_queue;
SemaphoreHandle_t flash_mutex;
SemaphoreHandle_t uart_mutex;

// System initialization
void system_init(void)
{
    HAL_Init();
    SystemClock_Config();
    
    // Initialize peripherals
    gpio_init();
    uart_init();
    i2c_init();
    
    // Initialize status LED
    led_set_status(0);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    // Configure the main internal regulator output voltage
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    
    // Initialize the RCC Oscillators
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    
    // Initialize the CPU, AHB and APB buses clocks
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

// RTOS initialization
void rtos_init(void)
{
    // Create queues
    sensor_queue = xQueueCreate(SENSOR_QUEUE_SIZE, sizeof(float));
    uart_rx_queue = xQueueCreate(UART_RX_QUEUE_SIZE, sizeof(uint8_t));
    data_queue = xQueueCreate(16, sizeof(uint16_t*));
    
    // Create mutexes
    flash_mutex = xSemaphoreCreateMutex();
    uart_mutex = xSemaphoreCreateMutex();
    
    if (!sensor_queue || !uart_rx_queue || !data_queue || 
        !flash_mutex || !uart_mutex) {
        system_error_handler("RTOS init failed");
    }
}

void rtos_start_tasks(void)
{
    BaseType_t result;
    
    result = xTaskCreate(sensor_task, "Sensor", TASK_STACK_SIZE, NULL, 
                        SENSOR_TASK_PRIORITY, NULL);
    if (result != pdPASS) {
        system_error_handler("Sensor task creation failed");
    }
    
    result = xTaskCreate(comm_task, "Comm", TASK_STACK_SIZE, NULL, 
                        COMM_TASK_PRIORITY, NULL);
    if (result != pdPASS) {
        system_error_handler("Comm task creation failed");
    }
    
    result = xTaskCreate(storage_task, "Storage", TASK_STACK_SIZE, NULL, 
                        STORAGE_TASK_PRIORITY, NULL);
    if (result != pdPASS) {
        system_error_handler("Storage task creation failed");
    }
    
    result = xTaskCreate(update_task, "Update", TASK_STACK_SIZE, NULL, 
                        UPDATE_TASK_PRIORITY, NULL);
    if (result != pdPASS) {
        system_error_handler("Update task creation failed");
    }
}

// Main function
int main(void)
{
    system_init();
    rtos_init();
    rtos_start_tasks();
    
    // Start scheduler
    vTaskStartScheduler();
    
    // Should never reach here
    while (1) {
        led_blink(3, 500);
        HAL_Delay(2000);
    }
}

// Utility functions
uint32_t get_system_tick(void)
{
    return xTaskGetTickCount();
}

void system_error_handler(const char* error)
{
    // Disable interrupts
    taskDISABLE_INTERRUPTS();
    
    // Blink error pattern
    while (1) {
        led_blink(5, 100);
        HAL_Delay(1000);
    }
}

// FreeRTOS hooks
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    system_error_handler("Stack overflow");
}

void vApplicationMallocFailedHook(void)
{
    system_error_handler("Malloc failed");
}

void vApplicationIdleHook(void)
{
    // Put MCU to low power mode when idle
    __WFI();
}
