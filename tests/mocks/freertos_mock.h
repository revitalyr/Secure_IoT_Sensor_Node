/**
 * @file freertos_mock.h
 * @brief FreeRTOS mock interface for testing
 * 
 * This header defines FreeRTOS mock interfaces, type definitions,
 * and simulated RTOS functionality for unit testing without
 * actual FreeRTOS dependencies.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef FREERTOS_MOCK_H
#define FREERTOS_MOCK_H

#include "../test_framework.h"
#include <stdint.h>

/* FreeRTOS Type Definitions for Testing */
typedef void* QueueHandle_t;          /**< Mock queue handle type */
typedef void* SemaphoreHandle_t;     /**< Mock semaphore handle type */
typedef void* TaskHandle_t;          /**< Mock task handle type */
typedef uint32_t TickType_t;         /**< Tick count type */
typedef long BaseType_t;             /**< Base return type */
typedef unsigned long UBaseType_t;   /**< Unsigned base type */

/* FreeRTOS Enums */
typedef enum {
    eNoAction = 0,
    eSetBits,
    eIncrement,
    eSetValueWithOverwrite,
    eSetValueWithoutOverwrite
} eNotifyAction;

// Constants
#define pdTRUE 1
#define pdFALSE 0
#define portMAX_DELAY ((TickType_t)0xFFFFFFFFUL)
#define pdMS_TO_TICKS(x) ((TickType_t)((x) * configTICK_RATE_HZ / 1000UL))

// Mock functions
QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize);
BaseType_t xQueueSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait);
BaseType_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait);
BaseType_t xQueueSendFromISR(QueueHandle_t xQueue, const void *pvItemToQueue, BaseType_t *pxHigherPriorityTaskWoken);

SemaphoreHandle_t xSemaphoreCreateMutex(void);
BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait);
BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore);
BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t xSemaphore, BaseType_t *pxHigherPriorityTaskWoken);

BaseType_t xTaskCreate(void (*pvTaskCode)(void *), const char *pcName, uint16_t usStackDepth, void *pvParameters, UBaseType_t uxPriority, TaskHandle_t *pxCreatedTask);
void vTaskDelay(const TickType_t xTicksToDelay);
TickType_t xTaskGetTickCount(void);
BaseType_t xTaskNotify(TaskHandle_t xTaskToNotify, uint32_t ulValue, uint32_t eAction, uint32_t *pulPreviousNotifyValue);
BaseType_t xTaskNotifyFromISR(TaskHandle_t xTaskToNotify, uint32_t ulValue, BaseType_t *pxHigherPriorityTaskWoken);
uint32_t ulTaskNotifyTake(BaseType_t xClearCountOnExit, TickType_t xTicksToWait);

void vTaskStartScheduler(void);
void vTaskSuspendAll(void);
BaseType_t xTaskResumeAll(void);

// Mock globals
extern QueueHandle_t mock_sensor_queue;
extern SemaphoreHandle_t mock_flash_mutex;
extern SemaphoreHandle_t mock_uart_mutex;

/* Global Variables (for compatibility with firmware) */
extern QueueHandle_t sensor_queue;    /**< Sensor data queue */
extern QueueHandle_t uart_rx_queue;   /**< UART receive queue */
extern QueueHandle_t data_queue;      /**< General data queue */
extern SemaphoreHandle_t flash_mutex; /**< Flash access mutex */
extern SemaphoreHandle_t uart_mutex;  /**< UART access mutex */

#endif // FREERTOS_MOCK_H
