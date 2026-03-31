/**
 * @file test_uart.c
 * @brief Unit tests for UART driver functionality
 * 
 * This file contains unit tests for UART communication including
 * initialization, data transmission, string operations, and buffer
 * management.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#include "../test_framework.h"
#include "../mocks/hal_mock.h"
#include "../../drivers/uart.h"
#include <string.h>

/**
 * @brief Test UART initialization
 * 
 * Verifies that UART peripheral can be properly initialized
 * and internal buffers are reset.
 * 
 * @return int 1 if test passes, 0 otherwise
 */
int test_uart_init(void)
{
    mock_reset();
    
    uart_init();
    
    // Verify UART initialization state
    TEST_ASSERT(g_mock_state.uart_tx_len == 0);
    TEST_ASSERT(g_mock_state.uart_rx_len == 0);
    
    return 1;
}

/**
 * @brief Test UART write functionality
 * 
 * Verifies that data can be written to UART transmission buffer
 * correctly.
 * 
 * @return int 1 if test passes, 0 otherwise
 */
int test_uart_write(void)
{
    mock_reset();
    uart_init();
    
    uint8_t test_data[] = "Hello World";
    const uint16_t len = sizeof(test_data) - 1; // Exclude null terminator
    
    uart_write(test_data, len);
    
    TEST_ASSERT(g_mock_state.uart_tx_len == len);
    TEST_ASSERT_MEM_EQUAL(test_data, g_mock_state.uart_tx_buffer, len);
    
    return 1;
}

/**
 * @brief Test UART string write functionality
 * 
 * Verifies that null-terminated strings can be written
 * to UART correctly.
 * 
 * @return int 1 if test passes, 0 otherwise
 */
int test_uart_write_string(void)
{
    mock_reset();
    uart_init();
    
    const char* test_str = "Test String";
    
    uart_write_string(test_str);
    
    const uint16_t expected_len = strlen(test_str);
    TEST_ASSERT(g_mock_state.uart_tx_len == expected_len);
    TEST_ASSERT_MEM_EQUAL(test_str, g_mock_state.uart_tx_buffer, expected_len);
    
    return 1;
}

/**
 * @brief Test UART read functionality
 * 
 * Verifies that data can be read from UART receive buffer
 * correctly using mock data injection.
 * 
 * @return int 1 if test passes, 0 otherwise
 */
int test_uart_read(void)
{
    mock_reset();
    uart_init();
    
    uint8_t inject_data[] = "RX Data";
    const uint16_t inject_len = sizeof(inject_data) - 1;
    
    mock_uart_inject_data(inject_data, inject_len);
    
    uint8_t read_buffer[32];
    const uint16_t read_len = uart_read(read_buffer, sizeof(read_buffer));
    
    TEST_ASSERT(read_len == inject_len);
    TEST_ASSERT_MEM_EQUAL(inject_data, read_buffer, inject_len);
    
    return 1;
}

/**
 * @brief Test UART buffer overflow handling
 * 
 * Verifies that UART driver handles buffer overflow conditions
 * gracefully.
 * 
 * @return int 1 if test passes, 0 otherwise
 */
int test_uart_buffer_overflow(void)
{
    mock_reset();
    uart_init();
    
    // Inject data larger than buffer
    uint8_t large_data[2048];
    memset(large_data, 0xAA, sizeof(large_data));
    
    mock_uart_inject_data(large_data, sizeof(large_data));
    
    uint8_t read_buffer[1024];
    uint16_t read_len = uart_read(read_buffer, sizeof(read_buffer));
    
    // Should read up to buffer size
    TEST_ASSERT(read_len <= sizeof(read_buffer));
    
    return 1;
}

// UART test suite
test_case_t uart_tests[] = {
    {"UART Initialization", test_uart_init},
    {"UART Write", test_uart_write},
    {"UART Write String", test_uart_write_string},
    {"UART Read", test_uart_read},
    {"UART Buffer Overflow", test_uart_buffer_overflow},
};

test_suite_t uart_test_suite = {
    "UART Driver Tests",
    uart_tests,
    sizeof(uart_tests) / sizeof(uart_tests[0])
};
