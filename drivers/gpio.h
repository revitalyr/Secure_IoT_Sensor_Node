#pragma once

#include "system_config.h"
#include <stdint.h>

void gpio_init(void);
void led_set_status(uint8_t on);
void led_blink(uint8_t count, uint32_t delay_ms);
uint8_t button_is_pressed(void);
void gpio_set_output(uint16_t pin, GPIO_TypeDef* port, uint8_t state);
uint8_t gpio_get_input(uint16_t pin, GPIO_TypeDef* port);
