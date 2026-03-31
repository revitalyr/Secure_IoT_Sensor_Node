/**
 * @file test_ota.c
 * @brief Unit tests for OTA metadata management
 * 
 * This file contains unit tests for OTA (Over-The-Air) update metadata
 * functionality including initialization, read/write operations, update
 * pending state management, and slot switching.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#include "../test_framework.h"
#include "../mocks/hal_mock.h"
#include "../../middleware/crypto/ota_metadata.h"
#include <string.h>

/**
 * @brief Test OTA metadata initialization
 * 
 * Verifies that the OTA metadata system can be properly initialized
 * and returns BOOT_OK status.
 * 
 * @return int 1 if test passes, 0 otherwise
 */
int test_ota_metadata_init(void)
{
    mock_reset();
    
    boot_error_t result = ota_metadata_init();
    
    TEST_ASSERT(result == BOOT_OK);
    
    return 1;
}

/**
 * @brief Test OTA metadata read/write operations
 * 
 * Verifies that OTA metadata can be written to storage and read back
 * with data integrity preserved.
 * 
 * @return int 1 if test passes, 0 otherwise
 */
int test_ota_metadata_read_write(void)
{
    mock_reset();
    ota_metadata_init();
    
    // Create test metadata
    ota_metadata_t test_meta = {
        .active_slot = 1,
        .update_pending = 1,
        .crc = 0x12345678,
        .size = 0x10000,
        .version = 2,
        .reserved = {0, 0, 0}
    };
    
    // Write metadata
    boot_error_t result = ota_metadata_write(&test_meta);
    TEST_ASSERT(result == BOOT_OK);
    
    // Read back metadata
    ota_metadata_t read_meta;
    result = ota_metadata_read(&read_meta);
    TEST_ASSERT(result == BOOT_OK);
    
    // Verify data integrity
    TEST_ASSERT_EQUAL(test_meta.active_slot, read_meta.active_slot);
    TEST_ASSERT_EQUAL(test_meta.update_pending, read_meta.update_pending);
    TEST_ASSERT_EQUAL(test_meta.crc, read_meta.crc);
    TEST_ASSERT_EQUAL(test_meta.size, read_meta.size);
    TEST_ASSERT_EQUAL(test_meta.version, read_meta.version);
    
    return 1;
}

/**
 * @brief Test OTA metadata pending state management
 * 
 * Verifies that OTA update pending state can be set and queried
 * correctly.
 * 
 * @return int 1 if test passes, 0 otherwise
 */
int test_ota_metadata_set_pending(void)
{
    mock_reset();
    ota_metadata_init();
    
    const uint32_t test_crc = 0xABCDEF00;
    const uint32_t test_size = 0x20000;
    const uint32_t test_version = 5;
    
    boot_error_t result = ota_metadata_set_pending(test_crc, test_size, test_version);
    
    TEST_ASSERT(result == BOOT_OK);
    
    // Verify pending flag is set
    TEST_ASSERT_EQUAL(1, ota_metadata_is_update_pending());
    
    return 1;
}

/**
 * @brief Test OTA metadata commit operation
 * 
 * Verifies that pending OTA updates can be committed to active slots
 * and pending state is cleared.
 * 
 * @return int 1 if test passes, 0 otherwise
 */
int test_ota_metadata_commit(void)
{
    mock_reset();
    ota_metadata_init();
    
    // Set pending first
    ota_metadata_set_pending(0x12345678, 0x10000, 2);
    
    // Commit to slot 1
    boot_error_t result = ota_metadata_commit(1);
    
    TEST_ASSERT(result == BOOT_OK);
    TEST_ASSERT_EQUAL(1, ota_metadata_get_active_slot());
    TEST_ASSERT_EQUAL(0, ota_metadata_is_update_pending());
    
    return 1;
}

/**
 * @brief Test OTA metadata rollback operation
 * 
 * Verifies that system can rollback to previous firmware slot
 * in case of update failure.
 * 
 * @return int 1 if test passes, 0 otherwise
 */
int test_ota_metadata_rollback(void)
{
    mock_reset();
    ota_metadata_init();
    
    // Start with slot 0
    ota_metadata_commit(0);
    TEST_ASSERT_EQUAL(0, ota_metadata_get_active_slot());
    
    // Rollback to slot 1
    boot_error_t result = ota_metadata_rollback();
    
    TEST_ASSERT(result == BOOT_OK);
    TEST_ASSERT_EQUAL(1, ota_metadata_get_active_slot());
    TEST_ASSERT_EQUAL(0, ota_metadata_is_update_pending());
    
    return 1;
}

/**
 * @brief Test OTA metadata active slot query
 * 
 * Verifies that the currently active firmware slot can be
 * queried correctly.
 * 
 * @return int 1 if test passes, 0 otherwise
 */
int test_ota_metadata_get_active_slot(void)
{
    mock_reset();
    ota_metadata_init();
    
    // Test slot 0
    ota_metadata_commit(0);
    TEST_ASSERT_EQUAL(0, ota_metadata_get_active_slot());
    
    // Test slot 1
    ota_metadata_commit(1);
    TEST_ASSERT_EQUAL(1, ota_metadata_get_active_slot());
    
    return 1;
}

/**
 * @brief Test OTA metadata pending state query
 * 
 * Verifies that update pending state can be queried correctly
 * throughout the update lifecycle.
 * 
 * @return int 1 if test passes, 0 otherwise
 */
int test_ota_metadata_is_update_pending(void)
{
    mock_reset();
    ota_metadata_init();
    
    // Initially not pending
    TEST_ASSERT_EQUAL(0, ota_metadata_is_update_pending());
    
    // Set pending
    ota_metadata_set_pending(0x12345678, 0x10000, 1);
    TEST_ASSERT_EQUAL(1, ota_metadata_is_update_pending());
    
    // Commit (clears pending)
    ota_metadata_commit(0);
    TEST_ASSERT_EQUAL(0, ota_metadata_is_update_pending());
    
    return 1;
}

// OTA metadata test suite definition
test_case_t ota_tests[] = {
    {"OTA Metadata Init", test_ota_metadata_init},
    {"OTA Metadata Read/Write", test_ota_metadata_read_write},
    {"OTA Metadata Set Pending", test_ota_metadata_set_pending},
    {"OTA Metadata Commit", test_ota_metadata_commit},
    {"OTA Metadata Rollback", test_ota_metadata_rollback},
    {"OTA Metadata Get Active Slot", test_ota_metadata_get_active_slot},
    {"OTA Metadata Is Update Pending", test_ota_metadata_is_update_pending},
};

test_suite_t ota_test_suite = {
    "OTA Metadata Tests",
    ota_tests,
    sizeof(ota_tests) / sizeof(ota_tests[0])
};
