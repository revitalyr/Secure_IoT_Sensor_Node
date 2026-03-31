#include "sensor_task.h"
#include "i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f4xx_hal.h"

extern QueueHandle_t sensor_queue;

// Mock sensor implementation - simulates temperature sensor
static float m_temperature = 36.5f;
static float m_increment = 0.1f;
static uint32_t m_read_count = 0;

void sensor_init(void)
{
    // Initialize I2C for sensor communication
    // In real implementation, this would configure the actual sensor
    m_temperature = 36.5f;
    m_read_count = 0;
}

float read_sensor_temperature(void)
{
    // Mock temperature sensor with realistic variation
    m_temperature += m_increment;
    
    // Add some noise
    float noise = ((float)(m_read_count % 10) - 5.0f) * 0.01f;
    m_temperature += noise;
    
    // Reverse direction at limits
    if (m_temperature > 38.0f) {
        m_temperature = 38.0f;
        m_increment = -0.1f;
    } else if (m_temperature < 35.0f) {
        m_temperature = 35.0f;
        m_increment = 0.1f;
    }
    
    m_read_count++;
    return m_temperature;
}

void sensor_task(void *arg)
{
    float temperature;
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 100ms period
    
    // Initialize sensor
    sensor_init();
    
    // Initialize the xLastWakeTime variable with the current time
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;)
    {
        // Read temperature from sensor
        temperature = read_sensor_temperature();
        
        // Send to queue for other tasks
        if (xQueueSend(sensor_queue, &temperature, 0) != pdTRUE) {
            // Queue full - handle error (could increment error counter)
            #ifdef DEBUG_ENABLED
            // printf("Sensor queue full\n");
            #endif
        }
        
        // Wait for the next cycle
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
