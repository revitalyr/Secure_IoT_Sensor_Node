/**
 * @file web_interface.h
 * @brief Web interface communication protocol interface
 * 
 * This header defines the web interface protocol, command structures,
 * and public API functions for JSON-based UART communication.
 * 
 * @author Secure IoT Team
 * @date 2026
 * @version 1.0.0
 */

#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include "main.h"
#include "uart.h"
#include "sensor_task.h"
#include <stdint.h>
#include <stdbool.h>

/* ── Public Type Definitions ───────────────────────────────────── */

/**
 * @brief Web interface commands
 */
typedef enum {
    WEB_CMD_GET_STATUS = 0x01U,    /**< Get system status */
    WEB_CMD_GET_SENSORS = 0x02U,   /**< Get sensor data */
    WEB_CMD_LED_ON = 0x03U,        /**< Turn LED on */
    WEB_CMD_LED_OFF = 0x04U,       /**< Turn LED off */
    WEB_CMD_REBOOT = 0x05U,        /**< Reboot device */
    WEB_CMD_CALIBRATE = 0x06U,     /**< Calibrate sensors */
    WEB_CMD_GET_HISTORY = 0x07U,   /**< Get sensor history */
    WEB_CMD_SYNC_TIME = 0x08U      /**< Sync time */
} WebCommand_t;

/**
 * @brief Web interface response status
 */
typedef enum {
    WEB_STATUS_OK = 0x00U,         /**< Operation successful */
    WEB_STATUS_ERROR = 0x01U,      /**< General error */
    WEB_STATUS_INVALID_CMD = 0x02U, /**< Invalid command */
    WEB_STATUS_BUSY = 0x03U       /**< Device busy */
} WebStatus_t;

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
    uint8_t data[64];             /**< Command data payload */
    uint8_t checksum;             /**< Packet checksum */
    uint8_t end_marker;          /**< Packet end marker (0x55) */
} __attribute__((packed)) WebPacket_t;

/* ── Public API Functions ───────────────────────────────────────── */

/**
 * @brief Initialize web interface subsystem
 * @return true if initialization successful, false otherwise
 */
bool web_interface_init(void);

/**
 * @brief Web interface communication task
 * 
 * This task handles UART communication with the web interface,
 * processes incoming commands, and sends sensor data periodically.
 * 
 * @param pvParameters Task parameters (unused)
 */
void web_interface_task(void *pvParameters);

/**
 * @brief Check if command was received
 * @return true if command received, false otherwise
 */
bool web_interface_command_received(void);

/**
 * @brief Get last received command packet
 * @return Pointer to received packet
 */
const WebPacket_t* web_interface_get_last_packet(void);

/**
 * @brief Send string via UART
 * @param pString Pointer to null-terminated string
 * @return true if sent successfully, false otherwise
 */
bool uart_send_string(const char *const pString);

/* ── Configuration Constants ───────────────────────────────────── */

/** Web interface task configuration */
#define WEB_INTERFACE_TASK_PRIORITY    (tskIDLE_PRIORITY + 2U)
#define WEB_INTERFACE_TASK_STACK_SIZE   512U
#define WEB_INTERFACE_UPDATE_PERIOD_MS  2000U

/* Protocol constants */
#define WEB_PACKET_START_MARKER        0xAAU
#define WEB_PACKET_END_MARKER          0x55U
#define WEB_MAX_PACKET_DATA_SIZE       64U
#define WEB_HISTORY_SIZE               30U

/* Timing constants */
#define WEB_UART_TIMEOUT_MS            100U
#define WEB_JSON_BUFFER_SIZE           256U
#define WEB_RESPONSE_BUFFER_SIZE       128U
#define WEB_COMMAND_RESPONSE_TIMEOUT_MS 5000U

/* ── Safety Macros ───────────────────────────────────────────── */

/**
 * @brief Validate command ID range
 * @param cmd Command ID to validate
 * @return true if command is valid, false otherwise
 */
static inline bool web_is_valid_command(const uint8_t cmd)
{
    return ((cmd >= WEB_CMD_GET_STATUS) && (cmd <= WEB_CMD_SYNC_TIME));
}

/**
 * @brief Validate packet data length
 * @param length Data length to validate
 * @return true if length is valid, false otherwise
 */
static inline bool web_is_valid_length(const uint8_t length)
{
    return (length <= WEB_MAX_PACKET_DATA_SIZE);
}

#endif /* WEB_INTERFACE_H */
