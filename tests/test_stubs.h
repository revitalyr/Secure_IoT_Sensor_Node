/**
 * @file test_stubs.h
 * @brief Test stub functions and types for unit testing
 * 
 * This header defines test stub functions, result types, and mock
 * interfaces for unit testing hardware and system components
 * without actual hardware dependencies.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef TEST_STUBS_H
#define TEST_STUBS_H

#include <stdint.h>
#include <stdio.h>

/**
 * @brief Test result enumeration
 * 
 * Result codes for test function execution status.
 */
typedef enum {
    TEST_OK = 0,        /**< Test completed successfully */
    TEST_ERROR = 1      /**< Test failed with error */
} test_result_t;

/* Mock Functions for Testing */

/**
 * @brief Test UART initialization stub
 * 
 * Mock function for UART initialization testing.
 * 
 * @return test_result_t TEST_OK on success
 */
test_result_t test_uart_init_stub(void);

/**
 * @brief Test UART transmission stub
 * 
 * Mock function for UART transmission testing.
 * 
 * @param data Pointer to data buffer
 * @param size Size of data in bytes
 * @return test_result_t TEST_OK on success
 */
test_result_t test_uart_transmit(uint8_t* data, uint16_t size);

/**
 * @brief Test UART reception stub
 * 
 * Mock function for UART reception testing.
 * 
 * @param data Pointer to receive buffer
 * @param size Size of buffer in bytes
 * @return test_result_t TEST_OK on success
 */
test_result_t test_uart_receive(uint8_t* data, uint16_t size);

/* GPIO Mock Functions */

/**
 * @brief Test GPIO initialization stub
 * 
 * Mock function for GPIO initialization testing.
 * 
 * @return test_result_t TEST_OK on success
 */
test_result_t test_gpio_init(void);

/**
 * @brief Test GPIO write stub
 * 
 * Mock function for GPIO write operation testing.
 * 
 * @param pin GPIO pin number
 * @param state Pin state (0 or 1)
 * @return test_result_t TEST_OK on success
 */
test_result_t test_gpio_write(uint16_t pin, uint8_t state);

/**
 * @brief Test GPIO read stub
 * 
 * Mock function for GPIO read operation testing.
 * 
 * @param pin GPIO pin number
 * @return test_result_t TEST_OK on success
 */
test_result_t test_gpio_read(uint16_t pin);

/* Flash Mock Functions */

/**
 * @brief Test flash initialization stub
 * 
 * Mock function for flash initialization testing.
 * 
 * @return test_result_t TEST_OK on success
 */
test_result_t test_flash_init(void);

/**
 * @brief Test flash write stub
 * 
 * Mock function for flash write operation testing.
 * 
 * @param addr Flash address
 * @param data Pointer to data buffer
 * @param len Data length in bytes
 * @return test_result_t TEST_OK on success
 */
test_result_t test_flash_write(uint32_t addr, uint8_t* data, uint32_t len);

/**
 * @brief Test flash read stub
 * 
 * Mock function for flash read operation testing.
 * 
 * @param addr Flash address
 * @param data Pointer to receive buffer
 * @param len Buffer length in bytes
 * @return test_result_t TEST_OK on success
 */
test_result_t test_flash_read(uint32_t addr, uint8_t* data, uint32_t len);

/* Metadata Mock Functions */

/**
 * @brief Test metadata initialization stub
 * 
 * Mock function for metadata initialization testing.
 * 
 * @return test_result_t TEST_OK on success
 */
test_result_t test_metadata_init(void);

/**
 * @brief Test metadata read stub
 * 
 * Mock function for metadata read operation testing.
 * 
 * @return test_result_t TEST_OK on success
 */
test_result_t test_metadata_read(void);

/**
 * @brief Test metadata write stub
 * 
 * Mock function for metadata write operation testing.
 * 
 * @return test_result_t TEST_OK on success
 */
test_result_t test_metadata_write(void);

/* Global Test State Variables */
extern int g_test_count;    /**< Total number of tests executed */
extern int g_test_passed;   /**< Number of tests passed */

#endif // TEST_STUBS_H
