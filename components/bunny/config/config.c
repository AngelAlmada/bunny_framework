#include "config.h"

/* Valores por defecto — se sobreescribirán al parsear device.json */
static bunny_config_t s_config = {
    .device = {
        .id          = "esp32-001",
        .name        = "Mi Dispositivo Bunny",
        .description = "Dispositivo ESP32 gestionado por Bunny Framework",
        .type        = "esp32",
        .version     = "0.1.0",
    },
    .discovery = {
        .enabled              = true,
        .udp_port             = 5555,
        .broadcast_interval_ms = 3000,
    },
    .webhook = {
        .port = 8080,
        .path = "/bunny",
    },
};

void bunny_config_load(void)
{
    /* TODO: parsear config/device.json desde SPIFFS y rellenar s_config */
}

const bunny_config_t *bunny_config_get(void)
{
    return &s_config;
}
