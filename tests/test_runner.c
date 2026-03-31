#include "test_framework.h"
#include "unit/test_uart.c"
#include "unit/test_flash.c"
#include "unit/test_ota.c"
#include "integration/test_system.c"

// Mock FreeRTOS functions for testing
void vTaskDelay(const TickType_t xTicksToDelay) {
    HAL_Delay(xTicksToDelay);
}

BaseType_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize) {
    return pdTRUE; // Mock success
}

BaseType_t xQueueSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait) {
    return pdTRUE; // Mock success
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait) {
    return pdTRUE; // Mock success
}

void* xSemaphoreCreateMutex(void) {
    return (void*)1; // Mock valid handle
}

BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait) {
    return pdTRUE; // Mock success
}

BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore) {
    return pdTRUE; // Mock success
}

int main(void)
{
    printf("=== Secure IoT Sensor Node Test Suite ===\n\n");
    
    mock_init();
    
    int all_passed = 1;
    
    // Run unit tests
    all_passed &= run_test_suite(&uart_test_suite);
    all_passed &= run_test_suite(&flash_test_suite);
    all_passed &= run_test_suite(&ota_test_suite);
    
    // Run integration tests
    all_passed &= run_test_suite(&integration_test_suite);
    
    // Print final summary
    print_test_summary();
    
    if (all_passed) {
        printf("\n🎉 All tests passed!\n");
        return 0;
    } else {
        printf("\n❌ Some tests failed!\n");
        return 1;
    }
}
