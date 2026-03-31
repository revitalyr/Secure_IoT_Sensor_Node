@echo off
REM Build script for Secure IoT Sensor Node (Windows)
REM Handles both firmware (cross-compiled) and tests (native)

setlocal enabledelayedexpansion

REM Default values
set JOBS=4
set VERBOSE=0
set COMMAND=%1
set OPTION=%2

REM Colors (limited support in Windows CMD)
set INFO=[INFO]
set SUCCESS=[SUCCESS]
set WARNING=[WARNING]
set ERROR=[ERROR]

REM Functions
:print_usage
echo Usage: %~nx0 [COMMAND] [OPTIONS]
echo.
echo Commands:
echo   firmware     Build firmware ^(bootloader + app^)
echo   bootloader   Build bootloader only
echo   app          Build application only
echo   tests        Build and run tests
echo   clean        Clean all build artifacts
echo   flash        Flash firmware to device
echo   debug        Start debug session
echo.
echo Options:
echo   -v, --verbose    Verbose output
echo   -j, --jobs N     Number of parallel jobs
echo.
echo Examples:
echo   %~nx0 firmware
echo   %~nx0 tests
echo   %~nx0 flash-app
goto :eof

:log_info
echo %INFO% %~1
goto :eof

:log_success
echo %SUCCESS% %~1
goto :eof

:log_warning
echo %WARNING% %~1
goto :eof

:log_error
echo %ERROR% %~1
goto :eof

:check_dependencies_firmware
where arm-none-eabi-gcc >nul 2>&1
if errorlevel 1 (
    %log_error% arm-none-eabi-gcc not found in PATH
    echo Install ARM GNU Toolchain: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm
    exit /b 1
)

where arm-none-eabi-objcopy >nul 2>&1
if errorlevel 1 (
    %log_error% arm-none-eabi-objcopy not found in PATH
    exit /b 1
)

where st-flash >nul 2>&1
if errorlevel 1 (
    %log_warning% st-flash not found. Install STM32 ST-LINK utilities
)

where cmake >nul 2>&1
if errorlevel 1 (
    %log_error% cmake not found in PATH
    exit /b 1
)

where make >nul 2>&1
if errorlevel 1 (
    %log_error% make not found in PATH
    exit /b 1
)
goto :eof

:check_dependencies_tests
where cmake >nul 2>&1
if errorlevel 1 (
    %log_error% cmake not found in PATH
    exit /b 1
)

where make >nul 2>&1
if errorlevel 1 (
    %log_error% make not found in PATH
    exit /b 1
)
goto :eof

:build_firmware
%log_info% Building firmware...

if not exist build\firmware mkdir build\firmware
cd build\firmware

REM Configure with ARM toolchain
cmake ..\.. ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_TOOLCHAIN_FILE=..\..\cmake\arm-none-eabi.cmake

if errorlevel 1 (
    %log_error% CMake configuration failed
    cd ..\..
    exit /b 1
)

REM Build
make -j%JOBS%

if errorlevel 1 (
    %log_error% Build failed
    cd ..\..
    exit /b 1
)

cd ..\..
%log_success% Firmware built successfully
%log_info% Binaries available in: build\firmware
goto :eof

:build_tests
%log_info% Building tests...

if not exist build\test mkdir build\test
cd build\test

REM Configure for native build (tests)
cmake ..\.. -DCMAKE_BUILD_TYPE=Debug

if errorlevel 1 (
    %log_error% CMake configuration failed
    cd ..\..
    exit /b 1
)

REM Build
make -j%JOBS%

if errorlevel 1 (
    %log_error% Test build failed
    cd ..\..
    exit /b 1
)

cd ..\..
%log_success% Tests built successfully
goto :eof

:run_tests
%log_info% Running tests...

cd build\test
test_runner.exe

if errorlevel 1 (
    %log_error% Some tests failed
    cd ..\..
    exit /b 1
)

cd ..\..
%log_success% All tests passed
goto :eof

:flash_firmware
set TARGET=%1
if not exist build\firmware\%TARGET%.bin (
    %log_error% Binary %TARGET%.bin not found. Build firmware first.
    exit /b 1
)

%log_info% Flashing %TARGET%.bin to device...

if "%TARGET%"=="bootloader" (
    st-flash write build\firmware\bootloader.bin 0x08000000
) else if "%TARGET%"=="app" (
    st-flash write build\firmware\app.bin 0x08010000
) else (
    %log_error% Unknown target: %TARGET%
    exit /b 1
)

if errorlevel 1 (
    %log_error% Flash failed
    exit /b 1
)

%log_success% Flash completed
goto :eof

:start_debug
if not exist build\firmware\bootloader.elf (
    %log_error% bootloader.elf not found. Build firmware first.
    exit /b 1
)

%log_info% Starting debug session...

REM Start st-util in background
start /b st-util

REM Wait a bit for st-util to start
timeout /t 2 /nobreak >nul

REM Start GDB
arm-none-eabi-gdb build\firmware\bootloader.elf -ex "target remote localhost:4242"
goto :eof

:clean_build
%log_info% Cleaning build artifacts...

if exist build (
    rmdir /s /q build
    %log_success% Build artifacts cleaned
) else (
    %log_warning% No build directory found
)
goto :eof

:main
REM Parse command
if "%COMMAND%"=="firmware" (
    call :check_dependencies_firmware
    call :build_firmware
) else if "%COMMAND%"=="bootloader" (
    call :check_dependencies_firmware
    call :build_firmware
    %log_info% Bootloader binary: build\firmware\bootloader.bin
) else if "%COMMAND%"=="app" (
    call :check_dependencies_firmware
    call :build_firmware
    %log_info% Application binary: build\firmware\app.bin
) else if "%COMMAND%"=="tests" (
    call :check_dependencies_tests
    call :build_tests
    call :run_tests
) else if "%COMMAND%"=="test" (
    call :check_dependencies_tests
    call :build_tests
    call :run_tests
) else if "%COMMAND%"=="clean" (
    call :clean_build
) else if "%COMMAND%"=="flash-bootloader" (
    call :check_dependencies_firmware
    call :flash_firmware bootloader
) else if "%COMMAND%"=="flash-app" (
    call :check_dependencies_firmware
    call :flash_firmware app
) else if "%COMMAND%"=="debug" (
    call :check_dependencies_firmware
    call :start_debug
) else if "%COMMAND%"=="help" (
    call :print_usage
) else if "%COMMAND%"=="-h" (
    call :print_usage
) else if "%COMMAND%"=="--help" (
    call :print_usage
) else (
    %log_error% Unknown command: %COMMAND%
    call :print_usage
    exit /b 1
)

:eof
