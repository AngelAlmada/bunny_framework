#include "discovery.h"
#include "../config/config.h"
#include "esp_log.h"

#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const char* TAG = "bunny_discovery";
static bool s_started = false;
static int s_sock = -1;
static TaskHandle_t s_task = NULL;
static uint32_t s_announce_count = 0;

static bool get_local_ip(char* out_ip, size_t out_len)
{
    esp_netif_ip_info_t ip_info;
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (!netif) {
        netif = esp_netif_get_handle_from_ifkey("ETH_DEF");
    }
    if (!netif) {
        return false;
    }

    if (esp_netif_get_ip_info(netif, &ip_info) != ESP_OK) {
        return false;
    }

    snprintf(out_ip, out_len, IPSTR, IP2STR(&ip_info.ip));
    return true;
}

static void discovery_task(void* arg)
{
    (void)arg;
    const bunny_config_t* cfg = bunny_config_get();
    if (!cfg) {
        ESP_LOGE(TAG, "Config unavailable in discovery task");
        s_task = NULL;
        vTaskDelete(NULL);
        return;
    }

    struct sockaddr_in dest_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(cfg->discovery.udp_port),
        .sin_addr.s_addr = inet_addr("255.255.255.255"),
    };

    while (s_started) {
        char ip_buf[32] = "0.0.0.0";
        (void)get_local_ip(ip_buf, sizeof(ip_buf));

        char payload[384];
        int written = snprintf(payload, sizeof(payload),
                               "{\"bunny\":true,\"id\":\"%s\",\"name\":\"%s\",\"version\":\"%s\",\"ip\":\"%s\",\"webhook_port\":%u,\"webhook_path\":\"%s\"}",
                               cfg->device.id,
                               cfg->device.name,
                               cfg->device.version,
                               ip_buf,
                               (unsigned)cfg->webhook.port,
                               cfg->webhook.path);
        if (written < 0) {
            ESP_LOGE(TAG, "Failed building UDP payload");
        } else {
            int sent = sendto(s_sock, payload, (size_t)written, 0,
                              (struct sockaddr*)&dest_addr, sizeof(dest_addr));
            if (sent < 0) {
                ESP_LOGW(TAG, "UDP announce send failed");
            } else {
                s_announce_count++;
                ESP_LOGI(TAG, "FLOW[1/4] UDP announce sent #%lu -> %s:%u",
                         (unsigned long)s_announce_count,
                         "255.255.255.255",
                         (unsigned)cfg->discovery.udp_port);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(cfg->discovery.broadcast_interval_ms));
    }

    s_task = NULL;
    vTaskDelete(NULL);
}

void bunny_discovery_init(void)
{
    const bunny_config_t* cfg = bunny_config_get();
    if (!cfg) {
        ESP_LOGE(TAG, "Config not available; UDP discovery init aborted");
        return;
    }

    ESP_LOGI(TAG,
             "UDP discovery init: enabled=%s port=%u interval_ms=%lu",
             cfg->discovery.enabled ? "true" : "false",
             (unsigned)cfg->discovery.udp_port,
             (unsigned long)cfg->discovery.broadcast_interval_ms);
}

void bunny_discovery_start(void)
{
    const bunny_config_t* cfg = bunny_config_get();
    if (!cfg) {
        ESP_LOGE(TAG, "Config not available; UDP discovery start aborted");
        return;
    }

    if (!cfg->discovery.enabled) {
        ESP_LOGW(TAG, "UDP discovery disabled in config");
        return;
    }

    if (s_started) {
        ESP_LOGW(TAG, "UDP discovery already running");
        return;
    }

    s_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (s_sock < 0) {
        ESP_LOGE(TAG, "Failed to create UDP socket");
        return;
    }

    int broadcast = 1;
    if (setsockopt(s_sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) != 0) {
        ESP_LOGE(TAG, "Failed to enable UDP broadcast");
        close(s_sock);
        s_sock = -1;
        return;
    }

    s_started = true;
    s_announce_count = 0;
    BaseType_t ok = xTaskCreate(discovery_task, "bunny_udp_discovery", 4096, NULL, 5, &s_task);
    if (ok != pdPASS) {
        ESP_LOGE(TAG, "Failed to create discovery task");
        s_started = false;
        close(s_sock);
        s_sock = -1;
        return;
    }

    ESP_LOGI(TAG, "UDP discovery started (real broadcast mode)");
}

void bunny_discovery_stop(void)
{
    if (!s_started) return;
    s_started = false;

    if (s_sock >= 0) {
        close(s_sock);
        s_sock = -1;
    }

    ESP_LOGI(TAG, "UDP discovery stopped");
}

uint32_t bunny_discovery_announce_count(void)
{
    return s_announce_count;
}
