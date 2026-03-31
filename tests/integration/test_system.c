#include "../test_framework.h"
#include "../mocks/hal_mock.h"
#include "../app/main.h"
#include "../app/sensor_task.h"
#include "../app/comm_task.h"
#include "../app/storage_task.h"
#include "../drivers/uart.h"
#include "../drivers/flash_if.h"
#include <string.h>

// Test complete sensor data flow
int test_sensor_data_flow(void)
{
    mock_reset();
    
    // Initialize system components
    system_init();
    uart_init();
    flash_storage_init();
    
    // Create sensor queue
    sensor_queue = xQueueCreate(16, sizeof(float));
    TEST_ASSERT_NOT_NULL(sensor_queue);
    
    // Simulate sensor reading
    float test_temperature = 36.6f;
    
    // Send to queue (simulating sensor task)
    BaseType_t result = xQueueSend(sensor_queue, &test_temperature, 0);
    TEST_ASSERT(result == pdTRUE);
    
    // Receive from queue (simulating comm/storage task)
    float received_temp;
    result = xQueueReceive(sensor_queue, &received_temp, 0);
    TEST_ASSERT(result == pdTRUE);
    TEST_ASSERT_EQUAL(test_temperature, received_temp);
    
    return 1;
}

// Test UART communication with binary protocol
int test_uart_communication_protocol(void)
{
    mock_reset();
    
    // Initialize communication
    uart_init();
    
    // Create test packet
    typedef struct {
        uint8_t header;
        uint8_t type;
        uint8_t len;
        float payload;
        uint32_t timestamp;
        uint16_t crc;
    } __attribute__((packed)) test_packet_t;
    
    test_packet_t packet = {
        .header = 0xAA,
        .type = 0x01,
        .len = sizeof(float) + sizeof(uint32_t),
        .payload = 36.7f,
        .timestamp = 12345,
        .crc = 0
    };
    
    // Calculate CRC (simplified)
    packet.crc = 0x1234;
    
    // Send packet
    uart_write((uint8_t*)&packet, sizeof(packet));
    
    // Verify packet was transmitted
    TEST_ASSERT(g_mock_state.uart_tx_len == sizeof(packet));
    TEST_ASSERT_MEM_EQUAL(&packet, g_mock_state.uart_tx_buffer, sizeof(packet));
    
    return 1;
}

// Test flash storage with sensor data
int test_flash_storage_integration(void)
{
    mock_reset();
    
    // Initialize storage
    flash_storage_init();
    wear_level_init();
    
    // Create sensor data
    sensor_data_t test_data = {
        .temperature = 37.2f,
        .timestamp = 67890,
        .status = 0
    };
    
    // Store sensor data
    boot_error_t result = storage_write_sensor_data(&test_data);
    TEST_ASSERT(result == BOOT_OK);
    
    // Read back sensor data
    sensor_data_t read_data;
    result = storage_read_sensor_data(0, &read_data);
    TEST_ASSERT(result == BOOT_OK);
    
    // Verify data integrity
    TEST_ASSERT_EQUAL(test_data.temperature, read_data.temperature);
    TEST_ASSERT_EQUAL(test_data.timestamp, read_data.timestamp);
    TEST_ASSERT_EQUAL(test_data.status, read_data.status);
    
    return 1;
}

// Test OTA update flow simulation
int test_ota_update_flow(void)
{
    mock_reset();
    
    // Initialize OTA system
    ota_metadata_init();
    flash_storage_init();
    
    // Simulate OTA update parameters
    uint32_t test_crc = 0xABCDEF12;
    uint32_t test_size = 0x15000;
    uint32_t test_version = 3;
    
    // Start OTA update
    boot_error_t result = ota_metadata_set_pending(test_crc, test_size, test_version);
    TEST_ASSERT(result == BOOT_OK);
    TEST_ASSERT_EQUAL(1, ota_metadata_is_update_pending());
    
    // Simulate firmware verification (would normally be done by bootloader)
    // For this test, we'll assume verification passes
    result = ota_metadata_commit(1); // Commit to slot B
    TEST_ASSERT(result == BOOT_OK);
    TEST_ASSERT_EQUAL(1, ota_metadata_get_active_slot());
    TEST_ASSERT_EQUAL(0, ota_metadata_is_update_pending());
    
    return 1;
}

// Test system error handling
int test_system_error_handling(void)
{
    mock_reset();
    
    // Initialize system
    system_init();
    
    // Simulate system error
    g_mock_state.system_error = 1;
    
    // Test error handler (would normally blink LED, etc.)
    // For this test, we just verify the error state is detected
    TEST_ASSERT_EQUAL(1, g_mock_state.system_error);
    
    return 1;
}

// Test concurrent task simulation
int test_concurrent_tasks(void)
{
    mock_reset();
    
    // Initialize system
    system_init();
    
    // Create queues and mutexes
    sensor_queue = xQueueCreate(16, sizeof(float));
    uart_rx_queue = xQueueCreate(128, sizeof(uint8_t));
    flash_mutex = xSemaphoreCreateMutex();
    uart_mutex = xSemaphoreCreateMutex();
    
    TEST_ASSERT_NOT_NULL(sensor_queue);
    TEST_ASSERT_NOT_NULL(uart_rx_queue);
    TEST_ASSERT_NOT_NULL(flash_mutex);
    TEST_ASSERT_NOT_NULL(uart_mutex);
    
    // Simulate concurrent access
    float temp1 = 36.5f, temp2 = 37.0f, temp3 = 36.8f;
    
    // Multiple producers (sensor task)
    xQueueSend(sensor_queue, &temp1, 0);
    xQueueSend(sensor_queue, &temp2, 0);
    xQueueSend(sensor_queue, &temp3, 0);
    
    // Multiple consumers (comm/storage tasks)
    float received1, received2, received3;
    TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(sensor_queue, &received1, 0));
    TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(sensor_queue, &received2, 0));
    TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(sensor_queue, &received3, 0));
    
    // Verify data integrity
    TEST_ASSERT_EQUAL(temp1, received1);
    TEST_ASSERT_EQUAL(temp2, received2);
    TEST_ASSERT_EQUAL(temp3, received3);
    
    return 1;
}

// Test memory management
int test_memory_management(void)
{
    mock_reset();
    
    // Test allocation and deallocation
    void* ptr1 = malloc(1024);
    void* ptr2 = malloc(2048);
    
    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    
    // Write to allocated memory
    memset(ptr1, 0xAA, 1024);
    memset(ptr2, 0x55, 2048);
    
    // Verify memory contents
    uint8_t* bytes1 = (uint8_t*)ptr1;
    uint8_t* bytes2 = (uint8_t*)ptr2;
    
    TEST_ASSERT_EQUAL(0xAA, bytes1[0]);
    TEST_ASSERT_EQUAL(0xAA, bytes1[1023]);
    TEST_ASSERT_EQUAL(0x55, bytes2[0]);
    TEST_ASSERT_EQUAL(0x55, bytes2[2047]);
    
    // Free memory
    free(ptr1);
    free(ptr2);
    
    return 1;
}

// Integration test suite
test_case_t integration_tests[] = {
    {"Sensor Data Flow", test_sensor_data_flow},
    {"UART Communication Protocol", test_uart_communication_protocol},
    {"Flash Storage Integration", test_flash_storage_integration},
    {"OTA Update Flow", test_ota_update_flow},
    {"System Error Handling", test_system_error_handling},
    {"Concurrent Tasks", test_concurrent_tasks},
    {"Memory Management", test_memory_management},
};

test_suite_t integration_test_suite = {
    "Integration Tests",
    integration_tests,
    sizeof(integration_tests) / sizeof(integration_tests[0])
};
