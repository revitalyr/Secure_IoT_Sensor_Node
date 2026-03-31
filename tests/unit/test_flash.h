#pragma once

#include "../test_framework.h"

// Test functions
int test_flash_storage_init(void);
int test_flash_storage_write(void);
int test_flash_storage_read(void);
int test_flash_storage_erase(void);
int test_wear_level_init(void);
int test_wear_level_write(void);
int test_wear_level_read(void);
int test_wear_level_write_count(void);

// Test suite
extern test_case_t flash_tests[];
extern test_suite_t flash_test_suite;
