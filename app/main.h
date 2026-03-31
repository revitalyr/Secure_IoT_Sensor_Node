#pragma once

#include "system_config.h"
#include "memory_map.h"
#include <stdint.h>

// System initialization
void system_init(void);
void SystemClock_Config(void);

// RTOS initialization
void rtos_init(void);
void rtos_start_tasks(void);

// Task declarations
void sensor_task(void *arg);
void comm_task(void *arg);
void storage_task(void *arg);
void update_task(void *arg);

// Queue handles
extern QueueHandle_t sensor_queue;
extern QueueHandle_t uart_rx_queue;
extern QueueHandle_t data_queue;

// Mutex handles
extern SemaphoreHandle_t flash_mutex;
extern SemaphoreHandle_t uart_mutex;

// Utility functions
uint32_t get_system_tick(void);
void system_error_handler(const char* error);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
void vApplicationMallocFailedHook(void);
