#pragma once

#include "system_config.h"
#include <stdint.h>

void sensor_task(void *arg);
float read_sensor_temperature(void);
void sensor_init(void);
