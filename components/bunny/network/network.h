#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bunny Network – WebSocket / MQTT transport layer */
void bunny_network_init(void);
void bunny_network_connect(void);
bool bunny_network_ws_connected(void);

/* WiFi connectivity */
bool bunny_wifi_init(void);
bool bunny_wifi_connected(void);
const char* bunny_wifi_get_ip(void);
bool bunny_wifi_connect_wait(uint32_t timeout_ms);
void bunny_wifi_stop(void);

#ifdef __cplusplus
}
#endif
