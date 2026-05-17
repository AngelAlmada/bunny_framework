#include "display.h"
#include "display_config.h"
#include "bunny.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <cstring>
#include <cstdio>

static const char *TAG = "[display]";

namespace bunny {
namespace display {

// ─────────────────────────────────────────────────────────────────────────────
// INTERNAL STATE VARIABLES (encapsulated)
// ─────────────────────────────────────────────────────────────────────────────

static const char* g_display_status = "ON";
static int g_brightness = 255;
static char g_last_text[128] = "Bunny Display Ready";

// ─────────────────────────────────────────────────────────────────────────────
// PUBLIC: HARDWARE INITIALIZATION
// ─────────────────────────────────────────────────────────────────────────────

void init_hardware() {
    ESP_LOGI(TAG, "Initializing hardware...");
    
    // Configure GPIO for backlight
    gpio_config_t bl_config = {
        .pin_bit_mask = (1ULL << DISPLAY_PIN_BL),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    ESP_ERROR_CHECK(gpio_config(&bl_config));
    ESP_ERROR_CHECK(gpio_set_level(DISPLAY_PIN_BL, 1));
    
    // TODO: Initialize SPI bus
    // TODO: Configure ESP-IDF panel drivers
    // For now, just logging
    
    ESP_LOGI(TAG, "Hardware initialized successfully");
    ESP_LOGI(TAG, "MOSI: GPIO%d, SCLK: GPIO%d, CS: GPIO%d, DC: GPIO%d, RST: GPIO%d, BL: GPIO%d",
             DISPLAY_PIN_MOSI, DISPLAY_PIN_SCLK, DISPLAY_PIN_CS, 
             DISPLAY_PIN_DC, DISPLAY_PIN_RST, DISPLAY_PIN_BL);
}

// ─────────────────────────────────────────────────────────────────────────────
// PUBLIC: STATE CAPABILITY HANDLERS
// ─────────────────────────────────────────────────────────────────────────────

const char* get_display_status() {
    return g_display_status;
}

void set_display_status(const char* value) {
    if (value == nullptr) return;
    
    if (strcmp(value, "ON") == 0 || strcmp(value, "OFF") == 0) {
        g_display_status = value;
        ESP_LOGI(TAG, "Display status: %s", g_display_status);
        
        // Control backlight
        int level = (strcmp(value, "ON") == 0) ? 1 : 0;
        gpio_set_level(DISPLAY_PIN_BL, level);
    }
}

const char* get_brightness() {
    static char buf[10];
    snprintf(buf, sizeof(buf), "%d", g_brightness);
    return buf;
}

void set_brightness(const char* value) {
    if (value == nullptr) return;
    
    int level = atoi(value);
    if (level >= 0 && level <= 255) {
        g_brightness = level;
        ESP_LOGI(TAG, "Brightness: %d", g_brightness);
        
        // Simple PWM: > 128 = on, <= 128 = off
        // TODO: Implement real PWM
        gpio_set_level(DISPLAY_PIN_BL, level > 128 ? 1 : 0);
    }
}

const char* get_last_text() {
    return g_last_text;
}

void set_last_text(const char* value) {
    if (value == nullptr) return;
    
    strncpy(g_last_text, value, sizeof(g_last_text) - 1);
    g_last_text[sizeof(g_last_text) - 1] = '\0';
    ESP_LOGI(TAG, "Last text: %s", g_last_text);
}

// ─────────────────────────────────────────────────────────────────────────────
// PUBLIC: COMMAND CAPABILITY HANDLERS
// ─────────────────────────────────────────────────────────────────────────────

void cmd_fill_screen(const Params& params) {
    const char* color_name = params.get_string("color");
    if (color_name == nullptr) {
        ESP_LOGW(TAG, "fillScreen: missing 'color' parameter");
        return;
    }

    uint16_t color = DISPLAY_COLOR_BLACK;
    
    if (strcmp(color_name, "white") == 0)       color = DISPLAY_COLOR_WHITE;
    else if (strcmp(color_name, "red") == 0)    color = DISPLAY_COLOR_RED;
    else if (strcmp(color_name, "green") == 0)  color = DISPLAY_COLOR_GREEN;
    else if (strcmp(color_name, "blue") == 0)   color = DISPLAY_COLOR_CYAN;
    else if (strcmp(color_name, "yellow") == 0) color = DISPLAY_COLOR_YELLOW;
    else if (strcmp(color_name, "cyan") == 0)   color = DISPLAY_COLOR_CYAN;
    else if (strcmp(color_name, "magenta") == 0) color = DISPLAY_COLOR_MAGENTA;
    else if (strcmp(color_name, "black") != 0)  {
        ESP_LOGW(TAG, "fillScreen: unknown color '%s'", color_name);
        return;
    }

    ESP_LOGI(TAG, "Fill screen with color: %s (0x%04X)", color_name, color);
    
    // TODO: Implement actual screen fill
}

void cmd_draw_text(const Params& params) {
    const char* text = params.get_string("text");
    if (text == nullptr) {
        ESP_LOGW(TAG, "drawText: missing 'text' parameter");
        return;
    }

    int x = (int)params.get_number("x");
    int y = (int)params.get_number("y");

    set_last_text(text);
    ESP_LOGI(TAG, "Draw text at (%d, %d): '%s'", x, y, text);
    
    // TODO: Implement actual text rendering
}

void cmd_set_brightness(const Params& params) {
    int level = (int)params.get_number("level");
    
    if (level < 0 || level > 255) {
        ESP_LOGW(TAG, "setBrightness: level must be 0-255, got %d", level);
        return;
    }

    char buf[10];
    snprintf(buf, sizeof(buf), "%d", level);
    set_brightness(buf);
}

void cmd_clear_screen(const Params& params) {
    (void)params;  // unused
    ESP_LOGI(TAG, "Clear screen");
    
    // TODO: Implement actual screen clear
}

} // namespace display
} // namespace bunny

