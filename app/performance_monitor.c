/**
 * @file performance_monitor.c
 * @brief Performance monitoring and metrics collection implementation
 * 
 * This file implements the performance monitoring system for collecting
 * real-time metrics and system performance data.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#include "performance_monitor.h"
#include "FreeRTOS.h"
#include "task.h"

// Global performance metrics instance
static performance_metrics_t perf_metrics = {0};

// Performance monitoring constants
#define PERF_UPDATE_INTERVAL_MS    1000    // Update every second
#define PERF_DMA_LATENCY_SAMPLES   10       // Number of samples for average
#define PERF_FLASH_WRITE_SAMPLES  10       // Number of samples for average

// Running averages for performance data
static uint32_t dma_latency_samples[PERF_DMA_LATENCY_SAMPLES] = {0};
static uint32_t dma_latency_index = 0;
static uint32_t flash_write_samples[PERF_FLASH_WRITE_SAMPLES] = {0};
static uint32_t flash_write_index = 0;

void performance_init(void)
{
    // Clear all metrics
    memset(&perf_metrics, 0, sizeof(performance_metrics_t));
    
    // Initialize heap monitoring
    perf_metrics.heap_size_bytes = configTOTAL_HEAP_SIZE;
}

void performance_record_uart_tx(uint32_t bytes_sent, uint32_t duration_us)
{
    perf_metrics.uart_bytes_sent += bytes_sent;
    perf_metrics.uart_last_tx_time = xTaskGetTickCount();
    
    // Calculate throughput (bytes per second)
    if (duration_us > 0) {
        uint32_t throughput_bps = (bytes_sent * 1000000) / duration_us;
        // Store for reporting (could add to metrics structure)
    }
}

void performance_record_uart_rx(uint32_t bytes_received, uint32_t duration_us)
{
    perf_metrics.uart_bytes_received += bytes_received;
    perf_metrics.uart_last_rx_time = xTaskGetTickCount();
}

void performance_record_dma_transfer(uint32_t bytes_transferred, uint32_t latency_us)
{
    perf_metrics.dma_transfers_completed++;
    
    // Update running average for latency
    dma_latency_samples[dma_latency_index] = latency_us;
    dma_latency_index = (dma_latency_index + 1) % PERF_DMA_LATENCY_SAMPLES;
    
    // Calculate average
    uint32_t sum = 0;
    for (int i = 0; i < PERF_DMA_LATENCY_SAMPLES; i++) {
        sum += dma_latency_samples[i];
    }
    perf_metrics.dma_average_latency_us = sum / PERF_DMA_LATENCY_SAMPLES;
}

void performance_record_flash_write(uint32_t bytes_written, uint32_t duration_us)
{
    perf_metrics.flash_writes_completed++;
    perf_metrics.flash_endurance_cycles++;
    
    // Update running average for write time
    flash_write_samples[flash_write_index] = duration_us;
    flash_write_index = (flash_write_index + 1) % PERF_FLASH_WRITE_SAMPLES;
    
    // Calculate average
    uint32_t sum = 0;
    for (int i = 0; i < PERF_FLASH_WRITE_SAMPLES; i++) {
        sum += flash_write_samples[i];
    }
    perf_metrics.flash_average_write_time_us = sum / PERF_FLASH_WRITE_SAMPLES;
}

void performance_record_ota_transfer(uint32_t bytes_transferred, uint32_t duration_ms)
{
    perf_metrics.ota_bytes_transferred += bytes_transferred;
    perf_metrics.ota_total_time_ms += duration_ms;
    
    // Calculate CRC verify time (simulated)
    perf_metrics.ota_crc_verify_time_us = bytes_transferred * 50 / 1000;  // 50us per KB
}

const performance_metrics_t* performance_get_metrics(void)
{
    // Update dynamic metrics before returning
    perf_metrics.heap_free_bytes = xPortGetFreeHeapSize();
    perf_metrics.stack_high_watermark = uxTaskGetStackHighWaterMark(NULL);
    
    // Calculate CPU utilization (simplified)
    static uint32_t last_idle_count = 0;
    uint32_t current_idle_count = xTaskGetIdleRunTime();
    uint32_t total_ticks = xTaskGetTickCount();
    
    if (total_ticks > 0) {
        uint32_t idle_delta = current_idle_count - last_idle_count;
        perf_metrics.cpu_utilization_percent = 100 - ((idle_delta * 100) / total_ticks);
        last_idle_count = current_idle_count;
    }
    
    return &perf_metrics;
}

void performance_reset_metrics(void)
{
    memset(&perf_metrics, 0, sizeof(performance_metrics_t));
    perf_metrics.heap_size_bytes = configTOTAL_HEAP_SIZE;
}

void performance_task(void *arg)
{
    TickType_t last_wake_time = xTaskGetTickCount();
    
    (void)arg;  // Suppress unused parameter warning
    
    while (1) {
        // Update dynamic metrics
        const performance_metrics_t* metrics = performance_get_metrics();
        
        // Log performance data (could be sent via UART or stored)
        // This is where you would add performance logging
        
        // Wait for next update
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(PERF_UPDATE_INTERVAL_MS));
    }
}
