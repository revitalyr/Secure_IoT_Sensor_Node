# Test Suite for Secure IoT Sensor Node

This directory contains comprehensive unit and integration tests for the Secure IoT Sensor Node project.

## 🧪 Test Structure

```
tests/
├── test_framework.h/c     # Custom test framework
├── mocks/                # Hardware and RTOS mocks
│   ├── hal_mock.h/c      # STM32 HAL mock
│   └── freertos_mock.h/c # FreeRTOS mock
├── unit/                 # Unit tests
│   ├── test_uart.c       # UART driver tests
│   ├── test_flash.c      # Flash storage tests
│   └── test_ota.c       # OTA metadata tests
├── integration/          # Integration tests
│   └── test_system.c    # System integration tests
├── test_runner.c        # Main test executable
├── CMakeLists.txt       # Test build configuration
└── README.md           # This file
```

## 🚀 Building and Running Tests

### Prerequisites

- GCC (for host testing)
- CMake 3.15+
- Optional: gcov (for coverage)
- Optional: valgrind (for memory testing)

### Build Commands

```bash
# Configure build
mkdir build
cd build
cmake ..

# Build tests
make test_runner

# Run all tests
make run_tests

# Run only unit tests
make run_unit_tests

# Run only integration tests
make run_integration_tests

# Generate test coverage report (requires gcov)
make coverage

# Run memory leak tests (requires valgrind)
make memory_test

# Generate XML test report
make test_report

# Run performance tests
make performance_test

# Run tests in CI mode
make ci_test
```

## 📊 Test Categories

### Unit Tests

#### UART Driver Tests
- **UART Initialization** - Verify proper driver setup
- **UART Write** - Test data transmission
- **UART Write String** - Test string transmission
- **UART Read** - Test data reception
- **UART Buffer Overflow** - Test error handling

#### Flash Storage Tests
- **Flash Storage Init** - Verify initialization
- **Flash Storage Write** - Test data programming
- **Flash Storage Read** - Test data retrieval
- **Flash Storage Erase** - Test sector erasure
- **Wear Level Init** - Verify wear leveling setup
- **Wear Level Write** - Test wear-leveling writes
- **Wear Level Read** - Test wear-leveling reads
- **Wear Level Write Count** - Test write count tracking

#### OTA Metadata Tests
- **OTA Metadata Init** - Verify metadata initialization
- **OTA Metadata Read/Write** - Test persistence
- **OTA Metadata Set Pending** - Test update pending flag
- **OTA Metadata Commit** - Test update commitment
- **OTA Metadata Rollback** - Test rollback functionality
- **OTA Metadata Get Active Slot** - Test slot management
- **OTA Metadata Is Update Pending** - Test pending status

### Integration Tests

#### System Integration Tests
- **Sensor Data Flow** - Test end-to-end sensor data pipeline
- **UART Communication Protocol** - Test binary protocol implementation
- **Flash Storage Integration** - Test sensor data storage
- **OTA Update Flow** - Test complete OTA update process
- **System Error Handling** - Test error scenarios
- **Concurrent Tasks** - Test RTOS task coordination
- **Memory Management** - Test dynamic memory operations

## 🔧 Test Framework

### Custom Test Framework

The project uses a lightweight custom test framework with the following features:

- **Simple assertions** - TEST_ASSERT, TEST_ASSERT_EQUAL, etc.
- **Test suites** - Organized test collections
- **Mock integration** - Hardware and RTOS mocking
- **Detailed reporting** - Pass/fail statistics
- **CI support** - XML output format

### Mock Framework

#### HAL Mock
- **GPIO** - Pin state simulation
- **UART** - Buffer-based communication
- **I2C** - Transaction recording
- **SPI** - Data transfer simulation
- **Flash** - Memory-based flash simulation
- **System** - Tick and delay simulation

#### FreeRTOS Mock
- **Queues** - In-memory queue implementation
- **Semaphores** - Simple mutex simulation
- **Tasks** - Mock task creation
- **Notifications** - Simplified notification system

## 📈 Test Coverage

### Coverage Areas

- **Driver Layer** - UART, SPI, I2C, GPIO, Flash
- **Middleware Layer** - OTA metadata, wear leveling
- **Application Layer** - Task coordination, data flow
- **System Integration** - End-to-end scenarios
- **Error Handling** - Failure scenarios and recovery

### Coverage Reports

Generate coverage reports with:
```bash
make coverage
```

View HTML coverage report at:
```
build/coverage_html/index.html
```

## 🐛 Debugging Tests

### Running Individual Tests

```bash
# Build and run specific test
./test_runner --filter "UART Write"

# Run with verbose output
./test_runner --verbose

# Run with debug output
./test_runner --debug
```

### Memory Testing

```bash
# Check for memory leaks
make memory_test

# Valgrind output analysis
valgrind --leak-check=full --show-leak-kinds=all ./test_runner
```

## 📝 Adding New Tests

### Unit Test Template

```c
#include "../test_framework.h"
#include "../mocks/hal_mock.h"
#include "../drivers/your_driver.h"

int test_your_functionality(void)
{
    mock_reset();
    
    // Setup test conditions
    // Call function under test
    // Verify results
    
    TEST_ASSERT(condition);
    return 1;
}

test_case_t your_driver_tests[] = {
    {"Your Test Name", test_your_functionality},
    // Add more tests...
};

test_suite_t your_driver_test_suite = {
    "Your Driver Tests",
    your_driver_tests,
    sizeof(your_driver_tests) / sizeof(your_driver_tests[0])
};
```

### Integration Test Template

```c
#include "../test_framework.h"
#include "../mocks/hal_mock.h"
#include "../app/main.h"

int test_integration_scenario(void)
{
    mock_reset();
    
    // Initialize system components
    system_init();
    
    // Simulate integration scenario
    // Verify end-to-end behavior
    
    TEST_ASSERT(condition);
    return 1;
}

// Add to integration_tests array in test_system.c
```

## 🚨 Continuous Integration

### CI Configuration

The test suite supports CI environments with:

- **Non-interactive mode** - No prompts or delays
- **XML output** - JUnit-compatible test results
- **Exit codes** - Proper success/failure status
- **Parallel execution** - Fast test execution

### CI Commands

```bash
# Run in CI mode
make ci_test

# Generate CI report
make test_report
```

## 📊 Test Metrics

### Success Criteria

- **All unit tests pass** - Component-level verification
- **All integration tests pass** - System-level verification
- **No memory leaks** - Clean memory management
- **Coverage > 80%** - Comprehensive test coverage

### Performance Benchmarks

- **Test execution time** - < 5 seconds total
- **Memory usage** - < 10MB peak usage
- **Mock accuracy** - 100% HAL behavior simulation

## 🔗 Related Documentation

- [Main README](../README.md) - Project overview
- [Architecture](../docs/architecture.md) - System architecture
- [Hardware](../docs/hardware.md) - Hardware specifications

---

**Test Philosophy**: Comprehensive testing ensures reliable embedded firmware through unit isolation, integration verification, and continuous validation.
