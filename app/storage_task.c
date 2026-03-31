#include "storage_task.h"
#include "flash_if.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>

extern QueueHandle_t sensor_queue;
extern SemaphoreHandle_t flash_mutex;

// Storage configuration
#define STORAGE_BASE_ADDR     0x08080000  // Use second half of Flash
#define STORAGE_SECTOR        FLASH_SECTOR_7
#define STORAGE_MAX_ENTRIES   (STORAGE_SIZE)

// In-memory cache for wear leveling
static storage_entry_t m_storage_cache[STORAGE_SIZE];
static uint32_t m_write_index = 0;
static uint32_t m_entry_count = 0;
static uint32_t m_write_cycles = 0;
static uint8_t m_storage_initialized = 0;

void storage_init(void)
{
    if (m_storage_initialized) {
        return;
    }
    
    // Initialize cache with zeros
    memset(m_storage_cache, 0, sizeof(m_storage_cache));
    
    // Load existing data from flash (if any)
    if (xSemaphoreTake(flash_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        storage_entry_t* flash_data = (storage_entry_t*)STORAGE_BASE_ADDR;
        
        // Find the last valid entry
        for (uint32_t i = 0; i < STORAGE_MAX_ENTRIES; i++) {
            uint32_t crc = compute_crc16((uint8_t*)&flash_data[i].data, 
                                       sizeof(sensor_data_t));
            
            if (flash_data[i].crc == crc) {
                m_storage_cache[i] = flash_data[i];
                m_entry_count = i + 1;
                m_write_index = (i + 1) % STORAGE_MAX_ENTRIES;
            } else {
                break; // Stop at first invalid entry
            }
        }
        
        xSemaphoreGive(flash_mutex);
    }
    
    m_storage_initialized = 1;
}

boot_error_t storage_write_entry(const sensor_data_t* data)
{
    if (!m_storage_initialized) {
        storage_init();
    }
    
    // Create storage entry
    storage_entry_t entry;
    entry.data = *data;
    entry.crc = compute_crc16((uint8_t*)&entry.data, sizeof(sensor_data_t));
    
    // Update cache
    m_storage_cache[m_write_index] = entry;
    m_write_index = (m_write_index + 1) % STORAGE_MAX_ENTRIES;
    
    if (m_entry_count < STORAGE_MAX_ENTRIES) {
        m_entry_count++;
    }
    
    // Write to flash periodically (every 10 entries or when buffer is full)
    if ((m_write_cycles % 10 == 0) || (m_write_index == 0)) {
        if (xSemaphoreTake(flash_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            // Erase sector if starting fresh
            if (m_write_index == 0) {
                flash_erase_sector(STORAGE_SECTOR);
            }
            
            // Write entry to flash
            uint32_t addr = STORAGE_BASE_ADDR + ((m_write_index - 1) % STORAGE_MAX_ENTRIES) * 
                           sizeof(storage_entry_t);
            
            uint32_t* src = (uint32_t*)&entry;
            for (uint32_t i = 0; i < sizeof(storage_entry_t) / 4; i++) {
                flash_write_word(addr + i * 4, src[i]);
            }
            
            xSemaphoreGive(flash_mutex);
        }
    }
    
    m_write_cycles++;
    return BOOT_OK;
}

boot_error_t storage_read_entry(uint32_t index, sensor_data_t* data)
{
    if (!m_storage_initialized) {
        storage_init();
    }
    
    if (index >= STORAGE_MAX_ENTRIES) {
        return BOOT_FLASH_ERROR;
    }
    
    *data = m_storage_cache[index].data;
    return BOOT_OK;
}

boot_error_t storage_get_latest(sensor_data_t* data)
{
    if (!m_storage_initialized) {
        storage_init();
    }
    
    if (m_entry_count == 0) {
        return BOOT_FLASH_ERROR;
    }
    
    uint32_t latest_index = (m_write_index == 0) ? 
                           STORAGE_MAX_ENTRIES - 1 : m_write_index - 1;
    
    *data = m_storage_cache[latest_index].data;
    return BOOT_OK;
}

uint32_t storage_get_count(void)
{
    return m_entry_count;
}

void storage_task(void *arg)
{
    float temperature;
    sensor_data_t sensor_data;
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(1000); // Store every 1 second
    
    // Initialize storage
    storage_init();
    
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        // Wait for sensor data
        if (xQueueReceive(sensor_queue, &temperature, pdMS_TO_TICKS(100)) == pdTRUE) {
            // Create sensor data entry
            sensor_data.temperature = temperature;
            sensor_data.timestamp = xTaskGetTickCount();
            sensor_data.status = 0; // Normal status
            
            // Store the data
            storage_write_entry(&sensor_data);
        }
        
        // Periodic maintenance
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
