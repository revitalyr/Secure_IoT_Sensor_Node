/**
 * @file led_manager.h
 * @brief LED state machine for device status indication
 * 
 * This header defines the LED management system with state machine
 * for production-ready embedded device status indication.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include "system_config.h"
#include <stdint.h>

/**
 * @brief LED state enumeration
 * 
 * Defines LED states for different device conditions.
 * Classic embedded pattern for device status indication.
 */
typedef enum {
    LED_SLOW_BLINK = 0,    /**< Normal operation - 1Hz blink */
    LED_FAST_BLINK = 1,    /**< OTA in progress - 4Hz blink */
    LED_SOLID = 2,         /**< Error condition - solid on */
    LED_OFF = 3            /**< Device shutdown - off */
} led_state_t;

/**
 * @brief Initialize LED manager
 * 
 * Sets up GPIO for LED control and initializes state machine.
 */
void led_manager_init(void);

/**
 * @brief Set LED state
 * 
 * Updates LED state and starts corresponding blink pattern.
 * 
 * @param state New LED state
 */
void led_set_state(led_state_t state);

/**
 * @brief Get current LED state
 * 
 * @return Current LED state
 */
led_state_t led_get_state(void);

/**
 * @brief LED update task
 * 
 * FreeRTOS task that handles LED state machine timing.
 * 
 * @param arg Task argument (unused)
 */
void led_task(void *arg);

#endif // LED_MANAGER_H
