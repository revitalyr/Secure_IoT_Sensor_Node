# Build System for Secure IoT Sensor Node

This project uses CMake for building firmware and tests, with custom build scripts for convenience.

## 🛠️ Prerequisites

### For Firmware Build (Cross-compilation)
- **ARM GNU Toolchain**: `arm-none-eabi-gcc`, `arm-none-eabi-objcopy`
- **CMake**: Version 3.15 or higher
- **Make**: GNU Make
- **ST-LINK Tools**: `st-flash` for programming
- **Optional**: `st-util` for debugging

### For Tests (Native compilation)
- **Native C Compiler**: `gcc` or `clang`
- **CMake**: Version 3.15 or higher
- **Make**: GNU Make
- **Optional**: `gcov` for coverage
- **Optional**: `valgrind` for memory testing

## 🚀 Quick Start

### Windows
```batch
# Build firmware
build.bat firmware

# Build and run tests
build.bat tests

# Flash bootloader
build.bat flash-bootloader

# Flash application
build.bat flash-app
```

### Linux/macOS
```bash
# Make build script executable
chmod +x build.sh

# Build firmware
./build.sh firmware

# Build and run tests
./build.sh tests

# Flash bootloader
./build.sh flash-bootloader

# Flash application
./build.sh flash-app
```

## 📁 Build Structure

```
build/
├── firmware/          # Cross-compiled firmware
│   ├── bootloader.bin
│   ├── bootloader.elf
│   ├── app.bin
│   └── app.elf
└── test/              # Native test executable
    ├── test_runner
    └── test_results.xml
```

## 🔧 Manual CMake Usage

### Firmware Build
```bash
# Create build directory
mkdir -p build/firmware
cd build/firmware

# Configure with ARM toolchain
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi.cmake

# Build
make -j4

# Flash
make flash-bootloader  # or flash-app
```

### Test Build
```bash
# Create test build directory
mkdir -p build/test
cd build/test

# Configure for native build
cmake ../.. -DCMAKE_BUILD_TYPE=Debug

# Build tests
make -j4

# Run tests
make run_tests
```

## 📊 Build Targets

### Firmware Targets
- **bootloader.elf** - Bootloader executable
- **bootloader.bin** - Bootloader binary for flashing
- **app.elf** - Application executable
- **app.bin** - Application binary for flashing

### Test Targets
- **test_runner** - Test executable
- **run_tests** - Run all tests
- **run_unit_tests** - Run unit tests only
- **run_integration_tests** - Run integration tests only
- **coverage** - Generate coverage report (requires gcov)
- **memory_test** - Run memory leak tests (requires valgrind)

## 🔍 Debugging

### GDB Debug Session
```bash
# Start ST-LINK server
st-util &

# Connect GDB
arm-none-eabi-gdb build/firmware/bootloader.elf
(gdb) target remote localhost:4242
(gdb) load
(gdb) continue
```

### STM32CubeIDE Integration
1. Import project as "Existing Code"
2. Set build configuration to Release
3. Use provided linker scripts
4. Configure debugger for ST-LINK

## 📦 Installation

### ARM Toolchain Installation

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install gcc-arm-none-eabi make cmake stlink-tools
```

#### macOS (Homebrew)
```bash
brew install arm-none-eabi-gcc make cmake stlink
```

#### Windows
1. Download ARM GNU Toolchain from ARM website
2. Add to PATH
3. Install STM32 ST-LINK utilities
4. Install CMake and Make

### Verification
```bash
# Check toolchain
arm-none-eabi-gcc --version

# Check CMake
cmake --version

# Check ST-LINK
st-flash --version
```

## ⚙️ Configuration Options

### CMake Options
- `CMAKE_BUILD_TYPE`: Debug/Release
- `CMAKE_TOOLCHAIN_FILE`: Path to ARM toolchain file
- `MCU_FLAGS`: Additional compiler flags

### Build Script Options
- `-v, --verbose`: Verbose output
- `-j N, --jobs N`: Number of parallel jobs

## 🐛 Troubleshooting

### Common Issues

#### "arm-none-eabi-gcc not found"
**Solution**: Install ARM GNU Toolchain and add to PATH

#### "st-flash command not found"
**Solution**: Install STM32 ST-LINK utilities

#### "CMAKE_C_COMPILER not found"
**Solution**: Use provided build scripts or specify toolchain file

#### "Permission denied" (Linux)
**Solution**: Add user to dialout group for USB access
```bash
sudo usermod -a -G dialout $USER
# Logout and login again
```

#### "ST-LINK not detected"
**Solution**: 
- Check USB connection
- Install ST-LINK drivers
- Try different USB port
- Reset ST-LINK device

### Build Failures

#### Clean Build
```bash
# Remove all build artifacts
./build.sh clean
# or
rm -rf build/
```

#### Verbose Output
```bash
# Enable verbose build
./build.sh firmware -v
# or
make VERBOSE=1
```

## 📈 Performance

### Build Times
- **Firmware**: ~2-3 minutes (first build)
- **Tests**: ~30 seconds
- **Incremental**: ~10-20 seconds

### Parallel Builds
- Use `-j4` for 4 parallel jobs
- Adjust based on CPU cores

### Build Size
- **Bootloader**: ~32KB flash, 4KB RAM
- **Application**: ~128KB flash, 32KB RAM

## 🔗 Related Documentation

- [Test Suite README](tests/README.md) - Test framework details
- [Main README](README.md) - Project overview
- [Hardware Requirements](docs/hardware.md) - Hardware specifications

---

**Build Philosophy**: Separate firmware and test builds with proper toolchain configuration for reliable cross-compilation and native testing.
