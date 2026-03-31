/**
 * @file sensor_task.c
 * @brief Sensor data acquisition and processing task
 * 
 * This module implements realistic sensor data simulation for the
 * Secure IoT Sensor Node. All data generation follows physical
 * constraints and produces production-quality test data.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#include "sensor_task.h"
#include "i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f4xx_hal.h"
#include "web_interface.h"
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

/* ── Configuration Constants ────────────────────────────────────── */

#define SENSOR_UPDATE_PERIOD_MS    2000U    /**< Task period in milliseconds */
#define SECONDS_PER_DAY            86400U   /**< Seconds in 24 hours */
#define SENSOR_QUEUE_TIMEOUT_MS    0U       /**< Non-blocking queue send */

/* Sensor physical limits - based on real-world constraints */
#define TEMP_MIN_C               15.0f     /**< Minimum temperature (°C) */
#define TEMP_MAX_C               35.0f     /**< Maximum temperature (°C) */
#define HUMIDITY_MIN_PCT         30.0f     /**< Minimum humidity (%) */
#define HUMIDITY_MAX_PCT         90.0f     /**< Maximum humidity (%) */
#define PRESSURE_MIN_HPA         990.0f    /**< Minimum pressure (hPa) */
#define PRESSURE_MAX_HPA         1030.0f   /**< Maximum pressure (hPa) */
#define PULSE_MIN_BPM            50U       /**< Minimum pulse (BPM) */
#define PULSE_MAX_BPM            100U      /**< Maximum pulse (BPM) */
#define CPU_USAGE_MIN_PCT        10U       /**< Minimum CPU usage (%) */
#define CPU_USAGE_MAX_PCT        90U       /**< Maximum CPU usage (%) */
#define MEMORY_USAGE_MIN_PCT     20U       /**< Minimum memory usage (%) */
#define MEMORY_USAGE_MAX_PCT     80U       /**< Maximum memory usage (%) */
#define BATTERY_LEVEL_MIN_PCT     20U       /**< Minimum battery level (%) */
#define BATTERY_LEVEL_MAX_PCT     100U      /**< Maximum battery level (%) */

/* Simulation parameters */
#define BATTERY_DISCHARGE_RATE   0.0001f   /**< Battery discharge per second */
#define NOISE_AMPLITUDE_TEMP     0.05f     /**< Temperature noise amplitude */
#define NOISE_AMPLITUDE_HUMIDITY 0.3f      /**< Humidity noise amplitude */
#define NOISE_AMPLITUDE_PRESSURE 0.2f      /**< Pressure noise amplitude */
#define NOISE_AMPLITUDE_PULSE    0.5f      /**< Pulse noise amplitude */
#define NOISE_AMPLITUDE_CPU     2.0f      /**< CPU usage noise amplitude */
#define NOISE_AMPLITUDE_MEMORY   1.5f      /**< Memory usage noise amplitude */
#define NOISE_AMPLITUDE_BATTERY  0.1f      /**< Battery noise amplitude */

/* Frequency constants for realistic variations */
#define TEMP_FREQ_FACTOR         (2.0f * 3.14159f / (float)SECONDS_PER_DAY)
#define PULSE_FREQ_FACTOR        0.0001f
#define CPU_FREQ_FACTOR          0.0002f
#define MEMORY_FREQ_FACTOR       0.000125f
#define HUMIDITY_FREQ_FACTOR     0.00008f
#define PRESSURE_FREQ_FACTOR     0.00005f

/* ── Type Definitions ───────────────────────────────────────────── */

/**
 * @brief Complete sensor data structure
 * 
 * Contains all sensor readings with proper fixed-width types
 * for deterministic memory layout and ARM optimization.
 */
typedef struct {
    float        temperature;    /**< Temperature in Celsius */
    float        humidity;       /**< Relative humidity in % */
    float        pressure;       /**< Atmospheric pressure in hPa */
    uint16_t     pulse;          /**< Heart rate in BPM */
    uint8_t      cpu_usage;      /**< CPU utilization in % */
    uint8_t      memory_usage;   /**< Memory utilization in % */
    uint16_t     battery_level;  /**< Battery charge level in % */
    uint8_t      led_status;     /**< LED indicator state */
} FullSensorData_t;

/* ── Static Data ───────────────────────────────────────────────── */

/** Global sensor data instance - shared with web interface */
static FullSensorData_t s_sensor_data = {
    .temperature    = 24.5f,
    .humidity       = 60.0f,
    .pressure       = 1013.25f,
    .pulse          = 72U,
    .cpu_usage      = 45U,
    .memory_usage   = 65U,
    .battery_level  = 87U,
    .led_status     = 0U
};

/** Simulation counters */
static uint32_t s_read_count = 0U;
static uint32_t s_time_counter = 0U;

/** External queue handle - must be provided by main application */
extern QueueHandle_t g_sensor_queue;

/* ── Helper Functions ──────────────────────────────────────────── */

/**
 * @brief Clamp float value between min and max
 * @param value Value to clamp
 * @param min Minimum allowed value
 * @param max Maximum allowed value
 * @return Clamped value
 */
static inline float clamp_float(const float value, const float min, const float max)
{
    if (value > max) {
        return max;
    } else if (value < min) {
        return min;
    }
    return value;
}

/**
 * @brief Clamp uint16_t value between min and max
 * @param value Value to clamp
 * @param min Minimum allowed value
 * @param max Maximum allowed value
 * @return Clamped value
 */
static inline uint16_t clamp_uint16(const uint16_t value, const uint16_t min, const uint16_t max)
{
    if (value > max) {
        return max;
    } else if (value < min) {
        return min;
    }
    return value;
}

/**
 * @brief Clamp uint8_t value between min and max
 * @param value Value to clamp
 * @param min Minimum allowed value
 * @param max Maximum allowed value
 * @return Clamped value
 */
static inline uint8_t clamp_uint8(const uint8_t value, const uint8_t min, const uint8_t max)
{
    if (value > max) {
        return max;
    } else if (value < min) {
        return min;
    }
    return value;
}

/**
 * @brief Generate realistic temperature reading
 * 
 * Simulates daily temperature variations using sine function
 * with added noise for realism.
 * 
 * @return Temperature in Celsius
 */
static float generate_temperature(void)
{
    /* Daily temperature variation - peaks around 2PM */
    const float hour_factor = sinf((float)(s_time_counter % SECONDS_PER_DAY) * TEMP_FREQ_FACTOR);
    const float base_temp = 22.0f + 3.0f * hour_factor;
    
    /* Add realistic measurement noise */
    const float noise = ((float)(s_read_count % 20U) - 10.0f) * NOISE_AMPLITUDE_TEMP;
    
    s_sensor_data.temperature = clamp_float(base_temp + noise, TEMP_MIN_C, TEMP_MAX_C);
    return s_sensor_data.temperature;
}

/**
 * @brief Generate realistic humidity reading
 * 
 * Simulates humidity variations with slower frequency
 * than temperature to represent different physical processes.
 * 
 * @return Relative humidity in percentage
 */
static float generate_humidity(void)
{
    const float humidity_base = 60.0f + sinf((float)s_time_counter * HUMIDITY_FREQ_FACTOR) * 15.0f;
    const float humidity_noise = ((float)(s_read_count % 18U) - 9.0f) * NOISE_AMPLITUDE_HUMIDITY;
    
    s_sensor_data.humidity = clamp_float(humidity_base + humidity_noise, HUMIDITY_MIN_PCT, HUMIDITY_MAX_PCT);
    return s_sensor_data.humidity;
}

/**
 * @brief Generate realistic atmospheric pressure reading
 * 
 * Simulates barometric pressure variations typically caused
 * by weather changes.
 * 
 * @return Atmospheric pressure in hPa
 */
static float generate_pressure(void)
{
    const float pressure_base = 1013.25f + sinf((float)s_time_counter * PRESSURE_FREQ_FACTOR) * 10.0f;
    const float pressure_noise = ((float)(s_read_count % 22U) - 11.0f) * NOISE_AMPLITUDE_PRESSURE;
    
    s_sensor_data.pressure = clamp_float(pressure_base + pressure_noise, PRESSURE_MIN_HPA, PRESSURE_MAX_HPA);
    return s_sensor_data.pressure;
}

/**
 * @brief Generate realistic pulse reading
 * 
 * Simulates heart rate variations based on activity level
 * and circadian rhythm.
 * 
 * @return Heart rate in beats per minute
 */
static uint16_t generate_pulse(void)
{
    const float pulse_base = 70.0f + sinf((float)s_time_counter * PULSE_FREQ_FACTOR) * 10.0f;
    const float pulse_noise = ((float)(s_read_count % 15U) - 7.0f) * NOISE_AMPLITUDE_PULSE;
    
    s_sensor_data.pulse = clamp_uint16((uint16_t)(pulse_base + pulse_noise), PULSE_MIN_BPM, PULSE_MAX_BPM);
    return s_sensor_data.pulse;
}

/**
 * @brief Generate realistic CPU usage reading
 * 
 * Simulates CPU load variations with periodic peaks
 * representing processing bursts.
 * 
 * @return CPU usage in percentage
 */
static uint8_t generate_cpu_usage(void)
{
    const float cpu_base = 30.0f + sinf((float)s_time_counter * CPU_FREQ_FACTOR) * 20.0f;
    const float cpu_noise = ((float)(s_read_count % 10U) - 5.0f) * NOISE_AMPLITUDE_CPU;
    
    s_sensor_data.cpu_usage = clamp_uint8((uint8_t)(cpu_base + cpu_noise), CPU_USAGE_MIN_PCT, CPU_USAGE_MAX_PCT);
    return s_sensor_data.cpu_usage;
}

/**
 * @brief Generate realistic memory usage reading
 * 
 * Simulates memory allocation patterns typical for
 * embedded IoT applications.
 * 
 * @return Memory usage in percentage
 */
static uint8_t generate_memory_usage(void)
{
    const float mem_base = 40.0f + sinf((float)s_time_counter * MEMORY_FREQ_FACTOR) * 20.0f;
    const float mem_noise = ((float)(s_read_count % 12U) - 6.0f) * NOISE_AMPLITUDE_MEMORY;
    
    s_sensor_data.memory_usage = clamp_uint8((uint8_t)(mem_base + mem_noise), MEMORY_USAGE_MIN_PCT, MEMORY_USAGE_MAX_PCT);
    return s_sensor_data.memory_usage;
}

/**
 * @brief Generate realistic battery level reading
 * 
 * Simulates gradual battery discharge with small variations
 * due to temperature and load changes.
 * 
 * @return Battery level in percentage
 */
static uint16_t generate_battery_level(void)
{
    const float discharge_amount = (float)s_time_counter * BATTERY_DISCHARGE_RATE;
    const float battery_noise = ((float)(s_read_count % 25U) - 12.0f) * NOISE_AMPLITUDE_BATTERY;
    
    s_sensor_data.battery_level = clamp_uint16((uint16_t)(87.0f - discharge_amount + battery_noise), 
                                             BATTERY_LEVEL_MIN_PCT, BATTERY_LEVEL_MAX_PCT);
    return s_sensor_data.battery_level;
}

/* ── Public API Functions ───────────────────────────────────────── */

/**
 * @brief Initialize sensor subsystem
 * 
 * Sets up I2C interface for real sensor communication.
 * In simulation mode, initializes counters and data structures.
 * 
 * @return true if initialization successful, false otherwise
 */
bool sensor_init(void)
{
    /* Reset simulation counters */
    s_read_count = 0U;
    s_time_counter = 0U;
    
    /* Initialize I2C for real sensor communication */
    /* In production, this would configure actual I2C peripheral */
    /* HAL_I2C_Init(&hi2c1); */
    
    return true;
}

/**
 * @brief Read temperature sensor
 * @return Temperature in Celsius
 */
float read_sensor_temperature(void)
{
    return generate_temperature();
}

/**
 * @brief Read humidity sensor
 * @return Relative humidity in percentage
 */
float read_sensor_humidity(void)
{
    return generate_humidity();
}

/**
 * @brief Read pressure sensor
 * @return Atmospheric pressure in hPa
 */
float read_sensor_pressure(void)
{
    return generate_pressure();
}

/**
 * @brief Read pulse sensor
 * @return Heart rate in beats per minute
 */
uint16_t read_sensor_pulse(void)
{
    return generate_pulse();
}

/**
 * @brief Read CPU usage
 * @return CPU utilization in percentage
 */
uint8_t read_cpu_usage(void)
{
    return generate_cpu_usage();
}

/**
 * @brief Read memory usage
 * @return Memory utilization in percentage
 */
uint8_t read_memory_usage(void)
{
    return generate_memory_usage();
}

/**
 * @brief Read battery level
 * @return Battery charge level in percentage
 */
uint16_t read_battery_level(void)
{
    return generate_battery_level();
}

/* ── FreeRTOS Task Implementation ───────────────────────────────── */

/**
 * @brief Sensor data acquisition task
 * 
 * This task runs periodically to generate realistic sensor data
 * and enqueue it for processing by other tasks.
 * 
 * @param pvParameters Task parameters (unused)
 */
void sensor_task(void *pvParameters)
{
    (void)pvParameters; /* Suppress unused parameter warning */
    
    const TickType_t xFrequency = pdMS_TO_TICKS(SENSOR_UPDATE_PERIOD_MS);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    /* Initialize sensor subsystem */
    if (!sensor_init()) {
        /* Initialization failed - delete task */
        vTaskDelete(NULL);
        return;
    }
    
    /* Main task loop - never returns */
    for (;;) {
        /* Update simulation time counter */
        s_time_counter += (SENSOR_UPDATE_PERIOD_MS / 1000U);
        
        /* Generate all sensor readings */
        generate_temperature();
        generate_humidity();
        generate_pressure();
        generate_pulse();
        generate_cpu_usage();
        generate_memory_usage();
        generate_battery_level();
        
        /* Send sensor data to queue for other tasks */
        if (xQueueSend(g_sensor_queue, &s_sensor_data, pdMS_TO_TICKS(SENSOR_QUEUE_TIMEOUT_MS)) != pdTRUE) {
            /* Queue full - increment error counter or log */
            /* In production, this should trigger error handling */
            #ifdef DEBUG_ENABLED
            /* Note: printf should be replaced with proper logging in production */
            #endif
        }
        
        /* Increment read counter for noise generation */
        s_read_count++;
        
        /* Wait for next cycle - precise timing */
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
    
    /* This line should never be reached */
    vTaskDelete(NULL);
}

/* ── ISR-Safe Functions (if needed) ───────────────────────────────── */

/**
 * @brief Get current sensor data (ISR-safe)
 * 
 * This function can be called from interrupt context to get
 * the latest sensor readings without blocking.
 * 
 * @return Pointer to current sensor data structure
 */
const FullSensorData_t* sensor_get_data_isr(void)
{
    return &s_sensor_data;
}

/* ── Timing-Sensitive Regions (requires manual review) ───────────── */

/**
 * @note The following regions require manual review:
 * 
 * 1. I2C initialization timing - ensure proper clock setup
 * 2. Queue send timeout - verify non-blocking behavior is acceptable
 * 3. Float operations - verify FPU is enabled and configured
 * 4. Task timing - verify 2-second period meets system requirements
 * 
 * <REVIEW_REQUIRED>
 * - I2C peripheral configuration timing
 * - Float operation performance on target MCU
 * - Queue overflow handling strategy
 * </REVIEW_REQUIRED>
 */
