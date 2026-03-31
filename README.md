# 🚀 Secure IoT Sensor Node (STM32 + FreeRTOS)

A production-ready embedded firmware project demonstrating advanced IoT device capabilities with secure boot, OTA updates, and real-time data processing.

## 🧩 Overview

This project implements a comprehensive IoT sensor node with the following key features:

- **Secure Boot Chain** - SHA256 hash verification + digital signature validation
- **Dual-Bank OTA Updates** - Fail-safe firmware updates with automatic rollback
- **RTOS Architecture** - FreeRTOS-based concurrent task design
- **High-Performance Communication** - DMA-based UART/SPI with interrupt handling
- **Flash Storage Management** - Wear-leveling and circular buffer implementation
- **Binary Protocol** - CRC-protected communication protocol
- **Real-time Sensor Processing** - Continuous data acquisition and transmission

## 🏗️ Architecture

### Memory Layout

```
0x08000000 ┌─────────────────┐
           │   Bootloader    │ 64KB
           │   (Secure)      │
0x08010000 ├─────────────────┤
           │   Application   │ 256KB
           │   Slot A        │
0x08050000 ├─────────────────┤
           │   Application   │ 256KB
           │   Slot B (OTA)  │
0x08090000 ├─────────────────┤
           │   Metadata      │ 4KB
           │   (OTA State)   │
0x08091000 ├─────────────────┤
           │   Storage       │ 128KB
           │   (Data Log)    │
0x080B1000 └─────────────────┘
```

### System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    FreeRTOS Kernel                           │
├─────────────────────────────────────────────────────────────┤
│  Sensor Task  │  Comm Task  │  Storage Task  │  Update Task   │
├─────────────────────────────────────────────────────────────┤
│                Middleware Layer                              │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────────┐ │
│  │   Queue     │ │   Mutex     │ │    OTA Metadata        │ │
│  │ Management  │ │ Management  │ │    Management          │ │
│  └─────────────┘ └─────────────┘ └─────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                  Driver Layer                                │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐           │
│  │   UART DMA  │ │   SPI DMA   │ │   Flash     │           │
│  │   + Idle    │ │   + Double  │ │   Wear      │           │
│  │   Detection │ │   Buffer    │ │   Leveling  │           │
│  └─────────────┘ └─────────────┘ └─────────────┘           │
├─────────────────────────────────────────────────────────────┤
│                  Hardware Layer                              │
│           STM32F4xx (Cortex-M4 @ 168MHz)                    │
└─────────────────────────────────────────────────────────────┘
```

## 📡 Communication Protocol

### Binary Packet Format

```
┌────────┬──────┬─────────────────┬──────┐
│ Header │ Type │   Payload       │ CRC  │
│  0xAA  │  1B  │ Variable Length │ 2B   │
└────────┴──────┴─────────────────┴──────┘
```

### OTA Update Protocol

```
HOST → DEVICE:
START
SIZE <bytes>
DATA <chunk>
DATA <chunk>
...
END

DEVICE → HOST:
READY
OK
OK
...
OK (then reset)
```

## 🔐 Security Features

### Secure Boot Process

1. **Bootloader Initialization**
   - Hardware peripheral setup
   - Metadata integrity check

2. **Firmware Verification**
   - Stack pointer validation
   - Reset vector verification
   - SHA256 hash computation
   - Digital signature verification

3. **Application Launch**
   - Vector table relocation
   - Stack pointer setup
   - Jump to application entry point

### OTA Update Security

- **Dual-Bank Architecture** - Redundant firmware slots
- **Atomic Updates** - Complete verification before activation
- **Automatic Rollback** - Failed update recovery
- **CRC Validation** - Data integrity verification
- **Version Control** - Firmware version tracking

## ⚙️ Technical Implementation

### DMA-Based UART Communication

```c
// Circular DMA with idle line detection
HAL_UART_Receive_DMA(&huart2, rx_buffer, RX_BUFFER_SIZE);
__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);

// Zero-copy buffer processing
void process_rx_data(uint8_t* data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        xQueueSendFromISR(uart_rx_queue, &data[i], NULL);
    }
}
```

### SPI Double Buffering

```c
// Ping-pong buffer design
static uint16_t dma_buffer[BUF_SIZE * 2]; // A + B

// Half/Full transfer callbacks
void HAL_SPI_RxHalfCpltCallback() { process_buffer(&dma_buffer[0]); }
void HAL_SPI_RxCpltCallback() { process_buffer(&dma_buffer[BUF_SIZE]); }
```

### Flash Wear Leveling

```c
// Logical to physical block mapping
typedef struct {
    uint32_t logical_block;
    uint32_t physical_block;
    uint32_t erase_count;
    uint32_t is_valid;
} wear_block_meta_t;

// Write distribution across blocks
boot_error_t wear_level_write(uint8_t* data, uint32_t len) {
    // Find block with minimum erase count
    // Erase and write to minimize wear
}
```

## 🧬 FreeRTOS Task Design

### Task Priorities and Responsibilities

| Task | Priority | Stack | Responsibility |
|------|----------|-------|----------------|
| Update Task | 3 (High) | 256B | OTA update processing |
| Comm Task | 2 (Medium) | 256B | UART communication |
| Storage Task | 2 (Medium) | 256B | Flash data logging |
| Sensor Task | 1 (Low) | 256B | Sensor data acquisition |

### Inter-Task Communication

```c
// Queue-based data flow
QueueHandle_t sensor_queue;     // Sensor → Comm/Storage
QueueHandle_t uart_rx_queue;    // UART → Update Task
QueueHandle_t data_queue;       // SPI → Processing Task

// Resource protection
SemaphoreHandle_t flash_mutex;  // Flash access protection
SemaphoreHandle_t uart_mutex;  // UART access protection
```

## 🛠️ Build and Development

### Prerequisites

- **Toolchain**: ARM GNU Toolchain (arm-none-eabi-gcc)
- **Debugger**: ST-Link or J-Link
- **IDE**: STM32CubeIDE, VS Code with Cortex-Debug
- **Flash Utility**: st-flash or OpenOCD

### Build Commands

```bash
# Build all components
make all

# Build individual components
make bootloader
make app

# Flash to device
make flash-bootloader
make flash-app

# Debug session
make debug

# Clean build
make clean
```

### Project Structure

```
├── bootloader/          # Secure bootloader
│   ├── bootloader.c     # Main bootloader logic
│   ├── flash_if.c       # Flash interface
│   ├── signature.c      # Crypto functions
│   └── bootloader.ld    # Linker script
├── app/                 # Main application
│   ├── main.c           # Application entry
│   ├── *_task.c         # FreeRTOS tasks
│   └── app.ld           # Application linker script
├── drivers/             # Hardware drivers
│   ├── uart.c           # DMA UART driver
│   ├── spi.c            # DMA SPI driver
│   ├── i2c.c            # I2C sensor driver
│   ├── gpio.c           # GPIO driver
│   └── flash_if.c       # Flash storage driver
├── middleware/          # Middleware components
│   ├── freertos/        # RTOS kernel
│   └── crypto/          # Security & OTA metadata
├── config/              # Configuration files
│   ├── system_config.h  # System definitions
│   └── memory_map.h     # Memory layout
└── Makefile            # Build system
```

## 📊 Performance Characteristics

### Real-Time Capabilities

- **Sensor Sampling**: 100ms period (10 Hz)
- **Data Transmission**: Binary protocol with CRC
- **OTA Transfer**: Configurable chunk size (256B default)
- **Flash Write**: Wear-leveling with 10,000 cycle endurance
- **DMA Throughput**: Zero-copy buffer processing

### Memory Usage

| Component | Flash Usage | RAM Usage |
|-----------|-------------|-----------|
| Bootloader | ~32KB | ~4KB |
| Application | ~128KB | ~32KB |
| FreeRTOS Heap | - | ~16KB |
| Buffers | - | ~8KB |

## 🔍 Debugging and Testing

### Debug Features

- **SWD Debug Interface** - Full debug capability
- **UART Logging** - Real-time status output
- **LED Status Indicators** - Visual system state
- **Assert Handlers** - Error condition detection
- **Stack Overflow Detection** - FreeRTOS monitoring

### Test Scenarios

1. **Normal Operation**
   - Sensor data acquisition
   - UART communication
   - Flash storage

2. **OTA Update Flow**
   - Firmware transfer
   - Verification process
   - System restart

3. **Error Recovery**
   - Corrupted firmware
   - Failed update rollback
   - Flash wear scenarios

## 🚀 Production Deployment

### Security Considerations

- **Key Management** - Secure key storage for signatures
- **Supply Chain Security** - Code signing process
- **Runtime Protection** - Memory protection units
- **Update Authentication** - Secure OTA channels

### Scalability Features

- **Modular Architecture** - Easy feature addition
- **Configurable Parameters** - Runtime configuration
- **Multi-Protocol Support** - UART/SPI/I2C interfaces
- **Cloud Integration** - Protocol extensibility

## 📈 Demonstrated Skills

This project showcases expertise in:

- **Embedded C/C++** with ARM Cortex-M architecture
- **Real-Time Operating Systems** (FreeRTOS)
- **Hardware Abstraction** and driver development
- **DMA and interrupt-driven programming**
- **Secure firmware design** and cryptography
- **Wireless communication protocols**
- **Flash memory management** and wear leveling
- **Production-ready build systems**
- **Debugging and testing methodologies**

## 📝 License

This project is provided as a demonstration of embedded systems engineering capabilities. Use for educational and portfolio purposes.

---

**Architecture Highlights**: RTOS-based concurrent design, secure boot chain with hash verification, dual-bank firmware updates, flash wear-leveling strategy, binary communication protocol with CRC protection.
