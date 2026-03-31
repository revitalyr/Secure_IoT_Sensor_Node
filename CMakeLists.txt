cmake_minimum_required(VERSION 3.15)

# Check if we're building for tests (native) or firmware (cross-compilation)
if(BUILD_TESTS)
    # Test configuration - use native compiler
    project(Secure_IoT_Sensor_Node_Tests VERSION 1.0.0 LANGUAGES C)
    
    # Set C standard for tests
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    
    # Compiler flags for tests (MSVC compatible)
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
    
    # Include directories for tests
    include_directories(
        ${CMAKE_SOURCE_DIR}/tests
        ${CMAKE_SOURCE_DIR}/tests/mocks
        ${CMAKE_SOURCE_DIR}/config
        ${CMAKE_SOURCE_DIR}/bootloader
        ${CMAKE_SOURCE_DIR}/app
        ${CMAKE_SOURCE_DIR}/drivers
        ${CMAKE_SOURCE_DIR}/middleware
    )
    
    # Test framework sources
    set(TEST_FRAMEWORK_SOURCES
        tests/test_framework.c
        tests/test_stubs.c
        tests/mocks/freertos_mock.c
        tests/mocks/hal_mock.c
    )
    
    # Unit test sources
    set(UNIT_TEST_SOURCES
        tests/unit/test_flash.c
        tests/unit/test_ota.c
        tests/unit/test_uart.c
    )
    
    # Integration test sources
    set(INTEGRATION_TEST_SOURCES
        tests/integration/test_system.c
    )
    
    # Create test framework library
    add_library(test_framework STATIC
        ${TEST_FRAMEWORK_SOURCES}
    )
    
    # Create unit test executable
    add_executable(unit_tests
        tests/test_runner.c
        ${UNIT_TEST_SOURCES}
        ${INTEGRATION_TEST_SOURCES}
        ${TEST_FRAMEWORK_SOURCES}
    )
    
    # Create integration test executable
    add_executable(integration_tests
        tests/test_runner.c
        ${UNIT_TEST_SOURCES}
        ${INTEGRATION_TEST_SOURCES}
        ${TEST_FRAMEWORK_SOURCES}
    )
    
    # Link test framework
    target_link_libraries(unit_tests test_framework)
    target_link_libraries(integration_tests test_framework)
    
    # Enable testing
    enable_testing()
    
    # Add tests to CTest
    add_test(NAME UnitTests COMMAND unit_tests)
    add_test(NAME IntegrationTests COMMAND integration_tests)
    add_test(NAME MinimalTest COMMAND minimal_test)
    
else()
    # Firmware configuration - use ARM cross-compiler
    project(Secure_IoT_Sensor_Node VERSION 1.0.0 LANGUAGES C ASM)
    
    # Set C standard
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    
    # Set system for cross-compilation
    set(CMAKE_SYSTEM_NAME Generic)
    set(CMAKE_SYSTEM_PROCESSOR cortex-m4)
    
    # Cross-compilation settings
    set(CMAKE_C_COMPILER arm-none-eabi-gcc)
    set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
    set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
    set(CMAKE_OBJDUMP arm-none-eabi-objdump)
    set(CMAKE_SIZE arm-none-eabi-size)
    
    # MCU-specific flags
    set(MCU_FLAGS 
        -mcpu=cortex-m4 
        -mthumb 
        -mfloat-abi=hard 
        -mfpu=fpv4-sp-d16
        -DSTM32F407xx 
        -DUSE_HAL_DRIVER
    )
    
    # Compiler flags
    add_compile_options(
        ${MCU_FLAGS}
        -Wall 
        -Wextra 
        -Wno-unused-parameter
        -Os 
        -g
        -ffunction-sections 
        -fdata-sections
    )
    
    # Linker flags
    set(CMAKE_EXE_LINKER_FLAGS 
        "${MCU_FLAGS}"
        -Wl,--gc-sections
        -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map
        --specs=nano.specs 
        --specs=nosys.specs
    )
    
    # Fix Ninja generator issues
    set(CMAKE_C_LINK_EXECUTABLE 
        "<CMAKE_C_COMPILER> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
    )
    
    # Include directories
    include_directories(
        ${CMAKE_SOURCE_DIR}/config
        ${CMAKE_SOURCE_DIR}/bootloader
        ${CMAKE_SOURCE_DIR}/app
        ${CMAKE_SOURCE_DIR}/drivers
        ${CMAKE_SOURCE_DIR}/middleware
    )
    
    # Source files
    set(BOOTLOADER_SOURCES
        bootloader/bootloader.c
        bootloader/flash_if.c
        bootloader/signature.c
        bootloader/startup_stm32f4xx.c
    )
    
    set(APP_SOURCES
        app/main.c
        app/sensor_task.c
        app/comm_task.c
        app/storage_task.c
        app/update_task.c
        app/web_interface.c
        app/led_manager.c
        app/performance_monitor.c
    )
    
    set(DRIVER_SOURCES
        drivers/uart.c
        drivers/i2c.c
        drivers/gpio.c
        drivers/spi.c
        drivers/flash_if.c
    )
    
    set(MIDDLEWARE_SOURCES
        middleware/crypto/ota_metadata.c
        middleware/stm32f4xx_hal.c
    )
    
    # Create bootloader executable
    add_executable(bootloader.elf
        ${BOOTLOADER_SOURCES}
        ${DRIVER_SOURCES}
        ${MIDDLEWARE_SOURCES}
    )
    
    target_link_libraries(bootloader.elf 
        -T${CMAKE_SOURCE_DIR}/bootloader/bootloader.ld
    )
    
    # Create application executable
    add_executable(app.elf
        ${APP_SOURCES}
        ${DRIVER_SOURCES}
        ${MIDDLEWARE_SOURCES}
    )
    
    target_link_libraries(app.elf 
        -T${CMAKE_SOURCE_DIR}/app/app.ld
    )
    
    # Create binary files from ELF
    add_custom_command(
        TARGET bootloader.elf
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $< ${CMAKE_BINARY_DIR}/bootloader.bin
        COMMENT "Creating bootloader binary"
    )
    
    add_custom_command(
        TARGET app.elf
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $< ${CMAKE_BINARY_DIR}/app.bin
        COMMENT "Creating application binary"
    )
    
    # Print size information
    add_custom_command(
        TARGET bootloader.elf
        POST_BUILD
        COMMAND ${CMAKE_SIZE} $<
        COMMENT "Bootloader size information"
    )
    
    add_custom_command(
        TARGET app.elf
        POST_BUILD
        COMMAND ${CMAKE_SIZE} $<
        COMMENT "Application size information"
    )
    
    # Flash targets
    add_custom_target(flash-bootloader
        COMMAND st-flash write ${CMAKE_BINARY_DIR}/bootloader.bin 0x08000000
        DEPENDS bootloader.bin
        COMMENT "Flashing bootloader to device"
    )
    
    add_custom_target(flash-app
        COMMAND st-flash write ${CMAKE_BINARY_DIR}/app.bin 0x08010000
        DEPENDS app.bin
        COMMENT "Flashing application to device"
    )
    
    # Debug target
    add_custom_target(debug
        COMMAND st-util & 
        COMMAND arm-none-eabi-gdb ${CMAKE_BINARY_DIR}/bootloader.elf -ex "target remote localhost:4242"
        DEPENDS bootloader.elf
        COMMENT "Starting debug session"
    )
    
    # Custom targets for convenience
    add_custom_target(build-all
        DEPENDS bootloader.bin app.bin
        COMMENT "Build bootloader and application"
    )
    
    add_custom_target(clean-all
        COMMAND ${CMAKE_BUILD_TOOL} clean
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
        COMMENT "Clean all build artifacts"
    )
    
    # Installation
    install(TARGETS bootloader.elf app.elf
        RUNTIME DESTINATION bin
        OPTIONAL
    )
    
    install(FILES ${CMAKE_BINARY_DIR}/bootloader.bin ${CMAKE_BINARY_DIR}/app.bin
        RUNTIME DESTINATION bin
        OPTIONAL
    )
    
    # Package configuration
    set(CPACK_PACKAGE_NAME "Secure_IoT_Sensor_Node")
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_PACKAGE_DESCRIPTION "Secure IoT Sensor Node Firmware")
    set(CPACK_PACKAGE_VENDOR "Embedded Systems")
    set(CPACK_GENERATOR "ZIP;TGZ")
    
    include(CPack)
    
    # Set additional link flags for memory usage
    set_property(TARGET bootloader.elf APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--print-memory-usage")
    set_property(TARGET app.elf APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--print-memory-usage")
    
    # Print configuration
    message(STATUS "Firmware Configuration:")
    message(STATUS "  Target: ARM Cortex-M4")
    message(STATUS "  Toolchain: ${CMAKE_C_COMPILER}")
    message(STATUS "  MCU Flags: ${MCU_FLAGS}")
    
endif()

# Compiler-specific settings
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
endif()

# Debug configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -g3 -DDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-Os -DNDEBUG)
endif()

# Print configuration
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Toolchain: ${CMAKE_C_COMPILER}")
message(STATUS "MCU: Cortex-M4 with FPU")
