#include "test_framework.h"
#include "mocks/freertos_mock.h"
#include "mocks/hal_mock.h"
#include "unit/test_uart.h"
#include "unit/test_flash.h"
#include "unit/test_ota.h"
#include "integration/test_system.h"

// Define config for FreeRTOS mock
#define configTICK_RATE_HZ 1000

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
