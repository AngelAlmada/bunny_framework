#include "network.h"
#include "wifi.h"
#include "../config/config.h"

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_err.h"

#include <stdlib.h>
#include <string.h>

static const char* TAG = "bunny_network";

static httpd_handle_t s_server = NULL;
static bool s_ws_connected = false;
static char s_ws_path[64] = "/bunny";

static esp_err_t ws_handler(httpd_req_t* req)
{
    if (req->method == HTTP_GET) {
        s_ws_connected = true;
        ESP_LOGI(TAG, "WebSocket handshake completed from %s", req->uri);
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt = {
        .final = true,
        .fragmented = false,
        .len = 0,
        .type = HTTPD_WS_TYPE_TEXT,
        .payload = NULL,
    };

    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WebSocket read length failed: %s", esp_err_to_name(ret));
        return ret;
    }

    if (ws_pkt.len > 0) {
        uint8_t* buf = (uint8_t*)malloc(ws_pkt.len + 1);
        if (!buf) {
            ESP_LOGE(TAG, "No memory for WebSocket frame");
            return ESP_ERR_NO_MEM;
        }

        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret == ESP_OK) {
            buf[ws_pkt.len] = '\0';
            ESP_LOGI(TAG, "WebSocket message received: %s", (char*)buf);
        } else {
            ESP_LOGE(TAG, "WebSocket read failed: %s", esp_err_to_name(ret));
        }

        free(buf);
    }

    return ret;
}

void bunny_network_init(void)
{
    const bunny_config_t* cfg = bunny_config_get();
    if (cfg && cfg->webhook.path) {
        strncpy(s_ws_path, cfg->webhook.path, sizeof(s_ws_path) - 1);
        s_ws_path[sizeof(s_ws_path) - 1] = '\0';
    }

    ESP_LOGI(TAG, "Network initialized (websocket path=%s)", s_ws_path);
}

void bunny_network_connect(void)
{
    if (s_server) {
        ESP_LOGW(TAG, "WebSocket server already running");
        return;
    }

    const bunny_config_t* cfg = bunny_config_get();
    httpd_config_t http_cfg = HTTPD_DEFAULT_CONFIG();
    if (cfg) {
        http_cfg.server_port = cfg->webhook.port;
    }

    esp_err_t err = httpd_start(&s_server, &http_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WebSocket server: %s", esp_err_to_name(err));
        s_server = NULL;
        return;
    }

    httpd_uri_t ws_uri = {
        .uri = s_ws_path,
        .method = HTTP_GET,
        .handler = ws_handler,
        .user_ctx = NULL,
        .is_websocket = true,
        .handle_ws_control_frames = true,
        .supported_subprotocol = NULL,
    };

    err = httpd_register_uri_handler(s_server, &ws_uri);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register WebSocket URI: %s", esp_err_to_name(err));
        httpd_stop(s_server);
        s_server = NULL;
        return;
    }

    ESP_LOGI(TAG, "WebSocket server listening on port %u path %s",
             (unsigned)http_cfg.server_port, s_ws_path);
}

bool bunny_network_ws_connected(void)
{
    return s_ws_connected;
}
