#include "network.h"
#include "wifi.h"
#include "../config/config.h"

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_err.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char* TAG = "bunny_network";
static const char* BCP_PROTOCOL_VERSION = "0.1.0";

static httpd_handle_t s_server = NULL;
static bool s_ws_connected = false;
static char s_ws_path[64] = "/bunny";
static int s_active_client_fd = -1;
static bool s_app_handshake_ok = false;
static char s_active_engine_id[64] = "";

static void reset_active_session(const char* reason)
{
    if (reason) {
        ESP_LOGW(TAG, "Resetting active session: %s", reason);
    }
    s_ws_connected = false;
    s_app_handshake_ok = false;
    s_active_client_fd = -1;
    s_active_engine_id[0] = '\0';
}

static const char* skip_ws(const char* p)
{
    while (p && *p && isspace((unsigned char)*p)) {
        ++p;
    }
    return p;
}

static bool json_extract_string(const char* json, const char* key, char* out, size_t out_len)
{
    if (!json || !key || !out || out_len == 0) {
        return false;
    }

    char pattern[64];
    int n = snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    if (n <= 0 || (size_t)n >= sizeof(pattern)) {
        return false;
    }

    const char* p = strstr(json, pattern);
    if (!p) {
        return false;
    }
    p += strlen(pattern);
    p = skip_ws(p);
    if (!p || *p != ':') {
        return false;
    }
    p++;
    p = skip_ws(p);
    if (!p || *p != '"') {
        return false;
    }
    p++;

    size_t i = 0;
    while (*p && *p != '"' && i + 1 < out_len) {
        if (*p == '\\' && *(p + 1) != '\0') {
            ++p;
        }
        out[i++] = *p++;
    }
    out[i] = '\0';
    return (*p == '"' && i > 0);
}

static bool json_has_object_key(const char* json, const char* key)
{
    if (!json || !key) {
        return false;
    }

    char pattern[64];
    int n = snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    if (n <= 0 || (size_t)n >= sizeof(pattern)) {
        return false;
    }

    const char* p = strstr(json, pattern);
    if (!p) {
        return false;
    }
    p += strlen(pattern);
    p = skip_ws(p);
    if (!p || *p != ':') {
        return false;
    }
    p++;
    p = skip_ws(p);
    return (p && *p == '{');
}

static esp_err_t ws_send_text(httpd_req_t* req, const char* text)
{
    httpd_ws_frame_t frame = {
        .final = true,
        .fragmented = false,
        .type = HTTPD_WS_TYPE_TEXT,
        .payload = (uint8_t*)text,
        .len = strlen(text),
    };
    return httpd_ws_send_frame(req, &frame);
}

static esp_err_t ws_send_handshake_error(httpd_req_t* req,
                                         const char* error_code,
                                         const char* message)
{
    char payload[256];
    snprintf(payload, sizeof(payload),
             "{\"type\":\"handshake_error\",\"error_code\":\"%s\",\"message\":\"%s\"}",
             error_code ? error_code : "HANDSHAKE_ERROR",
             message ? message : "handshake failed");
    return ws_send_text(req, payload);
}

static esp_err_t ws_handle_app_handshake(httpd_req_t* req, const char* payload)
{
    char type[32] = {0};
    char engine_id[64] = {0};
    char protocol_version[32] = {0};

    if (!json_extract_string(payload, "type", type, sizeof(type))) {
        (void)ws_send_handshake_error(req, "MISSING_FIELDS", "missing type");
        return ESP_FAIL;
    }
    if (strcmp(type, "handshake_init") != 0) {
        (void)ws_send_handshake_error(req, "INVALID_TYPE", "expected handshake_init");
        return ESP_FAIL;
    }
    if (!json_extract_string(payload, "engine_id", engine_id, sizeof(engine_id)) ||
        !json_extract_string(payload, "protocol_version", protocol_version, sizeof(protocol_version)) ||
        !json_has_object_key(payload, "capabilities")) {
        (void)ws_send_handshake_error(req, "MISSING_FIELDS",
                                      "required fields: engine_id, protocol_version, capabilities");
        return ESP_FAIL;
    }
    if (strcmp(protocol_version, BCP_PROTOCOL_VERSION) != 0) {
        (void)ws_send_handshake_error(req, "PROTOCOL_MISMATCH", "unsupported protocol_version");
        return ESP_FAIL;
    }

    const bunny_config_t* cfg = bunny_config_get();
    const char* device_id = (cfg && cfg->device.id) ? cfg->device.id : "unknown";

    char ack[256];
    snprintf(ack, sizeof(ack),
             "{\"type\":\"handshake_ack\",\"status\":\"ok\",\"device_id\":\"%s\",\"protocol_version\":\"%s\"}",
             device_id,
             BCP_PROTOCOL_VERSION);

    esp_err_t ret = ws_send_text(req, ack);
    if (ret != ESP_OK) {
        return ret;
    }

    s_app_handshake_ok = true;
    s_ws_connected = true;
    strncpy(s_active_engine_id, engine_id, sizeof(s_active_engine_id) - 1);
    s_active_engine_id[sizeof(s_active_engine_id) - 1] = '\0';
    ESP_LOGI(TAG, "BCP handshake accepted (engine_id=%s protocol_version=%s)",
             s_active_engine_id, BCP_PROTOCOL_VERSION);
    return ESP_OK;
}

static esp_err_t ws_handle_post_handshake_text(httpd_req_t* req, const char* payload)
{
    char type[32] = {0};
    if (!json_extract_string(payload, "type", type, sizeof(type))) {
        ESP_LOGW(TAG, "Ignoring message without type after handshake");
        return ESP_OK;
    }

    if (strcmp(type, "heartbeat") == 0 || strcmp(type, "heartbeat_ping") == 0) {
        char timestamp[64] = {0};
        char correlation_id[64] = {0};
        bool has_timestamp = json_extract_string(payload, "timestamp", timestamp, sizeof(timestamp)) ||
                             json_extract_string(payload, "ts", timestamp, sizeof(timestamp));
        bool has_correlation = json_extract_string(payload, "correlation_id", correlation_id, sizeof(correlation_id));

        char ack[320];
        if (has_correlation) {
            snprintf(ack, sizeof(ack),
                     "{\"type\":\"heartbeat_pong\",\"correlation_id\":\"%s\",\"ts\":\"%s\"}",
                     correlation_id,
                     has_timestamp ? timestamp : "");
        } else {
            snprintf(ack, sizeof(ack),
                     "{\"type\":\"heartbeat_ack\",\"status\":\"ok\",\"timestamp\":\"%s\"}",
                     has_timestamp ? timestamp : "");
        }

        esp_err_t ret = ws_send_text(req, ack);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Heartbeat acknowledged (engine=%s)",
                     s_active_engine_id[0] ? s_active_engine_id : "unknown");
        }
        return ret;
    }

    ESP_LOGI(TAG, "WebSocket message received (engine=%s): %s",
             s_active_engine_id[0] ? s_active_engine_id : "unknown",
             payload);
    return ESP_OK;
}

static esp_err_t ws_handler(httpd_req_t* req)
{
    int client_fd = httpd_req_to_sockfd(req);

    if (req->method == HTTP_GET) {
        if (s_active_client_fd >= 0 && s_active_client_fd != client_fd) {
            ESP_LOGW(TAG, "Rejecting second motor connection (active_fd=%d incoming_fd=%d)",
                     s_active_client_fd, client_fd);
            httpd_resp_set_status(req, "409 Conflict");
            httpd_resp_set_type(req, "application/json");
            httpd_resp_sendstr(req,
                              "{\"error\":\"ENGINE_ALREADY_CONNECTED\",\"message\":\"only one motor can connect at a time\"}");
            return ESP_FAIL;
        }

        if (s_active_client_fd < 0) {
            s_active_client_fd = client_fd;
            s_app_handshake_ok = false;
            s_ws_connected = false;
            s_active_engine_id[0] = '\0';
            ESP_LOGI(TAG, "Reserved WebSocket session for motor fd=%d", client_fd);
        }

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
        if (client_fd == s_active_client_fd) {
            reset_active_session("websocket read length failed");
        }
        ESP_LOGE(TAG, "WebSocket read length failed: %s", esp_err_to_name(ret));
        return ret;
    }

    if (s_active_client_fd >= 0 && client_fd != s_active_client_fd) {
        ESP_LOGW(TAG, "Dropping frame from non-active motor fd=%d", client_fd);
        return ESP_FAIL;
    }

    if (s_active_client_fd < 0) {
        s_active_client_fd = client_fd;
        s_app_handshake_ok = false;
        s_ws_connected = false;
    }

    if (ws_pkt.type == HTTPD_WS_TYPE_CLOSE) {
        reset_active_session("peer closed websocket");
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

            if (ws_pkt.type == HTTPD_WS_TYPE_TEXT) {
                if (!s_app_handshake_ok) {
                    ret = ws_handle_app_handshake(req, (const char*)buf);
                    if (ret != ESP_OK) {
                        reset_active_session("app handshake rejected");
                    }
                } else {
                    ret = ws_handle_post_handshake_text(req, (const char*)buf);
                    if (ret != ESP_OK) {
                        reset_active_session("post-handshake message handling failed");
                    }
                }
            }
        } else {
            if (client_fd == s_active_client_fd) {
                reset_active_session("websocket read failed");
            }
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
