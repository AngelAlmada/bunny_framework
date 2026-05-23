#include "display.h"
#include "display_config.h"
#include "bunny.h"
#include "esp_log.h"
#include "bunny_gpio.h"
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
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
// LOVYANGFX CONFIGURATION
// ─────────────────────────────────────────────────────────────────────────────

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI      _bus_instance;

public:
    LGFX(void)
    {
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI3_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = DISPLAY_SPI_FREQ;
            cfg.freq_read  = 16000000;
            cfg.spi_3wire  = true;
            cfg.use_lock   = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            cfg.pin_sclk = DISPLAY_PIN_SCLK;
            cfg.pin_mosi = DISPLAY_PIN_MOSI;
            cfg.pin_miso = DISPLAY_PIN_MISO;
            cfg.pin_dc   = DISPLAY_PIN_DC;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = DISPLAY_PIN_CS;
            cfg.pin_rst          = DISPLAY_PIN_RST;
            cfg.pin_busy         = -1;
            cfg.memory_width     = DISPLAY_WIDTH;
            cfg.memory_height    = DISPLAY_HEIGHT;
            cfg.panel_width      = DISPLAY_WIDTH;
            cfg.panel_height     = DISPLAY_HEIGHT;
            cfg.offset_x         = 0;
            cfg.offset_y         = 0;
            cfg.offset_rotation  = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits  = 1;
            cfg.readable         = true;
            cfg.invert           = true;
            cfg.rgb_order        = false;
            cfg.dlen_16bit       = false;
            cfg.bus_shared       = true;
            _panel_instance.config(cfg);
        }
        setPanel(&_panel_instance);
    }
};

static LGFX lcd;

// ─────────────────────────────────────────────────────────────────────────────
// PUBLIC: HARDWARE INITIALIZATION
// ─────────────────────────────────────────────────────────────────────────────

void init_hardware() {
    ESP_LOGI(TAG, "Initializing hardware...");
    
    // Configure GPIO for backlight
    bunny::gpio::configure(DISPLAY_PIN_BL, bunny::gpio::Mode::OUTPUT);
    bunny::gpio::write(DISPLAY_PIN_BL, 1);
    
    // Initialize LovyanGFX
    lcd.init();
    lcd.setRotation(1); // Landscape
    lcd.fillScreen(lcd.color565(0, 0, 0));
    lcd.setTextColor(lcd.color565(255, 255, 255));
    lcd.setTextSize(2);
    
    // Draw initial text
    lcd.setCursor(10, 10);
    lcd.print("Bunny Display Ready");
    
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
        bunny::gpio::write(DISPLAY_PIN_BL, level);
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
        bunny::gpio::write(DISPLAY_PIN_BL, level > 128 ? 1 : 0);
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
    lcd.fillScreen(color);
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
    lcd.setCursor(x, y);
    lcd.print(text);
}

void cmd_print_text(const Params& params) {
    const char* text = params.get_string("text");
    if (text == nullptr) {
        ESP_LOGW(TAG, "printText: missing 'text' parameter");
        return;
    }

    int size = params.has("size") ? (int)params.get_number("size") : 2;
    if (size < 1) size = 1;
    if (size > 5) size = 5;

    bool clear = params.has("clear") ? (params.get_number("clear") != 0) : true;

    // Safety length limit: 300 characters max
    char safe_text[301];
    strncpy(safe_text, text, 300);
    safe_text[300] = '\0';

    set_last_text(safe_text);
    ESP_LOGI(TAG, "Print wrapped text: '%s' (size=%d, clear=%d)", safe_text, size, clear);

    if (clear) {
        lcd.fillScreen(lcd.color565(0, 0, 0));
    }

    lcd.setTextWrap(true);
    lcd.setTextColor(lcd.color565(255, 255, 255), lcd.color565(0, 0, 0));
    lcd.setTextSize(size);
    lcd.setCursor(10, 10);
    lcd.print(safe_text);
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
    lcd.fillScreen(DISPLAY_COLOR_BLACK);
}

} // namespace display
} // namespace bunny

