#include "bootloader.h"
#include "stm32f4xx_hal.h"

// Global variables
static UART_HandleTypeDef huart2;
static ota_metadata_t g_metadata;

// External symbols for application
extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

// Bootloader initialization
boot_error_t bootloader_init(void)
{
    HAL_Init();
    SystemClock_Config();
    
    // Initialize GPIO for status LED
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = LED_STATUS_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_STATUS_PORT, &gpio);
    
    // Initialize UART for debugging
    __HAL_RCC_USART2_CLK_ENABLE();
    huart2.Instance = USART2;
    huart2.Init.BaudRate = UART_BAUDRATE;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
    
    // Initialize metadata
    boot_error_t err = metadata_init();
    if (err != BOOT_OK) {
        led_blink(5, 200); // Error pattern
        return err;
    }
    
    led_set_status(1); // LED on - bootloader active
    return BOOT_OK;
}

// Main bootloader logic
boot_error_t bootloader_main(void)
{
    boot_error_t err;
    
    // Read metadata
    err = metadata_read(&g_metadata);
    if (err != BOOT_OK) {
        return err;
    }
    
    // Check if update is pending
    if (g_metadata.update_pending) {
        // Verify firmware in slot B
        err = verify_firmware(APP_SLOT_B_ADDR, g_metadata.size);
        if (err == BOOT_OK) {
            // Switch to slot B
            g_metadata.active_slot = 1;
            g_metadata.update_pending = 0;
            metadata_write(&g_metadata);
            
            jump_to_application(APP_SLOT_B_ADDR);
            return BOOT_OK;
        } else {
            // Rollback - keep slot A
            g_metadata.update_pending = 0;
            metadata_write(&g_metadata);
            
            // Fall through to slot A
        }
    }
    
    // Boot from active slot
    uint32_t app_addr = (g_metadata.active_slot == 0) ? APP_SLOT_A_ADDR : APP_SLOT_B_ADDR;
    
    // Verify firmware
    err = verify_firmware(app_addr, g_metadata.size);
    if (err != BOOT_OK) {
        // Try fallback to slot A if current slot is B
        if (g_metadata.active_slot == 1) {
            err = verify_firmware(APP_SLOT_A_ADDR, APP_SIZE);
            if (err == BOOT_OK) {
                g_metadata.active_slot = 0;
                metadata_write(&g_metadata);
                jump_to_application(APP_SLOT_A_ADDR);
                return BOOT_OK;
            }
        }
        return err;
    }
    
    jump_to_application(app_addr);
    return BOOT_OK;
}

// Verify firmware integrity
boot_error_t verify_firmware(uint32_t addr, uint32_t size)
{
    if (size == 0 || size > APP_SIZE) {
        return BOOT_CRC_INVALID;
    }
    
    // Check stack pointer
    uint32_t stack_ptr = *(volatile uint32_t*)addr;
    if (stack_ptr < 0x20000000 || stack_ptr > 0x20020000) {
        return BOOT_NO_VALID_FIRMWARE;
    }
    
    // Check reset vector
    uint32_t reset_vector = *(volatile uint32_t*)(addr + 4);
    if (reset_vector < APP_SLOT_A_ADDR || reset_vector > APP_SLOT_B_ADDR + APP_SIZE) {
        return BOOT_NO_VALID_FIRMWARE;
    }
    
    // Compute and verify CRC
    uint32_t computed_crc = compute_crc32(addr, size);
    if (computed_crc != g_metadata.crc) {
        return BOOT_CRC_INVALID;
    }
    
    // Compute SHA256 hash
    uint8_t hash[32];
    compute_sha256((uint8_t*)addr, size, hash);
    
    // Verify signature (simplified - in real implementation would use RSA/ECDSA)
    uint8_t signature[256];
    if (verify_signature(hash, signature) != BOOT_OK) {
        return BOOT_SIGNATURE_INVALID;
    }
    
    return BOOT_OK;
}

// Verify digital signature (simplified implementation)
boot_error_t verify_signature(uint8_t* hash, uint8_t* signature)
{
    // In a real implementation, this would use mbedTLS or similar crypto library
    // For demonstration, we'll use a simple check
    
    // Check if signature exists (placeholder check)
    for (int i = 0; i < 256; i++) {
        if (signature[i] != 0x00) {
            return BOOT_OK; // Signature present
        }
    }
    
    return BOOT_SIGNATURE_INVALID;
}

// Jump to application
void jump_to_application(uint32_t addr)
{
    uint32_t stack_ptr = *(volatile uint32_t*)addr;
    uint32_t reset_vector = *(volatile uint32_t*)(addr + 4);
    
    // Disable interrupts
    __disable_irq();
    
    // Set stack pointer
    __set_MSP(stack_ptr);
    
    // Disable SysTick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    
    // Reset peripherals
    HAL_RCC_DeInit();
    
    // Set vector table offset
    SCB->VTOR = addr;
    
    // Enable interrupts
    __enable_irq();
    
    // Jump to application
    pFunction jump = (pFunction)reset_vector;
    jump();
}

// System reset
void system_reset(void)
{
    HAL_NVIC_SystemReset();
}

// LED control functions
void led_set_status(uint8_t on)
{
    HAL_GPIO_WritePin(LED_STATUS_PORT, LED_STATUS_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void led_blink(uint8_t count, uint32_t delay_ms)
{
    for (uint8_t i = 0; i < count; i++) {
        led_set_status(1);
        delay_ms(delay_ms);
        led_set_status(0);
        delay_ms(delay_ms);
    }
}

// Simple delay
void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}
