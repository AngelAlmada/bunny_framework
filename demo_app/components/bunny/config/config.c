#include "config.h"
#include <string.h>
#include <esp_log.h>

static const char *TAG = "bunny_config";

#ifndef BUNNY_DEVICE_ID
#define BUNNY_DEVICE_ID "esp32-001"
#endif

#ifndef BUNNY_DEVICE_NAME
#define BUNNY_DEVICE_NAME "Mi Dispositivo Bunny"
#endif

#ifndef BUNNY_DEVICE_DESCRIPTION
#define BUNNY_DEVICE_DESCRIPTION "Dispositivo ESP32 gestionado por Bunny Framework"
#endif

#ifndef BUNNY_DEVICE_TYPE
#define BUNNY_DEVICE_TYPE "esp32"
#endif

#ifndef BUNNY_DEVICE_VERSION
#define BUNNY_DEVICE_VERSION "0.1.0"
#endif

#ifndef BUNNY_WIFI_SSID
#define BUNNY_WIFI_SSID "TU_RED_WIFI"
#endif

#ifndef BUNNY_WIFI_PASSWORD
#define BUNNY_WIFI_PASSWORD "TU_CONTRASEÑA"
#endif

#ifndef BUNNY_WIFI_AUTH_TYPE
#define BUNNY_WIFI_AUTH_TYPE "WPA2"
#endif

#ifndef BUNNY_WIFI_MAX_RETRIES
#define BUNNY_WIFI_MAX_RETRIES 5
#endif

#ifndef BUNNY_WIFI_TIMEOUT_MS
#define BUNNY_WIFI_TIMEOUT_MS 10000
#endif

#ifndef BUNNY_DISCOVERY_ENABLED
#define BUNNY_DISCOVERY_ENABLED true
#endif

#ifndef BUNNY_DISCOVERY_UDP_PORT
#define BUNNY_DISCOVERY_UDP_PORT 5555
#endif

#ifndef BUNNY_DISCOVERY_BROADCAST_INTERVAL_MS
#define BUNNY_DISCOVERY_BROADCAST_INTERVAL_MS 3000
#endif

#ifndef BUNNY_WEBHOOK_PORT
#define BUNNY_WEBHOOK_PORT 8080
#endif

#ifndef BUNNY_WEBHOOK_PATH
#define BUNNY_WEBHOOK_PATH "/bunny"
#endif

/* Valores por defecto — se sobreescribirán al parsear device.json */
static bunny_config_t s_config = {
    .device = {
        .id          = BUNNY_DEVICE_ID,
        .name        = BUNNY_DEVICE_NAME,
        .description = BUNNY_DEVICE_DESCRIPTION,
        .type        = BUNNY_DEVICE_TYPE,
        .version     = BUNNY_DEVICE_VERSION,
    },
    .discovery = {
        .enabled               = BUNNY_DISCOVERY_ENABLED,
        .udp_port              = BUNNY_DISCOVERY_UDP_PORT,
        .broadcast_interval_ms = BUNNY_DISCOVERY_BROADCAST_INTERVAL_MS,
    },
    .webhook = {
        .port = BUNNY_WEBHOOK_PORT,
        .path = BUNNY_WEBHOOK_PATH,
    },
    .wifi = {
        .ssid        = BUNNY_WIFI_SSID,
        .password    = BUNNY_WIFI_PASSWORD,
        .auth_type   = BUNNY_WIFI_AUTH_TYPE,
        .max_retries = BUNNY_WIFI_MAX_RETRIES,
        .timeout_ms  = BUNNY_WIFI_TIMEOUT_MS,
    },
};

/* Buffers estáticos para almacenar strings del JSON */
static char s_wifi_ssid[32];
static char s_wifi_password[64];
static char s_wifi_auth_type[16];
static char s_device_id[32];
static char s_device_name[64];
static char s_device_description[256];
static char s_device_type[16];
static char s_device_version[16];
static char s_webhook_path[64];

void bunny_config_load(void)
{
    /* TODO: parsear config/device.json desde SPIFFS y rellenar s_config */
    /* Por ahora, usa los valores por defecto cargados en build-time desde config/device.json */
    ESP_LOGI(TAG,
             "Config loaded (id=%s name=%s ssid=%s)",
             s_config.device.id,
             s_config.device.name,
             s_config.wifi.ssid);
}

const bunny_config_t *bunny_config_get(void)
{
    return &s_config;
}
