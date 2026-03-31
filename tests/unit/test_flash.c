#include "../test_framework.h"
#include "../mocks/hal_mock.h"
#include "../drivers/flash_if.h"
#include <string.h>

// Test flash storage initialization
int test_flash_storage_init(void)
{
    mock_reset();
    
    boot_error_t result = flash_storage_init();
    
    TEST_ASSERT(result == BOOT_OK);
    
    return 1;
}

// Test flash storage write
int test_flash_storage_write(void)
{
    mock_reset();
    flash_storage_init();
    
    uint8_t test_data[] = "Flash Test Data";
    uint32_t addr = STORAGE_FLASH_BASE;
    uint32_t len = sizeof(test_data) - 1;
    
    boot_error_t result = flash_storage_write(addr, test_data, len);
    
    TEST_ASSERT(result == BOOT_OK);
    
    // Verify data was written to mock flash
    uint8_t read_back[32];
    mock_flash_read(addr, read_back, len);
    TEST_ASSERT_MEM_EQUAL(test_data, read_back, len);
    
    return 1;
}

// Test flash storage read
int test_flash_storage_read(void)
{
    mock_reset();
    flash_storage_init();
    
    uint8_t test_data[] = "Read Test Data";
    uint32_t addr = STORAGE_FLASH_BASE;
    uint32_t len = sizeof(test_data) - 1;
    
    // Pre-write data to mock flash
    mock_flash_write(addr, test_data, len);
    
    uint8_t read_buffer[32];
    boot_error_t result = flash_storage_read(addr, read_buffer, len);
    
    TEST_ASSERT(result == BOOT_OK);
    TEST_ASSERT_MEM_EQUAL(test_data, read_buffer, len);
    
    return 1;
}

// Test flash storage erase
int test_flash_storage_erase(void)
{
    mock_reset();
    flash_storage_init();
    
    // Write some data first
    uint8_t test_data[] = "Erase Test";
    uint32_t addr = STORAGE_FLASH_BASE;
    mock_flash_write(addr, test_data, sizeof(test_data));
    
    // Erase sector
    boot_error_t result = flash_storage_erase_sector(STORAGE_SECTOR_START);
    
    TEST_ASSERT(result == BOOT_OK);
    
    // Verify data is erased (should be 0xFF)
    uint8_t read_back[32];
    mock_flash_read(addr, read_back, sizeof(test_data));
    
    uint8_t erased_pattern[32];
    memset(erased_pattern, 0xFF, sizeof(erased_pattern));
    TEST_ASSERT_MEM_EQUAL(erased_pattern, read_back, sizeof(test_data));
    
    return 1;
}

// Test wear leveling initialization
int test_wear_level_init(void)
{
    mock_reset();
    
    boot_error_t result = wear_level_init();
    
    TEST_ASSERT(result == BOOT_OK);
    
    return 1;
}

// Test wear leveling write
int test_wear_level_write(void)
{
    mock_reset();
    wear_level_init();
    
    uint8_t test_data[] = "Wear Level Test";
    uint32_t len = sizeof(test_data) - 1;
    
    boot_error_t result = wear_level_write(test_data, len);
    
    TEST_ASSERT(result == BOOT_OK);
    
    return 1;
}

// Test wear leveling read
int test_wear_level_read(void)
{
    mock_reset();
    wear_level_init();
    
    uint8_t test_data[] = "Wear Read Test";
    uint32_t len = sizeof(test_data) - 1;
    
    // Write first
    wear_level_write(test_data, len);
    
    // Then read back
    uint8_t read_buffer[32];
    boot_error_t result = wear_level_read(0, read_buffer, len);
    
    TEST_ASSERT(result == BOOT_OK);
    TEST_ASSERT_MEM_EQUAL(test_data, read_buffer, len);
    
    return 1;
}

// Test wear leveling write count tracking
int test_wear_level_write_count(void)
{
    mock_reset();
    wear_level_init();
    
    uint8_t test_data[] = "Count Test";
    uint32_t len = sizeof(test_data) - 1;
    
    // Write multiple times to trigger wear leveling
    for (int i = 0; i < 10; i++) {
        wear_level_write(test_data, len);
    }
    
    uint32_t write_count = wear_level_get_write_count(0);
    TEST_ASSERT(write_count > 0);
    
    return 1;
}

// Flash storage test suite
test_case_t flash_tests[] = {
    {"Flash Storage Init", test_flash_storage_init},
    {"Flash Storage Write", test_flash_storage_write},
    {"Flash Storage Read", test_flash_storage_read},
    {"Flash Storage Erase", test_flash_storage_erase},
    {"Wear Level Init", test_wear_level_init},
    {"Wear Level Write", test_wear_level_write},
    {"Wear Level Read", test_wear_level_read},
    {"Wear Level Write Count", test_wear_level_write_count},
};

test_suite_t flash_test_suite = {
    "Flash Storage Tests",
    flash_tests,
    sizeof(flash_tests) / sizeof(flash_tests[0])
};
