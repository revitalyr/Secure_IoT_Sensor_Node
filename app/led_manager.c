/**
 * @file led_manager.c
 * @brief LED state machine implementation for device status indication
 * 
 * This file implements the LED management system with state machine
 * for production-ready embedded device status indication.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#include "led_manager.h"
#include "hal_mock.h"  // Replace with actual HAL
#include "FreeRTOS.h"
#include "task.h"

// LED timing constants (in milliseconds)
#define LED_SLOW_BLINK_PERIOD    1000    // 1Hz (500ms on, 500ms off)
#define LED_FAST_BLINK_PERIOD    250     // 4Hz (125ms on, 125ms off)
#define LED_SOLID_ON_TIME       0        // Always on

// Current LED state
static led_state_t current_led_state = LED_OFF;
static uint32_t led_timer = 0;
static uint8_t led_state_counter = 0;

void led_manager_init(void)
{
    // Initialize GPIO for LED (replace with actual HAL call)
    // HAL_GPIO_Init(&LED_GPIO);
    
    // Start with slow blink (normal operation)
    led_set_state(LED_SLOW_BLINK);
}

void led_set_state(led_state_t state)
{
    current_led_state = state;
    led_timer = 0;
    led_state_counter = 0;
    
    // Set initial LED state based on new state
    switch (state) {
        case LED_SLOW_BLINK:
        case LED_FAST_BLINK:
            // Turn LED on initially for blink patterns
            // HAL_GPIO_WritePin(LED_GPIO, GPIO_PIN_SET);
            gpio_write(LED_PIN, 1);
            break;
            
        case LED_SOLID:
            // Turn LED on solid
            // HAL_GPIO_WritePin(LED_GPIO, GPIO_PIN_SET);
            gpio_write(LED_PIN, 1);
            break;
            
        case LED_OFF:
        default:
            // Turn LED off
            // HAL_GPIO_WritePin(LED_GPIO, GPIO_PIN_RESET);
            gpio_write(LED_PIN, 0);
            break;
    }
}

led_state_t led_get_state(void)
{
    return current_led_state;
}

void led_task(void *arg)
{
    TickType_t last_wake_time = xTaskGetTickCount();
    uint32_t blink_period;
    uint8_t led_on = 1;
    
    (void)arg;  // Suppress unused parameter warning
    
    while (1) {
        switch (current_led_state) {
            case LED_SLOW_BLINK:
                blink_period = LED_SLOW_BLINK_PERIOD / 2;  // Half period for toggle
                break;
                
            case LED_FAST_BLINK:
                blink_period = LED_FAST_BLINK_PERIOD / 2;  // Half period for toggle
                break;
                
            case LED_SOLID:
                // LED stays on, just wait
                vTaskDelay(pdMS_TO_TICKS(100));
                continue;
                
            case LED_OFF:
            default:
                // LED stays off, just wait
                vTaskDelay(pdMS_TO_TICKS(100));
                continue;
        }
        
        // Toggle LED for blink patterns
        led_on = !led_on;
        // HAL_GPIO_WritePin(LED_GPIO, led_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
        gpio_write(LED_PIN, led_on ? 1 : 0);
        
        // Wait for next toggle
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(blink_period));
    }
}
