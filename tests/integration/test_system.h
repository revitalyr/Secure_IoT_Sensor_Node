#pragma once

#include "../test_framework.h"

// Test functions
int test_sensor_data_flow(void);
int test_uart_communication_protocol(void);
int test_flash_storage_integration(void);
int test_system_error_handling(void);
int test_concurrent_tasks(void);

// Test suite
extern test_case_t integration_tests[];
extern test_suite_t integration_test_suite;
