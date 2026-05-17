#include "bunny_gpio.h"
#include "driver/gpio.h"
#include "esp_log.h"

namespace bunny {
namespace gpio {

static const char* TAG = "bunny_gpio";

bool configure(int pin, Mode mode) {
    if (pin < 0) return false;

    gpio_config_t conf = {};
    conf.pin_bit_mask = (1ULL << pin);
    conf.intr_type = GPIO_INTR_DISABLE;

    switch (mode) {
        case Mode::INPUT:
            conf.mode = GPIO_MODE_INPUT;
            conf.pull_up_en = GPIO_PULLUP_DISABLE;
            conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        case Mode::OUTPUT:
            conf.mode = GPIO_MODE_OUTPUT;
            conf.pull_up_en = GPIO_PULLUP_DISABLE;
            conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        case Mode::INPUT_PULLUP:
            conf.mode = GPIO_MODE_INPUT;
            conf.pull_up_en = GPIO_PULLUP_ENABLE;
            conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        case Mode::INPUT_PULLDOWN:
            conf.mode = GPIO_MODE_INPUT;
            conf.pull_up_en = GPIO_PULLUP_DISABLE;
            conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            break;
    }

    esp_err_t err = gpio_config(&conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO %d: %s", pin, esp_err_to_name(err));
        return false;
    }
    return true;
}

void write(int pin, int level) {
    if (pin >= 0) {
        gpio_set_level((gpio_num_t)pin, level);
    }
}

int read(int pin) {
    if (pin >= 0) {
        return gpio_get_level((gpio_num_t)pin);
    }
    return 0;
}

} // namespace gpio
} // namespace bunny
