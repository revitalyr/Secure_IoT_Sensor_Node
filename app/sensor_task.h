/**
 * @file sensor_task.h
 * @brief Sensor data acquisition and processing interface
 * 
 * This header defines the sensor task interface, data structures,
 * and public API functions for the Secure IoT Sensor Node.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include "system_config.h"
#include <stdint.h>
#include <stdbool.h>

/* ── Public Type Definitions ───────────────────────────────────── */

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

/* ── Public API Functions ───────────────────────────────────────── */

/**
 * @brief Initialize sensor subsystem
 * 
 * Sets up I2C interface for real sensor communication.
 * In simulation mode, initializes counters and data structures.
 * 
 * @return true if initialization successful, false otherwise
 */
bool sensor_init(void);

/**
 * @brief Read temperature sensor
 * @return Temperature in Celsius
 */
float read_sensor_temperature(void);

/**
 * @brief Read humidity sensor
 * @return Relative humidity in percentage
 */
float read_sensor_humidity(void);

/**
 * @brief Read pressure sensor
 * @return Atmospheric pressure in hPa
 */
float read_sensor_pressure(void);

/**
 * @brief Read pulse sensor
 * @return Heart rate in beats per minute
 */
uint16_t read_sensor_pulse(void);

/**
 * @brief Read CPU usage
 * @return CPU utilization in percentage
 */
uint8_t read_cpu_usage(void);

/**
 * @brief Read memory usage
 * @return Memory utilization in percentage
 */
uint8_t read_memory_usage(void);

/**
 * @brief Read battery level
 * @return Battery charge level in percentage
 */
uint16_t read_battery_level(void);

/**
 * @brief Sensor data acquisition task
 * 
 * This task runs periodically to generate realistic sensor data
 * and enqueue it for processing by other tasks.
 * 
 * @param pvParameters Task parameters (unused)
 */
void sensor_task(void *pvParameters);

/**
 * @brief Get current sensor data (ISR-safe)
 * 
 * This function can be called from interrupt context to get
 * the latest sensor readings without blocking.
 * 
 * @return Pointer to current sensor data structure
 */
const FullSensorData_t* sensor_get_data_isr(void);

/* ── Configuration Constants ───────────────────────────────────── */

/** Sensor task configuration */
#define SENSOR_UPDATE_PERIOD_MS      2000U

/* Sensor physical limits - based on real-world constraints */
#define TEMP_MIN_C                   15.0f     /**< Minimum temperature (°C) */
#define TEMP_MAX_C                   35.0f     /**< Maximum temperature (°C) */
#define HUMIDITY_MIN_PCT             30.0f     /**< Minimum humidity (%) */
#define HUMIDITY_MAX_PCT             90.0f     /**< Maximum humidity (%) */
#define PRESSURE_MIN_HPA             990.0f    /**< Minimum pressure (hPa) */
#define PRESSURE_MAX_HPA             1030.0f   /**< Maximum pressure (hPa) */
#define PULSE_MIN_BPM                50U       /**< Minimum pulse (BPM) */
#define PULSE_MAX_BPM                100U      /**< Maximum pulse (BPM) */
#define CPU_USAGE_MIN_PCT            10U       /**< Minimum CPU usage (%) */
#define CPU_USAGE_MAX_PCT            90U       /**< Maximum CPU usage (%) */
#define MEMORY_USAGE_MIN_PCT         20U       /**< Minimum memory usage (%) */
#define MEMORY_USAGE_MAX_PCT         80U       /**< Maximum memory usage (%) */
#define BATTERY_LEVEL_MIN_PCT        20U       /**< Minimum battery level (%) */
#define BATTERY_LEVEL_MAX_PCT        100U      /**< Maximum battery level (%) */

#endif /* SENSOR_TASK_H */
