#include "i2c.h"
#include "stm32f4xx_hal.h"

// I2C handle
I2C_HandleTypeDef hi2c1;

// Mock sensor address (e.g., LM75 temperature sensor)
#define TEMP_SENSOR_ADDR    0x48
#define TEMP_REG_TEMP       0x00

void i2c_init(void)
{
    // GPIO Configuration for I2C1 (PB6: SCL, PB7: SDA)
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();
    
    GPIO_InitTypeDef gpio = {0};
    
    // I2C1 SCL (PB6)
    gpio.Pin = GPIO_PIN_6;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gpio);
    
    // I2C1 SDA (PB7)
    gpio.Pin = GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gpio);
    
    // I2C1 Configuration
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = I2C_CLOCK_HZ;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    HAL_I2C_Init(&hi2c1);
}

uint8_t i2c_read_byte(uint8_t dev_addr, uint8_t reg_addr)
{
    uint8_t data = 0;
    HAL_I2C_Master_Transmit(&hi2c1, dev_addr, &reg_addr, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c1, dev_addr, &data, 1, HAL_MAX_DELAY);
    return data;
}

uint16_t i2c_read_word(uint8_t dev_addr, uint8_t reg_addr)
{
    uint8_t data[2];
    HAL_I2C_Master_Transmit(&hi2c1, dev_addr, &reg_addr, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c1, dev_addr, data, 2, HAL_MAX_DELAY);
    return (data[0] << 8) | data[1];
}

boot_error_t i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint16_t len)
{
    HAL_StatusTypeDef status;
    
    status = HAL_I2C_Master_Transmit(&hi2c1, dev_addr, &reg_addr, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return BOOT_FLASH_ERROR;
    }
    
    status = HAL_I2C_Master_Receive(&hi2c1, dev_addr, data, len, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return BOOT_FLASH_ERROR;
    }
    
    return BOOT_OK;
}

boot_error_t i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
    uint8_t buf[2] = {reg_addr, data};
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, dev_addr, buf, 2, HAL_MAX_DELAY);
    return (status == HAL_OK) ? BOOT_OK : BOOT_FLASH_ERROR;
}

boot_error_t i2c_write_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint16_t len)
{
    uint8_t buf[32];
    if (len + 1 > sizeof(buf)) {
        return BOOT_FLASH_ERROR;
    }
    
    buf[0] = reg_addr;
    memcpy(&buf[1], data, len);
    
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, dev_addr, buf, len + 1, HAL_MAX_DELAY);
    return (status == HAL_OK) ? BOOT_OK : BOOT_FLASH_ERROR;
}

float i2c_read_temperature(void)
{
    // Mock implementation - in real system would read from actual sensor
    // For demonstration, we'll return a simulated temperature
    static float temp = 36.6f;
    static int counter = 0;
    
    counter++;
    temp += 0.05f * ((counter % 20) - 10); // Add some variation
    
    if (temp > 38.0f) temp = 36.0f;
    if (temp < 35.0f) temp = 37.0f;
    
    return temp;
    
    // Real implementation would be:
    // int16_t raw_temp = (int16_t)i2c_read_word(TEMP_SENSOR_ADDR, TEMP_REG_TEMP);
    // return (float)(raw_temp >> 5) * 0.125f; // LM75 conversion
}
