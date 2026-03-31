/**
 * @file web_interface.c
 * @brief Web interface communication protocol implementation
 * 
 * This module implements JSON-based communication protocol between
 * the IoT device and web dashboard via UART. It handles sensor data
 * transmission, command processing, and response generation.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#include "web_interface.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "sensor_task.h"
#include <stdint.h>
#include <stdbool.h>

/* ── Configuration Constants ────────────────────────────────────── */

#define WEB_INTERFACE_TASK_PRIORITY    (tskIDLE_PRIORITY + 2U)
#define WEB_INTERFACE_TASK_STACK_SIZE   512U
#define WEB_INTERFACE_UPDATE_PERIOD_MS  2000U
#define WEB_HISTORY_SIZE               30U
#define WEB_UART_TIMEOUT_MS            100U
#define WEB_JSON_BUFFER_SIZE           256U
#define WEB_RESPONSE_BUFFER_SIZE       128U

/* Protocol constants */
#define WEB_PACKET_START_MARKER        0xAAU
#define WEB_PACKET_END_MARKER          0x55U
#define WEB_MAX_PACKET_DATA_SIZE       64U
#define WEB_CHECKSUM_INIT              0x00U

/* Command processing timeout */
#define WEB_COMMAND_RESPONSE_TIMEOUT_MS 5000U

/* ── Type Definitions ───────────────────────────────────────────── */

/**
 * @brief Web interface packet structure
 * 
 * Binary protocol packet for reliable UART communication
 * with checksum validation.
 */
typedef struct {
    uint8_t start_marker;        /**< Packet start marker (0xAA) */
    uint8_t command;              /**< Command identifier */
    uint8_t length;               /**< Data payload length */
    uint8_t data[WEB_MAX_PACKET_DATA_SIZE]; /**< Command data */
    uint8_t checksum;             /**< Packet checksum */
    uint8_t end_marker;          /**< Packet end marker (0x55) */
} __attribute__((packed)) WebPacket_t;

/**
 * @brief Packet parsing state machine
 */
typedef enum {
    PACKET_STATE_WAIT_START,      /**< Waiting for start marker */
    PACKET_STATE_READ_COMMAND,    /**< Reading command byte */
    PACKET_STATE_READ_LENGTH,     /**< Reading length byte */
    PACKET_STATE_READ_DATA,       /**< Reading data payload */
    PACKET_STATE_READ_CHECKSUM,   /**< Reading checksum */
    PACKET_STATE_READ_END,        /**< Reading end marker */
    PACKET_STATE_COMPLETE         /**< Packet complete */
} PacketState_t;

/* ── Static Data ───────────────────────────────────────────────── */

/** Command reception flag */
static volatile bool s_command_received = false;

/** Packet parsing buffers */
static WebPacket_t s_rx_packet;
static WebPacket_t s_tx_packet;

/** Sensor data history for web interface */
static FullSensorData_t s_sensor_history[WEB_HISTORY_SIZE];
static uint8_t s_history_index = 0U;
static uint8_t s_history_count = 0U;

/** LED status tracking */
static uint8_t s_current_led_status = 0U;

/** External handles - must be provided by main application */
extern QueueHandle_t g_sensor_queue;
extern QueueHandle_t g_uart_rx_queue;
extern SemaphoreHandle_t g_uart_mutex;

/* ── Helper Functions ──────────────────────────────────────────── */

/**
 * @brief Calculate packet checksum
 * @param pData Pointer to data
 * @param length Data length
 * @return Calculated checksum
 */
static uint8_t calculate_checksum(const uint8_t *const pData, const uint8_t length)
{
    uint8_t checksum = WEB_CHECKSUM_INIT;
    
    for (uint8_t i = 0U; i < length; i++) {
        checksum ^= pData[i];
    }
    
    return checksum;
}

/**
 * @brief Validate packet structure and checksum
 * @param pPacket Pointer to packet
 * @return true if packet is valid, false otherwise
 */
static bool validate_packet(const WebPacket_t *const pPacket)
{
    if ((pPacket->start_marker != WEB_PACKET_START_MARKER) ||
        (pPacket->end_marker != WEB_PACKET_END_MARKER)) {
        return false;
    }
    
    /* Calculate checksum of command + length + data */
    uint8_t calculated_checksum = calculate_checksum(&pPacket->command, 
                                                   pPacket->length + 2U);
    
    return (calculated_checksum == pPacket->checksum);
}

/**
 * @brief Send JSON response via UART
 * @param pStatus Response status string
 * @param pMessage Response message string
 * @return true if sent successfully, false otherwise
 */
static bool send_json_response(const char *const pStatus, const char *const pMessage)
{
    if ((pStatus == NULL) || (pMessage == NULL)) {
        return false;
    }
    
    if (xSemaphoreTake(g_uart_mutex, pdMS_TO_TICKS(WEB_UART_TIMEOUT_MS)) == pdTRUE) {
        char json_buffer[WEB_RESPONSE_BUFFER_SIZE];
        
        const int len = snprintf(json_buffer, sizeof(json_buffer),
            "{\"status\":\"%s\",\"message\":\"%s\",\"timestamp\":%lu}\n",
            pStatus, pMessage, (unsigned long)xTaskGetTickCount());
        
        if ((len > 0) && (len < (int)sizeof(json_buffer))) {
            const bool result = uart_send_string(json_buffer);
            xSemaphoreGive(g_uart_mutex);
            return result;
        }
        
        xSemaphoreGive(g_uart_mutex);
    }
    
    return false;
}

/**
 * @brief Send sensor data as JSON via UART
 * @param pData Pointer to sensor data
 * @return true if sent successfully, false otherwise
 */
static bool send_sensor_data_json(const FullSensorData_t *const pData)
{
    if (pData == NULL) {
        return false;
    }
    
    if (xSemaphoreTake(g_uart_mutex, pdMS_TO_TICKS(WEB_UART_TIMEOUT_MS)) == pdTRUE) {
        char json_buffer[WEB_JSON_BUFFER_SIZE];
        
        const int len = snprintf(json_buffer, sizeof(json_buffer),
            "{\"timestamp\":%lu,\"temperature\":%.1f,\"humidity\":%.1f,\"pressure\":%.1f,"
            "\"cpu\":%u,\"memory\":%u,\"pulse\":%u,\"battery\":%u,\"led_status\":%u}\n",
            (unsigned long)xTaskGetTickCount(),
            pData->temperature, pData->humidity, pData->pressure,
            pData->cpu_usage, pData->memory_usage, pData->pulse, 
            pData->battery_level, pData->led_status);
        
        if ((len > 0) && (len < (int)sizeof(json_buffer))) {
            const bool result = uart_send_string(json_buffer);
            xSemaphoreGive(g_uart_mutex);
            return result;
        }
        
        xSemaphoreGive(g_uart_mutex);
    }
    
    return false;
}

/**
 * @brief Send system status as JSON via UART
 * @return true if sent successfully, false otherwise
 */
static bool send_system_status_json(void)
{
    if (xSemaphoreTake(g_uart_mutex, pdMS_TO_TICKS(WEB_UART_TIMEOUT_MS)) == pdTRUE) {
        char json_buffer[WEB_JSON_BUFFER_SIZE];
        const uint32_t uptime = xTaskGetTickCount();
        const uint32_t free_heap = xPortGetFreeHeapSize();
        
        const int len = snprintf(json_buffer, sizeof(json_buffer),
            "{\"status\":\"online\",\"uptime\":%lu,\"firmware\":\"v2.1.3\","
            "\"free_memory\":%lu,\"total_memory\":%u,\"reboot_count\":0}\n",
            (unsigned long)uptime, (unsigned long)free_heap, configTOTAL_HEAP_SIZE);
        
        if ((len > 0) && (len < (int)sizeof(json_buffer))) {
            const bool result = uart_send_string(json_buffer);
            xSemaphoreGive(g_uart_mutex);
            return result;
        }
        
        xSemaphoreGive(g_uart_mutex);
    }
    
    return false;
}

/**
 * @brief Send sensor data history as JSON via UART
 * @return true if sent successfully, false otherwise
 */
static bool send_history_json(void)
{
    if (xSemaphoreTake(g_uart_mutex, pdMS_TO_TICKS(WEB_UART_TIMEOUT_MS)) == pdTRUE) {
        /* Send opening bracket */
        uart_send_string("{\"history\":[");
        
        for (uint8_t i = 0U; i < s_history_count; i++) {
            const uint8_t index = (s_history_index - s_history_count + i + WEB_HISTORY_SIZE) % WEB_HISTORY_SIZE;
            const FullSensorData_t *const pData = &s_sensor_history[index];
            
            char json_buffer[200];
            const int len = snprintf(json_buffer, sizeof(json_buffer),
                "{\"timestamp\":%lu,\"temperature\":%.1f,\"humidity\":%.1f,\"pressure\":%.1f,"
                "\"cpu\":%u,\"memory\":%u,\"pulse\":%u,\"battery\":%u,\"led_status\":%u}",
                (unsigned long)xTaskGetTickCount(),
                pData->temperature, pData->humidity, pData->pressure,
                pData->cpu_usage, pData->memory_usage, pData->pulse, 
                pData->battery_level, pData->led_status);
            
            if ((len > 0) && (len < (int)sizeof(json_buffer))) {
                uart_send_string(json_buffer);
                
                if (i < (s_history_count - 1U)) {
                    uart_send_string(",");
                }
            }
        }
        
        /* Send closing bracket */
        uart_send_string("]}\n");
        
        xSemaphoreGive(g_uart_mutex);
        return true;
    }
    
    return false;
}

/**
 * @brief Store sensor data in history buffer
 * @param pData Pointer to sensor data to store
 */
static void store_sensor_data(const FullSensorData_t *const pData)
{
    if (pData == NULL) {
        return;
    }
    
    /* Store in circular buffer */
    s_sensor_history[s_history_index] = *pData;
    s_history_index = (s_history_index + 1U) % WEB_HISTORY_SIZE;
    
    if (s_history_count < WEB_HISTORY_SIZE) {
        s_history_count++;
    }
}

/* ── Command Processing ───────────────────────────────────────── */

/**
 * @brief Process received web command
 * @param command Command identifier
 * @param pData Pointer to command data
 * @param length Data length
 */
static void process_command(const WebCommand_t command, const uint8_t *const pData, const uint8_t length)
{
    (void)pData;  /* Suppress unused parameter warning */
    (void)length; /* Suppress unused parameter warning */
    
    switch (command) {
        case WEB_CMD_GET_STATUS:
            (void)send_system_status_json();
            break;
            
        case WEB_CMD_GET_SENSORS:
            /* Send latest sensor data if available */
            if (s_history_count > 0U) {
                const uint8_t latest_index = (s_history_index - 1U + WEB_HISTORY_SIZE) % WEB_HISTORY_SIZE;
                (void)send_sensor_data_json(&s_sensor_history[latest_index]);
            }
            break;
            
        case WEB_CMD_LED_ON:
            s_current_led_status = 1U;
            /* Update LED hardware (mock implementation) */
            /* HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); */
            (void)send_json_response("success", "LED включен");
            break;
            
        case WEB_CMD_LED_OFF:
            s_current_led_status = 0U;
            /* Update LED hardware (mock implementation) */
            /* HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); */
            (void)send_json_response("success", "LED выключен");
            break;
            
        case WEB_CMD_REBOOT:
            (void)send_json_response("success", "Перезагрузка через 2 секунды");
            vTaskDelay(pdMS_TO_TICKS(2000U));
            NVIC_SystemReset();
            break; /* Never reached */
            
        case WEB_CMD_CALIBRATE:
            /* Mock calibration implementation */
            (void)send_json_response("success", "Калибровка выполнена");
            break;
            
        case WEB_CMD_GET_HISTORY:
            (void)send_history_json();
            break;
            
        case WEB_CMD_SYNC_TIME:
            (void)send_json_response("success", "Время синхронизировано");
            break;
            
        default:
            (void)send_json_response("error", "Неизвестная команда");
            break;
    }
}

/* ── Public API Functions ───────────────────────────────────────── */

/**
 * @brief Initialize web interface subsystem
 * @return true if initialization successful, false otherwise
 */
bool web_interface_init(void)
{
    /* Reset state variables */
    s_command_received = false;
    s_history_index = 0U;
    s_history_count = 0U;
    s_current_led_status = 0U;
    
    /* Clear packet buffers */
    (void)memset(&s_rx_packet, 0, sizeof(s_rx_packet));
    (void)memset(&s_tx_packet, 0, sizeof(s_tx_packet));
    (void)memset(s_sensor_history, 0, sizeof(s_sensor_history));
    
    return true;
}

/**
 * @brief Web interface communication task
 * 
 * This task handles UART communication with the web interface,
 * processes incoming commands, and sends sensor data periodically.
 * 
 * @param pvParameters Task parameters (unused)
 */
void web_interface_task(void *pvParameters)
{
    (void)pvParameters; /* Suppress unused parameter warning */
    
    const TickType_t xFrequency = pdMS_TO_TICKS(WEB_INTERFACE_UPDATE_PERIOD_MS);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    PacketState_t packet_state = PACKET_STATE_WAIT_START;
    uint8_t data_index = 0U;
    uint8_t calculated_checksum = WEB_CHECKSUM_INIT;
    
    FullSensorData_t current_sensor_data;
    
    for (;;) {
        /* Process UART receive queue */
        uint8_t rx_byte;
        while (xQueueReceive(g_uart_rx_queue, &rx_byte, 0) == pdTRUE) {
            switch (packet_state) {
                case PACKET_STATE_WAIT_START:
                    if (rx_byte == WEB_PACKET_START_MARKER) {
                        s_rx_packet.start_marker = rx_byte;
                        calculated_checksum = rx_byte;
                        packet_state = PACKET_STATE_READ_COMMAND;
                    }
                    break;
                    
                case PACKET_STATE_READ_COMMAND:
                    s_rx_packet.command = rx_byte;
                    calculated_checksum ^= rx_byte;
                    packet_state = PACKET_STATE_READ_LENGTH;
                    break;
                    
                case PACKET_STATE_READ_LENGTH:
                    s_rx_packet.length = rx_byte;
                    calculated_checksum ^= rx_byte;
                    data_index = 0U;
                    packet_state = (rx_byte > 0U) ? PACKET_STATE_READ_DATA : PACKET_STATE_READ_CHECKSUM;
                    break;
                    
                case PACKET_STATE_READ_DATA:
                    if (data_index < s_rx_packet.length && data_index < WEB_MAX_PACKET_DATA_SIZE) {
                        s_rx_packet.data[data_index] = rx_byte;
                        calculated_checksum ^= rx_byte;
                        data_index++;
                        
                        if (data_index >= s_rx_packet.length) {
                            packet_state = PACKET_STATE_READ_CHECKSUM;
                        }
                    }
                    break;
                    
                case PACKET_STATE_READ_CHECKSUM:
                    s_rx_packet.checksum = rx_byte;
                    packet_state = PACKET_STATE_READ_END;
                    break;
                    
                case PACKET_STATE_READ_END:
                    if (rx_byte == WEB_PACKET_END_MARKER) {
                        s_rx_packet.end_marker = rx_byte;
                        
                        if (validate_packet(&s_rx_packet)) {
                            s_command_received = true;
                            process_command((WebCommand_t)s_rx_packet.command, 
                                           s_rx_packet.data, s_rx_packet.length);
                        }
                    }
                    packet_state = PACKET_STATE_WAIT_START;
                    break;
                    
                default:
                    packet_state = PACKET_STATE_WAIT_START;
                    break;
            }
        }
        
        /* Process sensor data from sensor task */
        if (xQueueReceive(g_sensor_queue, &current_sensor_data, 0) == pdTRUE) {
            /* Update LED status in sensor data */
            current_sensor_data.led_status = s_current_led_status;
            
            /* Store in history */
            store_sensor_data(&current_sensor_data);
            
            /* Send sensor data via UART */
            (void)send_sensor_data_json(&current_sensor_data);
        }
        
        /* Wait for next cycle - precise timing */
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
    
    /* This line should never be reached */
    vTaskDelete(NULL);
}

/**
 * @brief Check if command was received
 * @return true if command received, false otherwise
 */
bool web_interface_command_received(void)
{
    return s_command_received;
}

/**
 * @brief Get last received command packet
 * @return Pointer to received packet
 */
const WebPacket_t* web_interface_get_last_packet(void)
{
    return &s_rx_packet;
}

/* ── UART Interface (mock implementation) ───────────────────────── */

/**
 * @brief Send string via UART
 * @param pString Pointer to null-terminated string
 * @return true if sent successfully, false otherwise
 */
bool uart_send_string(const char *const pString)
{
    if (pString == NULL) {
        return false;
    }
    
    /* Mock implementation - in production, use HAL_UART_Transmit */
    /* HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1, 
                                                   (uint8_t*)pString, 
                                                   strlen(pString), 
                                                   HAL_MAX_DELAY); */
    
    /* For simulation, just return success */
    return true;
}

/* ── Timing-Sensitive Regions (requires manual review) ───────────── */

/**
 * @note The following regions require manual review:
 * 
 * 1. UART transmission timing - verify HAL_UART_Transmit timeout
 * 2. Queue operations - verify timeout values are appropriate
 * 3. JSON buffer sizes - verify no overflow possible
 * 4. Task timing - verify 2-second period meets requirements
 * 5. Interrupt priority - verify UART interrupt priority allows FreeRTOS API calls
 * 
 * <REVIEW_REQUIRED>
 * - UART peripheral configuration and interrupt priority
 * - HAL_UART_Transmit timeout values
 * - Queue overflow handling strategy
 * - JSON buffer overflow protection
 * - Task stack size verification
 * </REVIEW_REQUIRED>
 */
