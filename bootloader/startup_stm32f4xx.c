#include "bootloader.h"

// Reset handler
void Reset_Handler(void);

// Default exception handlers
void Default_Handler(void) {
    while (1) {}
}

void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

// Vector table
__attribute__((section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))(&_estack),             // Top of Stack
    Reset_Handler,                          // Reset Handler
    NMI_Handler,                            // NMI Handler
    HardFault_Handler,                      // Hard Fault Handler
    MemManage_Handler,                      // MPU Fault Handler
    BusFault_Handler,                       // Bus Fault Handler
    UsageFault_Handler,                     // Usage Fault Handler
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    SVC_Handler,                            // SVCall Handler
    DebugMon_Handler,                       // Debug Monitor Handler
    0,                                      // Reserved
    PendSV_Handler,                         // PendSV Handler
    SysTick_Handler,                        // SysTick Handler
    
    // External Interrupts
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Reset handler
void Reset_Handler(void)
{
    uint32_t *pSrc, *pDest;
    
    // Copy data from flash to RAM
    pSrc = &_sidata;
    pDest = &_sdata;
    while (pDest < &_edata) {
        *pDest++ = *pSrc++;
    }
    
    // Zero fill the bss segment
    pDest = &_sbss;
    while (pDest < &_ebss) {
        *pDest++ = 0;
    }
    
    // Initialize bootloader
    if (bootloader_init() == BOOT_OK) {
        // Run bootloader main logic
        bootloader_main();
    }
    
    // If we get here, something went wrong
    while (1) {
        led_blink(1, 1000); // Slow blink for error
    }
}
