/**
 * @file test_framework.h
 * @brief Test framework interface for unit and integration testing
 * 
 * This header defines the test framework interface, assertion macros,
 * test suite structures, and testing utilities for comprehensive
 * unit and integration testing of the Secure IoT Sensor Node.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test Framework Macros */

/**
 * @brief Test assertion macro
 * 
 * Asserts that a condition is true during test execution.
 * Reports failure with file, line, and condition information.
 * 
 * @param condition Boolean condition to test
 */
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s:%d - Assertion failed: %s\n", __FILE__, __LINE__, #condition); \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("FAIL: %s:%d - Expected %ld, got %ld\n", __FILE__, __LINE__, (long)(expected), (long)(actual)); \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            printf("FAIL: %s:%d - Pointer is NULL\n", __FILE__, __LINE__); \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != NULL) { \
            printf("FAIL: %s:%d - Pointer is not NULL\n", __FILE__, __LINE__); \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_MEM_EQUAL(expected, actual, size) \
    do { \
        if (memcmp((expected), (actual), (size)) != 0) { \
            printf("FAIL: %s:%d - Memory not equal\n", __FILE__, __LINE__); \
            return 0; \
        } \
    } while(0)

// Test function type
typedef int (*test_func_t)(void);

// Test case structure
typedef struct {
    const char* name;
    test_func_t func;
} test_case_t;

// Test suite structure
typedef struct {
    const char* name;
    test_case_t* tests;
    int count;
} test_suite_t;

// Test runner functions
int run_test_suite(const test_suite_t* suite);
int run_test_case(const test_case_t* test);
void print_test_summary(void);

// Mock functions for hardware
void mock_init(void);
void mock_reset(void);

/* Global Test State */
extern int tests_run;     /**< Total number of tests executed */
extern int tests_passed;  /**< Number of tests passed */
extern int tests_failed;  /**< Number of tests failed */

#endif // TEST_FRAMEWORK_H
