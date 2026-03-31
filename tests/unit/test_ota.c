#include "../test_framework.h"
#include "../mocks/hal_mock.h"
#include "../middleware/crypto/ota_metadata.h"
#include <string.h>

// Test OTA metadata initialization
int test_ota_metadata_init(void)
{
    mock_reset();
    
    boot_error_t result = ota_metadata_init();
    
    TEST_ASSERT(result == BOOT_OK);
    
    return 1;
}

// Test OTA metadata read/write
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
    
    // Verify data
    TEST_ASSERT_EQUAL(test_meta.active_slot, read_meta.active_slot);
    TEST_ASSERT_EQUAL(test_meta.update_pending, read_meta.update_pending);
    TEST_ASSERT_EQUAL(test_meta.crc, read_meta.crc);
    TEST_ASSERT_EQUAL(test_meta.size, read_meta.size);
    TEST_ASSERT_EQUAL(test_meta.version, read_meta.version);
    
    return 1;
}

// Test OTA metadata set pending
int test_ota_metadata_set_pending(void)
{
    mock_reset();
    ota_metadata_init();
    
    uint32_t test_crc = 0xABCDEF00;
    uint32_t test_size = 0x20000;
    uint32_t test_version = 5;
    
    boot_error_t result = ota_metadata_set_pending(test_crc, test_size, test_version);
    
    TEST_ASSERT(result == BOOT_OK);
    
    // Verify pending flag is set
    TEST_ASSERT_EQUAL(1, ota_metadata_is_update_pending());
    
    return 1;
}

// Test OTA metadata commit
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

// Test OTA metadata rollback
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

// Test OTA metadata get active slot
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

// Test OTA metadata is update pending
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

// OTA metadata test suite
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
