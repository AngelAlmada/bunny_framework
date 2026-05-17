#include "wifi.h"
#include "../config/config.h"
#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <string.h>

static const char *TAG = "bunny_wifi";

/* WiFi event group bits */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static EventGroupHandle_t s_wifi_event_group = NULL;
static esp_netif_t *s_sta_netif = NULL;
static int s_retry_num = 0;
static const bunny_wifi_config_t *s_wifi_config = NULL;
static bool s_wifi_initialized = false;  /* Guard against double initialization */

/**
 * WiFi event handler
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Starting WiFi connection...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < s_wifi_config->max_retries) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry WiFi connection (%d/%d)...", s_retry_num, s_wifi_config->max_retries);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Failed to connect to WiFi after %d retries", s_wifi_config->max_retries);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

bool bunny_wifi_init(void)
{
    /* Guard against double initialization */
    if (s_wifi_initialized) {
        ESP_LOGD(TAG, "WiFi already initialized, skipping reinit");
        return true;
    }

    /* Get WiFi config */
    const bunny_config_t *config = bunny_config_get();
    s_wifi_config = &config->wifi;

    /* Check if WiFi is configured */
    if (!s_wifi_config->ssid || strlen(s_wifi_config->ssid) == 0) {
        ESP_LOGW(TAG, "WiFi not configured (empty SSID)");
        return false;
    }

    /* Create event group */
    if (s_wifi_event_group == NULL) {
        s_wifi_event_group = xEventGroupCreate();
    }

    /* NVS is required by WiFi driver. */
    esp_err_t nvs_ret = nvs_flash_init();
    if (nvs_ret == ESP_ERR_NVS_NO_FREE_PAGES || nvs_ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS needs erase, reinitializing");
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_ret);

    /* Initialize TCP/IP stack */
    esp_netif_init();

    /* Create event loop if not already created */
    esp_event_loop_create_default();

    /* Create WiFi STA network interface */
    s_sta_netif = esp_netif_create_default_wifi_sta();

    /* WiFi configuration */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    /* Register event handlers */
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    /* Set WiFi mode and credentials */
    esp_wifi_set_mode(WIFI_MODE_STA);

    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.sta.ssid, s_wifi_config->ssid, 
            sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, s_wifi_config->password, 
            sizeof(wifi_config.sta.password) - 1);

    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    s_wifi_initialized = true;
    ESP_LOGI(TAG, "WiFi initialized (SSID: %s)", s_wifi_config->ssid);
    return true;
}

bool bunny_wifi_connected(void)
{
    if (s_wifi_event_group == NULL) {
        return false;
    }

    EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}

const char* bunny_wifi_get_ip(void)
{
    if (!bunny_wifi_connected() || s_sta_netif == NULL) {
        return NULL;
    }

    static char ip_str[16];
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(s_sta_netif, &ip_info);
    
    snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&ip_info.ip));
    return ip_str;
}

bool bunny_wifi_connect_wait(uint32_t timeout_ms)
{
    if (!bunny_wifi_init()) {
        return false;
    }

    /* Wait for connection or timeout */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                          WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                          pdFALSE,
                                          pdFALSE,
                                          pdMS_TO_TICKS(timeout_ms));

    return (bits & WIFI_CONNECTED_BIT) != 0;
}

void bunny_wifi_stop(void)
{
    if (s_sta_netif != NULL) {
        esp_wifi_stop();
        esp_netif_destroy(s_sta_netif);
        s_sta_netif = NULL;
    }

    if (s_wifi_event_group != NULL) {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
    }
}
