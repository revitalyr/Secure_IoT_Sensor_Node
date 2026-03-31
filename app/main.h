/**
 * @file main.h
 * @brief Main application header with system interfaces
 * 
 * This header defines the main application interfaces, system
 * initialization functions, and RTOS task management for the
 * Secure IoT Sensor Node.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef MAIN_H
#define MAIN_H

#include "system_config.h"
#include "memory_map.h"
#include <stdint.h>

/**
 * @brief Initialize system hardware and peripherals
 * 
 * Configures system clock, initializes hardware peripherals,
 * and prepares the system for RTOS startup.
 */
void system_init(void);

/**
 * @brief Configure system clock
 * 
 * Sets up the system clock configuration for optimal
 * performance and power consumption.
 */
void SystemClock_Config(void);

/**
 * @brief Initialize RTOS kernel and objects
 * 
 * Creates RTOS objects including queues, semaphores,
 * and prepares task infrastructure.
 */
void rtos_init(void);

/**
 * @brief Start all RTOS tasks
 * 
 * Creates and starts all application tasks with proper
 * priorities and stack sizes.
 */
void rtos_start_tasks(void);

/* Task Declarations */
/**
 * @brief Sensor data acquisition task
 * 
 * FreeRTOS task for sensor data collection and processing.
 * 
 * @param arg Task argument (unused)
 */
void sensor_task(void *arg);

/**
 * @brief Communication task
 * 
 * FreeRTOS task for UART communication and data transmission.
 * 
 * @param arg Task argument (unused)
 */
void comm_task(void *arg);

/**
 * @brief Storage task
 * 
 * FreeRTOS task for flash storage and data persistence.
 * 
 * @param arg Task argument (unused)
 */
void storage_task(void *arg);

/**
 * @brief OTA update task
 * 
 * FreeRTOS task for firmware updates and management.
 * 
 * @param arg Task argument (unused)
 */
void update_task(void *arg);

/* Queue Handles */
extern QueueHandle_t sensor_queue;    /**< Sensor data queue */
extern QueueHandle_t uart_rx_queue;   /**< UART receive queue */
extern QueueHandle_t data_queue;      /**< General data queue */

/* Mutex Handles */
extern SemaphoreHandle_t flash_mutex; /**< Flash access mutex */
extern SemaphoreHandle_t uart_mutex;  /**< UART access mutex */

/* Utility Functions */
/**
 * @brief Get current system tick count
 * 
 * Returns the current system tick count for timing
 * and synchronization purposes.
 * 
 * @return uint32_t Current system tick count
 */
uint32_t get_system_tick(void);

/**
 * @brief System error handler
 * 
 * Handles system-level errors with logging and recovery
 * procedures.
 * 
 * @param error Error description string
 */
void system_error_handler(const char* error);

/**
 * @brief Stack overflow hook
 * 
 * FreeRTOS callback for stack overflow detection.
 * 
 * @param xTask Task handle that overflowed
 * @param pcTaskName Task name
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);

/**
 * @brief Memory allocation failure hook
 * 
 * FreeRTOS callback for malloc failure detection.
 */
void vApplicationMallocFailedHook(void);

#endif // MAIN_H
