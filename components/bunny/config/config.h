#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Datos de identidad del dispositivo — el usuario los configura en config/device.json */
typedef struct {
    const char *id;
    const char *name;
    const char *description;
    const char *type;
    const char *version;
} bunny_device_info_t;

/* Configuración del descubrimiento UDP */
typedef struct {
    bool enabled;
    uint16_t udp_port;
    uint32_t broadcast_interval_ms;
} bunny_discovery_config_t;

/* Configuración del webhook (servidor HTTP en el ESP32) */
typedef struct {
    uint16_t port;
    const char *path;
} bunny_webhook_config_t;

/* Configuración completa del dispositivo */
typedef struct {
    bunny_device_info_t      device;
    bunny_discovery_config_t discovery;
    bunny_webhook_config_t   webhook;
} bunny_config_t;

void bunny_config_load(void);
const bunny_config_t *bunny_config_get(void);

#ifdef __cplusplus
}
#endif
