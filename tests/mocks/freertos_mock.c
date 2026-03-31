#include "freertos_mock.h"
#include "../mocks/hal_mock.h"
#include <stdlib.h>
#include <string.h>

// Mock globals
QueueHandle_t mock_sensor_queue = NULL;
SemaphoreHandle_t mock_flash_mutex = NULL;
SemaphoreHandle_t mock_uart_mutex = NULL;

// Mock queue implementation
typedef struct {
    void* buffer;
    uint32_t item_size;
    uint32_t capacity;
    uint32_t count;
    uint32_t head;
    uint32_t tail;
} mock_queue_t;

// Mock semaphore implementation
typedef struct {
    uint8_t is_taken;
} mock_semaphore_t;

QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize)
{
    mock_queue_t* queue = malloc(sizeof(mock_queue_t));
    if (!queue) return NULL;
    
    queue->buffer = malloc(uxQueueLength * uxItemSize);
    if (!queue->buffer) {
        free(queue);
        return NULL;
    }
    
    queue->item_size = uxItemSize;
    queue->capacity = uxQueueLength;
    queue->count = 0;
    queue->head = 0;
    queue->tail = 0;
    
    return queue;
}

BaseType_t xQueueSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait)
{
    mock_queue_t* queue = (mock_queue_t*)xQueue;
    if (!queue || !pvItemToQueue || queue->count >= queue->capacity) {
        return pdFALSE;
    }
    
    uint8_t* buffer = (uint8_t*)queue->buffer;
    memcpy(&buffer[queue->tail * queue->item_size], pvItemToQueue, queue->item_size);
    
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;
    
    return pdTRUE;
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait)
{
    mock_queue_t* queue = (mock_queue_t*)xQueue;
    if (!queue || !pvBuffer || queue->count == 0) {
        return pdFALSE;
    }
    
    uint8_t* buffer = (uint8_t*)queue->buffer;
    memcpy(pvBuffer, &buffer[queue->head * queue->item_size], queue->item_size);
    
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;
    
    return pdTRUE;
}

BaseType_t xQueueSendFromISR(QueueHandle_t xQueue, const void *pvItemToQueue, BaseType_t *pxHigherPriorityTaskWoken)
{
    // Simplified ISR version - same as normal version for testing
    return xQueueSend(xQueue, pvItemToQueue, 0);
}

SemaphoreHandle_t xSemaphoreCreateMutex(void)
{
    mock_semaphore_t* sem = malloc(sizeof(mock_semaphore_t));
    if (!sem) return NULL;
    
    sem->is_taken = 0;
    return sem;
}

BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait)
{
    mock_semaphore_t* sem = (mock_semaphore_t*)xSemaphore;
    if (!sem) return pdFALSE;
    
    if (sem->is_taken) {
        return pdFALSE; // Simplified - no blocking in tests
    }
    
    sem->is_taken = 1;
    return pdTRUE;
}

BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore)
{
    mock_semaphore_t* sem = (mock_semaphore_t*)xSemaphore;
    if (!sem) return pdFALSE;
    
    sem->is_taken = 0;
    return pdTRUE;
}

BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t xSemaphore, BaseType_t *pxHigherPriorityTaskWoken)
{
    return xSemaphoreGive(xSemaphore);
}

BaseType_t xTaskCreate(void (*pvTaskCode)(void *), const char *pcName, uint16_t usStackDepth, void *pvParameters, UBaseType_t uxPriority, TaskHandle_t *pxCreatedTask)
{
    // For testing, we don't actually create tasks
    if (pxCreatedTask) {
        *pxCreatedTask = (TaskHandle_t)1; // Mock handle
    }
    return pdTRUE;
}

void vTaskDelay(const TickType_t xTicksToDelay)
{
    HAL_Delay(xTicksToDelay);
}

TickType_t xTaskGetTickCount(void)
{
    return HAL_GetTick();
}

BaseType_t xTaskNotify(TaskHandle_t xTaskToNotify, uint32_t ulValue, eNotifyAction eAction, uint32_t *pulPreviousNotifyValue)
{
    return pdTRUE; // Mock success
}

BaseType_t xTaskNotifyFromISR(TaskHandle_t xTaskToNotify, uint32_t ulValue, BaseType_t *pxHigherPriorityTaskWoken)
{
    return pdTRUE; // Mock success
}

uint32_t ulTaskNotifyTake(BaseType_t xClearCountOnExit, TickType_t xTicksToWait)
{
    return 1; // Mock notification received
}

void vTaskStartScheduler(void)
{
    // Mock - don't actually start scheduler in tests
}

void vTaskSuspendAll(void)
{
    // Mock
}

BaseType_t xTaskResumeAll(void)
{
    return pdTRUE; // Mock success
}
