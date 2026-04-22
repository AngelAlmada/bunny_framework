#include "config.h"
#include <string.h>
#include <esp_log.h>

static const char *TAG = "bunny_config";

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
    ESP_LOGI(TAG, "Config loaded (SSID=%s)", s_config.wifi.ssid);
}

const bunny_config_t *bunny_config_get(void)
{
    return &s_config;
}
