#pragma once

#include "../test_framework.h"

// Test functions
int test_uart_init(void);
int test_uart_write(void);
int test_uart_write_string(void);
int test_uart_read(void);
int test_uart_buffer_overflow(void);

// Test suite
extern test_case_t uart_tests[];
extern test_suite_t uart_test_suite;
