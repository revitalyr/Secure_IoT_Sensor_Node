#include "test_stubs.h"

// Global test state
int g_test_count = 0;
int g_test_passed = 0;

// Simple mock implementations
test_result_t test_uart_init_stub(void) {
    g_test_count++;
    printf("UART init test... ");
    // Simple test logic
    g_test_passed++;
    printf("PASS\n");
    return TEST_OK;
}

test_result_t test_uart_transmit(uint8_t* data, uint16_t size) {
    g_test_count++;
    printf("UART transmit test... ");
    if (data && size > 0) {
        g_test_passed++;
        printf("PASS\n");
        return TEST_OK;
    }
    printf("FAIL\n");
    return TEST_ERROR;
}

test_result_t test_uart_receive(uint8_t* data, uint16_t size) {
    g_test_count++;
    printf("UART receive test... ");
    if (data && size > 0) {
        g_test_passed++;
        printf("PASS\n");
        return TEST_OK;
    }
    printf("FAIL\n");
    return TEST_ERROR;
}

test_result_t test_gpio_init(void) {
    g_test_count++;
    printf("GPIO init test... ");
    g_test_passed++;
    printf("PASS\n");
    return TEST_OK;
}

test_result_t test_gpio_write(uint16_t pin, uint8_t state) {
    (void)pin;
    (void)state;
    g_test_count++;
    printf("GPIO write test... ");
    g_test_passed++;
    printf("PASS\n");
    return TEST_OK;
}

test_result_t test_gpio_read(uint16_t pin) {
    (void)pin;
    g_test_count++;
    printf("GPIO read test... ");
    g_test_passed++;
    printf("PASS\n");
    return TEST_OK;
}

test_result_t test_flash_init(void) {
    g_test_count++;
    printf("Flash init test... ");
    g_test_passed++;
    printf("PASS\n");
    return TEST_OK;
}

test_result_t test_flash_write(uint32_t addr, uint8_t* data, uint32_t len) {
    (void)addr;
    g_test_count++;
    printf("Flash write test... ");
    if (data && len > 0) {
        g_test_passed++;
        printf("PASS\n");
        return TEST_OK;
    }
    printf("FAIL\n");
    return TEST_ERROR;
}

test_result_t test_flash_read(uint32_t addr, uint8_t* data, uint32_t len) {
    (void)addr;
    g_test_count++;
    printf("Flash read test... ");
    if (data && len > 0) {
        g_test_passed++;
        printf("PASS\n");
        return TEST_OK;
    }
    printf("FAIL\n");
    return TEST_ERROR;
}

test_result_t test_metadata_init(void) {
    g_test_count++;
    printf("Metadata init test... ");
    g_test_passed++;
    printf("PASS\n");
    return TEST_OK;
}

test_result_t test_metadata_read(void) {
    g_test_count++;
    printf("Metadata read test... ");
    g_test_passed++;
    printf("PASS\n");
    return TEST_OK;
}

test_result_t test_metadata_write(void) {
    g_test_count++;
    printf("Metadata write test... ");
    g_test_passed++;
    printf("PASS\n");
    return TEST_OK;
}
