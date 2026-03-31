#!/bin/bash
# Build script for Secure IoT Sensor Node
# Handles both firmware (cross-compiled) and tests (native)

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Directories
BUILD_DIR="build"
FIRMWARE_BUILD_DIR="${BUILD_DIR}/firmware"
TEST_BUILD_DIR="${BUILD_DIR}/test"

# Functions
print_usage() {
    echo "Usage: $0 [COMMAND] [OPTIONS]"
    echo ""
    echo "Commands:"
    echo "  firmware     Build firmware (bootloader + app)"
    echo "  bootloader   Build bootloader only"
    echo "  app          Build application only"
    echo "  tests        Build and run tests"
    echo "  clean        Clean all build artifacts"
    echo "  flash        Flash firmware to device"
    echo "  debug        Start debug session"
    echo ""
    echo "Options:"
    echo "  -v, --verbose    Verbose output"
    echo "  -j, --jobs N     Number of parallel jobs"
    echo ""
    echo "Examples:"
    echo "  $0 firmware"
    echo "  $0 tests"
    echo "  $0 flash-app"
}

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check dependencies
check_dependencies() {
    local missing_deps=()
    
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi
    
    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    fi
    
    if [ "$1" = "firmware" ]; then
        if ! command -v arm-none-eabi-gcc &> /dev/null; then
            missing_deps+=("arm-none-eabi-gcc")
        fi
        if ! command -v arm-none-eabi-objcopy &> /dev/null; then
            missing_deps+=("arm-none-eabi-objcopy")
        fi
        if ! command -v st-flash &> /dev/null; then
            missing_deps+=("st-flash")
        fi
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        log_error "Missing dependencies: ${missing_deps[*]}"
        echo ""
        echo "Install missing dependencies:"
        if command -v apt-get &> /dev/null; then
            echo "  sudo apt-get install cmake make gcc-arm-none-eabi stlink-tools"
        elif command -v brew &> /dev/null; then
            echo "  brew install cmake make arm-none-eabi-gcc stlink"
        elif command -v pacman &> /dev/null; then
            echo "  sudo pacman -S cmake make arm-none-eabi-gcc stlink"
        fi
        exit 1
    fi
}

# Build firmware
build_firmware() {
    log_info "Building firmware..."
    
    mkdir -p "${FIRMWARE_BUILD_DIR}"
    cd "${FIRMWARE_BUILD_DIR}"
    
    # Configure with ARM toolchain
    cmake ../.. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE="${CMAKE_SOURCE_DIR}/cmake/arm-none-eabi.cmake"
    
    # Build
    make -j"${JOBS:-4}"
    
    cd - > /dev/null
    
    log_success "Firmware built successfully"
    log_info "Binaries available in: ${FIRMWARE_BUILD_DIR}"
}

# Build tests
build_tests() {
    log_info "Building tests..."
    
    mkdir -p "${TEST_BUILD_DIR}"
    cd "${TEST_BUILD_DIR}"
    
    # Configure for native build (tests)
    cmake ../.. -DCMAKE_BUILD_TYPE=Debug
    
    # Build
    make -j"${JOBS:-4}"
    
    cd - > /dev/null
    
    log_success "Tests built successfully"
}

# Run tests
run_tests() {
    log_info "Running tests..."
    
    cd "${TEST_BUILD_DIR}"
    ./test_runner
    
    cd - > /dev/null
}

# Flash firmware
flash_firmware() {
    local target="$1"
    
    if [ ! -f "${FIRMWARE_BUILD_DIR}/${target}.bin" ]; then
        log_error "Binary ${target}.bin not found. Build firmware first."
        exit 1
    fi
    
    log_info "Flashing ${target}.bin to device..."
    
    case "$target" in
        "bootloader")
            st-flash write "${FIRMWARE_BUILD_DIR}/bootloader.bin" 0x08000000
            ;;
        "app")
            st-flash write "${FIRMWARE_BUILD_DIR}/app.bin" 0x08010000
            ;;
        *)
            log_error "Unknown target: $target"
            exit 1
            ;;
    esac
    
    log_success "Flash completed"
}

# Debug session
start_debug() {
    if [ ! -f "${FIRMWARE_BUILD_DIR}/bootloader.elf" ]; then
        log_error "bootloader.elf not found. Build firmware first."
        exit 1
    fi
    
    log_info "Starting debug session..."
    
    # Start st-util in background
    st-util &
    ST_UTIL_PID=$!
    
    # Wait a bit for st-util to start
    sleep 2
    
    # Start GDB
    arm-none-eabi-gdb "${FIRMWARE_BUILD_DIR}/bootloader.elf" -ex "target remote localhost:4242"
    
    # Clean up st-util
    kill $ST_UTIL_PID 2>/dev/null || true
}

# Clean build artifacts
clean_build() {
    log_info "Cleaning build artifacts..."
    
    if [ -d "${BUILD_DIR}" ]; then
        rm -rf "${BUILD_DIR}"
        log_success "Build artifacts cleaned"
    else
        log_warning "No build directory found"
    fi
}

# Main script logic
main() {
    local command="$1"
    local option="$2"
    
    # Set defaults
    JOBS=4
    VERBOSE=0
    
    # Parse options
    while [[ $# -gt 0 ]]; do
        case $1 in
            -v|--verbose)
                VERBOSE=1
                shift
                ;;
            -j|--jobs)
                JOBS="$2"
                shift 2
                ;;
            *)
                shift
                ;;
        esac
    done
    
    # Set verbose flag
    if [ "$VERBOSE" -eq 1 ]; then
        set -x
    fi
    
    # Execute command
    case "$command" in
        "firmware")
            check_dependencies "firmware"
            build_firmware
            ;;
        "bootloader")
            check_dependencies "firmware"
            build_firmware
            log_info "Bootloader binary: ${FIRMWARE_BUILD_DIR}/bootloader.bin"
            ;;
        "app")
            check_dependencies "firmware"
            build_firmware
            log_info "Application binary: ${FIRMWARE_BUILD_DIR}/app.bin"
            ;;
        "tests")
            check_dependencies "tests"
            build_tests
            run_tests
            ;;
        "test")
            check_dependencies "tests"
            build_tests
            run_tests
            ;;
        "clean")
            clean_build
            ;;
        "flash-bootloader")
            check_dependencies "firmware"
            flash_firmware "bootloader"
            ;;
        "flash-app")
            check_dependencies "firmware"
            flash_firmware "app"
            ;;
        "debug")
            check_dependencies "firmware"
            start_debug
            ;;
        "help"|"-h"|"--help")
            print_usage
            ;;
        *)
            log_error "Unknown command: $command"
            print_usage
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"
