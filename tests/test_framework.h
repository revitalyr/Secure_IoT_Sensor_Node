#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Test framework macros
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

// Mock functions for hardware
void mock_init(void);
void mock_reset(void);

// Global test state
extern int tests_run;
extern int tests_passed;
extern int tests_failed;
