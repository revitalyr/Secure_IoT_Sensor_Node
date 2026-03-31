cmake_minimum_required(VERSION 3.15)
project(Secure_IoT_Sensor_Node VERSION 1.0.0 LANGUAGES C ASM)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Toolchain configuration for ARM Cortex-M4
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

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/bootloader
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/drivers
    ${CMAKE_SOURCE_DIR}/middleware/freertos/include
    ${CMAKE_SOURCE_DIR}/middleware/crypto
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
)

# FreeRTOS sources (simplified for demo)
set(FREERTOS_SOURCES
    middleware/freertos/port.c
    middleware/freertos/list.c
    middleware/freertos/queue.c
    middleware/freertos/tasks.c
)

# HAL sources (simplified for demo)
set(HAL_SOURCES
    middleware/stm32f4xx_hal.c
    middleware/stm32f4xx_hal_uart.c
    middleware/stm32f4xx_hal_i2c.c
    middleware/stm32f4xx_hal_spi.c
    middleware/stm32f4xx_hal_flash.c
)

# Create bootloader executable
add_executable(bootloader.elf
    ${BOOTLOADER_SOURCES}
    ${DRIVER_SOURCES}
    ${HAL_SOURCES}
)

target_link_libraries(bootloader.elf 
    -T${CMAKE_SOURCE_DIR}/bootloader/bootloader.ld
)

# Create application executable
add_executable(app.elf
    ${APP_SOURCES}
    ${DRIVER_SOURCES}
    ${MIDDLEWARE_SOURCES}
    ${FREERTOS_SOURCES}
    ${HAL_SOURCES}
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

# Create directory structure
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR})

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

# Include tests
add_subdirectory(tests)

# Installation
install(TARGETS bootloader.bin app.bin
    DESTINATION bin
    OPTIONAL
)

# Package configuration
set(CPACK_PACKAGE_NAME "Secure_IoT_Sensor_Node")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION "Secure IoT Sensor Node Firmware")
set(CPACK_PACKAGE_VENDOR "Embedded Systems")
set(CPACK_GENERATOR "ZIP;TGZ")

include(CPack)

# Build configuration
set_property(TARGET bootloader.elf APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--print-memory-usage")
set_property(TARGET app.elf APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--print-memory-usage")

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
