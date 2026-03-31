#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Basic HAL return types
typedef enum {
    HAL_OK       = 0x00U,
    HAL_ERROR    = 0x01U,
    HAL_BUSY     = 0x02U,
    HAL_TIMEOUT   = 0x03U
} HAL_StatusTypeDef;

// GPIO types
typedef enum {
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET   = 1
} GPIO_PinState;

typedef struct {
    uint32_t Pin;
    uint32_t Mode;
    uint32_t Pull;
    uint32_t Speed;
    uint32_t Alternate;
} GPIO_InitTypeDef;

typedef struct {
    uint32_t Mode;
    uint32_t Pull;
    uint32_t Speed;
    uint32_t Alternate;
} GPIO_TypeDef;

// UART types
typedef struct {
    uint32_t Instance;
    uint32_t Init;
    uint32_t State;
    uint8_t *pTxBuffPtr;
    uint16_t TxXferSize;
    uint16_t TxXferCount;
    uint8_t *pRxBuffPtr;
    uint16_t RxXferSize;
    uint16_t RxXferCount;
    uint16_t Mask;
    uint16_t ErrorCode;
} UART_HandleTypeDef;

// I2C types
typedef struct {
    uint32_t Instance;
    I2C_InitTypeDef Init;
    uint8_t *pBuffPtr;
    uint16_t XferSize;
    uint16_t XferCount;
    uint32_t XferOptions;
    uint32_t PreviousState;
    HAL_LockTypeDef Lock;
    uint32_t ErrorCode;
    uint32_t Addr;
    uint32_t Devaddress;
    uint32_t Memaddres;
} I2C_HandleTypeDef;

typedef struct {
    uint32_t ClockSpeed;
    uint32_t DutyCycle;
    uint32_t OwnAddress1;
    uint32_t OwnAddress2;
    uint32_t OwnAddress2Masks;
    uint32_t GeneralCallMode;
    uint32_t NoStretchMode;
    uint32_t Mode;
    uint32_t AcknowledgedAddress;
    uint32_t AddressingMode;
} I2C_InitTypeDef;

// SPI types
typedef struct {
    uint32_t Instance;
    SPI_InitTypeDef Init;
    uint8_t *pTxBuffPtr;
    uint16_t TxXferSize;
    uint16_t TxXferCount;
    uint8_t *pRxBuffPtr;
    uint16_t RxXferSize;
    uint16_t RxXferCount;
    uint32_t CRCSize;
    uint32_t RxISR;
    uint32_t TxISR;
    uint32_t ErrorCode;
    DMA_HandleTypeDef *hdmatx;
    DMA_HandleTypeDef *hdmarx;
    HAL_LockTypeDef Lock;
    uint32_t State;
} SPI_HandleTypeDef;

typedef struct {
    uint32_t Mode;
    uint32_t Direction;
    uint32_t DataSize;
    uint32_t CLKPolarity;
    uint32_t CLKPhase;
    uint32_t NSS;
    uint32_t BaudRatePrescaler;
    uint32_t FirstBit;
    uint32_t TIMode;
    uint32_t CRCCalculation;
    uint32_t CRCPolynomial;
} SPI_InitTypeDef;

// Flash types
typedef struct {
    uint32_t TypeErase;
    uint32_t VoltageRange;
    uint32_t Sector;
    uint32_t NbSectors;
    uint32_t SectorError;
} FLASH_EraseInitTypeDef;

// System types
typedef struct {
    uint32_t SYSCLK_Frequency;
    uint32_t HCLK_Frequency;
    uint32_t PCLK1_Frequency;
    uint32_t PCLK2_Frequency;
} RCC_ClkInitTypeDef;

// Constants
#define GPIO_MODE_OUTPUT_PP     0x01U
#define GPIO_MODE_INPUT        0x00U
#define GPIO_MODE_AF_PP        0x02U
#define GPIO_NOPULL           0x00U
#define GPIO_SPEED_FREQ_LOW     0x00U

#define UART_WORDLENGTH_8B      0x00U
#define UART_STOPBITS_1         0x00U
#define UART_PARITY_NONE        0x00U
#define UART_MODE_TX_RX         0x0CU
#define UART_HWCONTROL_NONE     0x00U
#define UART_OVERSAMPLING_16   0x00U
#define UART_IT_IDLE           0x1000U

#define I2C_ADDRESSINGMODE_7BIT  0x00U

#define SPI_MODE_MASTER          0x01U
#define SPI_DIRECTION_2LINES     0x00U
#define SPI_DATASIZE_8BIT       0x00U
#define SPI_POLARITY_LOW        0x00U
#define SPI_PHASE_1EDGE         0x01U
#define SPI_NSS_SOFT            0x01U
#define SPI_BAUDRATEPRESCALER_64 0x20U

#define FLASH_TYPEERASE_SECTORS  0x01U
#define FLASH_VOLTAGE_RANGE_3   0x02U
#define FLASH_SECTOR_5          0x05U
#define FLASH_SECTOR_7          0x07U

#define HAL_MAX_DELAY           0xFFFFFFFFU

#define RCC_PERIPHCLK_USART2     0x00000000U
#define RCC_PERIPHCLK_I2C1      0x00200000U

// Function declarations
HAL_StatusTypeDef HAL_Init(void);
void HAL_Delay(uint32_t delay);
uint32_t HAL_GetTick(void);

void HAL_GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin);
void HAL_GPIO_Init(GPIO_InitTypeDef* init);
void __HAL_RCC_GPIOC_CLK_ENABLE(void);
void __HAL_RCC_USART2_CLK_ENABLE(void);
void __HAL_RCC_I2C1_CLK_ENABLE(void);
void __HAL_RCC_SPI1_CLK_ENABLE(void);
void __HAL_RCC_DMA1_CLK_ENABLE(void);
void __HAL_RCC_GPIOA_CLK_ENABLE(void);

HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef* huart);
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size);
HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size);
void HAL_UART_IRQHandler(UART_HandleTypeDef* huart);

HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef* hi2c);
HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef* hi2c, uint16_t dev_addr, uint8_t* data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef* hi2c, uint16_t dev_addr, uint8_t* data, uint16_t size, uint32_t timeout);

HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef* hspi);
HAL_StatusTypeDef HAL_SPI_TransmitReceive_DMA(SPI_HandleTypeDef* hspi, uint8_t* tx_data, uint8_t* rx_data, uint16_t size);

HAL_StatusTypeDef HAL_FLASH_Unlock(void);
HAL_StatusTypeDef HAL_FLASH_Lock(void);
HAL_StatusTypeDef HAL_FLASH_Program(uint32_t type_program, uint32_t address, uint64_t data);
HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef* erase, uint32_t* sector_error);

void SystemClock_Config(void);
void Error_Handler(void);

#ifdef __cplusplus
}
#endif
