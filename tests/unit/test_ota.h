#pragma once

#include "../test_framework.h"

// Test functions
int test_ota_metadata_init(void);
int test_ota_metadata_read_write(void);
int test_ota_metadata_set_pending(void);
int test_ota_metadata_commit(void);
int test_ota_metadata_rollback(void);
int test_ota_metadata_get_active_slot(void);
int test_ota_metadata_is_update_pending(void);

// Test suite
extern test_case_t ota_tests[];
extern test_suite_t ota_test_suite;
