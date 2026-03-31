/**
 * @file performance_monitor.h
 * @brief Performance monitoring and metrics collection
 * 
 * This header defines performance monitoring system for collecting
 * real-time metrics and system performance data.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef PERFORMANCE_MONITOR_H
#define PERFORMANCE_MONITOR_H

#include "system_config.h"
#include <stdint.h>

/**
 * @brief Performance metrics structure
 * 
 * Contains collected performance data for system monitoring.
 */
typedef struct {
    // UART Performance
    uint32_t uart_bytes_sent;
    uint32_t uart_bytes_received;
    uint32_t uart_errors;
    uint32_t uart_last_tx_time;
    uint32_t uart_last_rx_time;
    
    // DMA Performance
    uint32_t dma_transfers_completed;
    uint32_t dma_transfer_errors;
    uint32_t dma_average_latency_us;
    
    // Flash Performance
    uint32_t flash_writes_completed;
    uint32_t flash_write_errors;
    uint32_t flash_average_write_time_us;
    uint32_t flash_endurance_cycles;
    
    // OTA Performance
    uint32_t ota_total_time_ms;
    uint32_t ota_bytes_transferred;
    uint32_t ota_crc_verify_time_us;
    
    // Memory Usage
    uint32_t heap_size_bytes;
    uint32_t heap_free_bytes;
    uint32_t stack_high_watermark;
    
    // System Performance
    uint32_t cpu_utilization_percent;
    uint32_t task_switches_per_second;
    uint32_t interrupt_rate_per_second;
} performance_metrics_t;

/**
 * @brief Initialize performance monitor
 * 
 * Sets up performance monitoring system and clears metrics.
 */
void performance_init(void);

/**
 * @brief Record UART transmission
 * 
 * @param bytes_sent Number of bytes transmitted
 * @param duration_us Transmission duration in microseconds
 */
void performance_record_uart_tx(uint32_t bytes_sent, uint32_t duration_us);

/**
 * @brief Record UART reception
 * 
 * @param bytes_received Number of bytes received
 * @param duration_us Reception duration in microseconds
 */
void performance_record_uart_rx(uint32_t bytes_received, uint32_t duration_us);

/**
 * @brief Record DMA transfer
 * 
 * @param bytes_transferred Number of bytes transferred
 * @param latency_us Transfer latency in microseconds
 */
void performance_record_dma_transfer(uint32_t bytes_transferred, uint32_t latency_us);

/**
 * @brief Record flash write operation
 * 
 * @param bytes_written Number of bytes written
 * @param duration_us Write duration in microseconds
 */
void performance_record_flash_write(uint32_t bytes_written, uint32_t duration_us);

/**
 * @brief Record OTA operation
 * 
 * @param bytes_transferred Number of bytes transferred
 * @param duration_ms Transfer duration in milliseconds
 */
void performance_record_ota_transfer(uint32_t bytes_transferred, uint32_t duration_ms);

/**
 * @brief Get current performance metrics
 * 
 * @return Pointer to performance metrics structure
 */
const performance_metrics_t* performance_get_metrics(void);

/**
 * @brief Reset performance metrics
 * 
 * Clears all accumulated performance data.
 */
void performance_reset_metrics(void);

/**
 * @brief Performance monitoring task
 * 
 * FreeRTOS task that periodically updates performance metrics.
 * 
 * @param arg Task argument (unused)
 */
void performance_task(void *arg);

#endif // PERFORMANCE_MONITOR_H
