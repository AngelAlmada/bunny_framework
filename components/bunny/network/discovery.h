#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Descubrimiento UDP de Bunny
 *
 * El ESP32 emite broadcasts UDP periódicos con su identidad
 * (id, name, ip, webhook_port, path) para que el motor de procesos lo encuentre
 * en la red local sin necesidad de configuración manual.
 *
 * Formato del mensaje emitido (JSON):
 * {
 *   "bunny": true,
 *   "id": "esp32-001",
 *   "name": "Mi Dispositivo Bunny",
 *   "version": "0.1.0",
 *   "ip": "192.168.1.XX",
 *   "webhook_port": 8080,
 *   "webhook_path": "/bunny"
 * }
 */

void bunny_discovery_init(void);
void bunny_discovery_start(void);
void bunny_discovery_stop(void);

#ifdef __cplusplus
}
#endif
