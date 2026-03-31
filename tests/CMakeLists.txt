cmake_minimum_required(VERSION 3.15)

# Test project - use native compiler for host testing
project(Secure_IoT_Sensor_Node_Tests LANGUAGES C)

# Set C standard for tests
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories for tests
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../config
    ${CMAKE_CURRENT_SOURCE_DIR}/../bootloader
    ${CMAKE_CURRENT_SOURCE_DIR}/../app
    ${CMAKE_CURRENT_SOURCE_DIR}/../drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/../middleware/freertos/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../middleware/crypto
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

# Compiler flags for tests
if(MSVC)
    add_compile_options(
        /W3 
        /Zi 
        /DTEST_BUILD
        /std:c99
    )
else()
    add_compile_options(
        -Wall 
        -Wextra 
        -g
        -DTEST_BUILD
        -std=c99
    )
endif()

# Test framework source
set(TEST_FRAMEWORK_SOURCES
    test_framework.c
    mocks/hal_mock.c
    mocks/freertos_mock.c
)

# Unit test sources
set(UNIT_TEST_SOURCES
    unit/test_uart.c
    unit/test_flash.c
    unit/test_ota.c
)

# Integration test sources
set(INTEGRATION_TEST_SOURCES
    integration/test_system.c
)

# Create test runner executable
add_executable(test_runner
    test_runner.c
    ${TEST_FRAMEWORK_SOURCES}
    ${UNIT_TEST_SOURCES}
    ${INTEGRATION_TEST_SOURCES}
)

# Link math library if needed
if(UNIX)
    target_link_libraries(test_runner m)
endif()

# Add custom target to run tests
add_custom_target(run_tests
    COMMAND test_runner
    DEPENDS test_runner
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all unit and integration tests"
)

# Add custom target to run unit tests only
add_custom_target(run_unit_tests
    COMMAND test_runner --unit-only
    DEPENDS test_runner
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running unit tests only"
)

# Add custom target to run integration tests only
add_custom_target(run_integration_tests
    COMMAND test_runner --integration-only
    DEPENDS test_runner
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running integration tests only"
)

# Add custom target for test coverage (if gcov is available)
find_program(GCOV_PATH gcov)
if(GCOV_PATH)
    add_compile_options(--coverage)
    target_link_libraries(test_runner --coverage)
    
    add_custom_target(coverage
        COMMAND lcov --capture --directory . --output-file coverage.info
        COMMAND genhtml coverage.info --output-directory coverage_html
        DEPENDS test_runner
        COMMENT "Generating test coverage report"
    )
endif()

# Test output formatting
add_custom_target(test_report
    COMMAND test_runner --xml > test_results.xml
    DEPENDS test_runner
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating test report in XML format"
)

# Performance testing
add_custom_target(performance_test
    COMMAND test_runner --performance
    DEPENDS test_runner
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running performance tests"
)

# Memory leak testing (if valgrind is available)
find_program(VALGRIND_PATH valgrind)
if(VALGRIND_PATH)
    add_custom_target(memory_test
        COMMAND ${VALGRIND_PATH} --leak-check=full --show-leak-kinds=all test_runner
        DEPENDS test_runner
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running memory leak tests"
    )
endif()

# Continuous integration support
add_custom_target(ci_test
    COMMAND test_runner --ci
    DEPENDS test_runner
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests in CI mode"
)

# Create test data directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test_data)

# Install tests (optional)
install(TARGETS test_runner
    RUNTIME DESTINATION bin
    OPTIONAL
)

# Print test configuration
message(STATUS "Test Configuration:")
message(STATUS "  Test Framework: Custom")
message(STATUS "  Mock Framework: HAL Mock")
message(STATUS "  Unit Tests: ${CMAKE_CURRENT_LIST_DIR}/unit")
message(STATUS "  Integration Tests: ${CMAKE_CURRENT_LIST_DIR}/integration")
if(GCOV_PATH)
    message(STATUS "  Coverage: Enabled")
else()
    message(STATUS "  Coverage: Disabled (gcov not found)")
endif()
if(VALGRIND_PATH)
    message(STATUS "  Memory Testing: Enabled")
else()
    message(STATUS "  Memory Testing: Disabled (valgrind not found)")
endif()
