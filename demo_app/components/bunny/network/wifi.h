#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize WiFi from device configuration
 * Reads SSID and password from config and attempts connection
 * @return true if WiFi initialization started successfully
 */
bool bunny_wifi_init(void);

/**
 * Check if WiFi is connected
 * @return true if connected to access point
 */
bool bunny_wifi_connected(void);

/**
 * Get the IP address of the WiFi interface
 * @return pointer to IP string (e.g., "192.168.1.100") or NULL if not connected
 */
const char* bunny_wifi_get_ip(void);

/**
 * Start WiFi connection with timeout
 * Blocks until connected or timeout
 * @param timeout_ms timeout in milliseconds
 * @return true if connected successfully
 */
bool bunny_wifi_connect_wait(uint32_t timeout_ms);

/**
 * Stop WiFi connection
 */
void bunny_wifi_stop(void);

#ifdef __cplusplus
}
#endif
