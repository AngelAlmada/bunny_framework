#include "bunny_gpio.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <unordered_map>
#include <map>
#include <string>

namespace bunny {
namespace gpio {

static const char* TAG = "bunny_gpio";

// ── State mappings ───────────────────────────────────────────────────────────
static std::unordered_map<int, int> s_pin_to_channel;

// ── Digital I/O ──────────────────────────────────────────────────────────────

bool configure(int pin, Mode mode, const char* owner_tag) {
    if (pin < 0) return false;

    // Intentamos reservar el pin en el registro global para evitar colisiones
    if (!PinRegistry::instance().reserve(pin, owner_tag)) {
        ESP_LOGE(TAG, "Hardware Conflict: Pin %d already reserved by '%s'", 
                 pin, PinRegistry::instance().get_owner(pin).c_str());
        return false;
    }

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
        PinRegistry::instance().release(pin); // Liberamos la reserva si falla
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

// ── PWM (Control de intensidad, LEDC) ────────────────────────────────────────

bool pwm_configure(int pin, int channel, int frequency_hz, int resolution_bits) {
    if (pin < 0 || channel < 0 || channel >= LEDC_CHANNEL_MAX) return false;

    // Reservamos el pin
    std::string tag = "pwm_channel_" + std::to_string(channel);
    if (!PinRegistry::instance().reserve(pin, tag)) {
        ESP_LOGE(TAG, "PWM Hardware Conflict: Pin %d already reserved by '%s'", 
                 pin, PinRegistry::instance().get_owner(pin).c_str());
        return false;
    }

    // Configurar temporizador LEDC
    ledc_timer_config_t ledc_timer = {};
    ledc_timer.speed_mode       = LEDC_LOW_SPEED_MODE;
    ledc_timer.timer_num        = LEDC_TIMER_0;
    ledc_timer.duty_resolution  = (ledc_timer_bit_t)resolution_bits;
    ledc_timer.freq_hz          = frequency_hz;
    ledc_timer.clk_cfg          = LEDC_AUTO_CLK;

    esp_err_t err = ledc_timer_config(&ledc_timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_timer_config failed: %s", esp_err_to_name(err));
        PinRegistry::instance().release(pin);
        return false;
    }

    // Configurar canal LEDC
    ledc_channel_config_t ledc_channel = {};
    ledc_channel.speed_mode     = LEDC_LOW_SPEED_MODE;
    ledc_channel.channel        = (ledc_channel_t)channel;
    ledc_channel.timer_sel      = LEDC_TIMER_0;
    ledc_channel.intr_type      = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num       = pin;
    ledc_channel.duty           = 0;
    ledc_channel.hpoint         = 0;

    err = ledc_channel_config(&ledc_channel);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_channel_config failed: %s", esp_err_to_name(err));
        PinRegistry::instance().release(pin);
        return false;
    }

    // Guardar mapeo pin -> canal
    s_pin_to_channel[pin] = channel;
    return true;
}

void pwm_write(int pin, int duty_cycle) {
    auto it = s_pin_to_channel.find(pin);
    if (it != s_pin_to_channel.end()) {
        int channel = it->second;
        ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)channel, duty_cycle);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)channel);
    } else {
        ESP_LOGW(TAG, "pwm_write: Pin %d not configured for PWM", pin);
    }
}

// ── ADC (Lectura analógica, ADC1) ──────────────────────────────────────────

static adc_oneshot_unit_handle_t s_adc1_handle = nullptr;

static bool init_adc1_unit() {
    if (s_adc1_handle != nullptr) return true;

    adc_oneshot_unit_init_cfg_t init_config = {};
    init_config.unit_id = ADC_UNIT_1;

    esp_err_t err = adc_oneshot_new_unit(&init_config, &s_adc1_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init ADC1 unit: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

static bool get_adc1_channel(int pin, adc_channel_t& chan) {
    switch (pin) {
        case 36: chan = ADC_CHANNEL_0; return true;
        case 37: chan = ADC_CHANNEL_1; return true;
        case 38: chan = ADC_CHANNEL_2; return true;
        case 39: chan = ADC_CHANNEL_3; return true;
        case 32: chan = ADC_CHANNEL_4; return true;
        case 33: chan = ADC_CHANNEL_5; return true;
        case 34: chan = ADC_CHANNEL_6; return true;
        case 35: chan = ADC_CHANNEL_7; return true;
        default: return false; // Pin no es ADC1 en ESP32
    }
}

bool adc_configure(int pin) {
    if (pin < 0) return false;

    adc_channel_t chan;
    if (!get_adc1_channel(pin, chan)) {
        ESP_LOGE(TAG, "GPIO %d is not a valid ADC1 pin on ESP32", pin);
        return false;
    }

    if (!PinRegistry::instance().reserve(pin, "adc_sensor")) {
        ESP_LOGE(TAG, "ADC Hardware Conflict: Pin %d already reserved by '%s'", 
                 pin, PinRegistry::instance().get_owner(pin).c_str());
        return false;
    }

    if (!init_adc1_unit()) {
        PinRegistry::instance().release(pin);
        return false;
    }

    // Configuración del canal a 12 bits de ancho y 12dB de atenuación (0-3.3V)
    adc_oneshot_chan_cfg_t config = {};
    config.bitwidth = ADC_BITWIDTH_12;
    config.atten = ADC_ATTEN_DB_12;

    esp_err_t err = adc_oneshot_config_channel(s_adc1_handle, chan, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "adc_oneshot_config_channel failed: %s", esp_err_to_name(err));
        PinRegistry::instance().release(pin);
        return false;
    }

    return true;
}

int adc_read_raw(int pin) {
    adc_channel_t chan;
    if (s_adc1_handle == nullptr || !get_adc1_channel(pin, chan)) {
        return 0;
    }

    int raw_val = 0;
    esp_err_t err = adc_oneshot_read(s_adc1_handle, chan, &raw_val);
    if (err != ESP_OK) {
        return 0;
    }
    return raw_val;
}

double adc_read_voltage(int pin) {
    int raw = adc_read_raw(pin);
    // Resolución de 12 bits = 4095, rango de voltaje calibrado hasta 3.3V
    return (raw / 4095.0) * 3.3;
}

// ── Interrupciones asíncronas con Debounce (ISR -> Queue -> Task) ────────────

struct GpioEvent {
    int pin;
    uint32_t timestamp_ms;
};

static QueueHandle_t s_gpio_evt_queue = nullptr;
static std::map<int, InterruptCallback> s_interrupt_callbacks;
static std::map<int, int> s_debounce_times;
static std::map<int, uint32_t> s_last_trigger_times;
static bool s_isr_service_installed = false;

static void gpio_dispatch_task(void* arg) {
    GpioEvent evt;
    while (true) {
        if (xQueueReceive(s_gpio_evt_queue, &evt, portMAX_DELAY)) {
            uint32_t last_time = s_last_trigger_times[evt.pin];
            int debounce_ms = s_debounce_times[evt.pin];

            // Comprobación segura de rebote (debounce por software)
            if (evt.timestamp_ms - last_time >= (uint32_t)debounce_ms) {
                s_last_trigger_times[evt.pin] = evt.timestamp_ms;

                auto it = s_interrupt_callbacks.find(evt.pin);
                if (it != s_interrupt_callbacks.end() && it->second) {
                    it->second(); // Se ejecuta de forma 100% segura en esta tarea
                }
            }
        }
    }
}

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    int pin = (int)(intptr_t)arg;
    uint32_t now_ms = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;

    GpioEvent evt = { pin, now_ms };
    xQueueSendFromISR(s_gpio_evt_queue, &evt, NULL);
}

static bool init_isr_service() {
    if (s_isr_service_installed) return true;

    s_gpio_evt_queue = xQueueCreate(16, sizeof(GpioEvent));
    if (s_gpio_evt_queue == nullptr) return false;

    // Creamos la tarea asíncrona despachadora
    BaseType_t task_err = xTaskCreate(gpio_dispatch_task, "bunny_gpio_task", 4096, NULL, 5, NULL);
    if (task_err != pdPASS) {
        vQueueDelete(s_gpio_evt_queue);
        s_gpio_evt_queue = nullptr;
        return false;
    }

    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return false;
    }

    s_isr_service_installed = true;
    return true;
}

bool attach_interrupt(int pin, Edge edge, InterruptCallback callback, int debounce_ms) {
    if (pin < 0 || !callback) return false;

    if (!init_isr_service()) return false;

    gpio_int_type_t intr_type = GPIO_INTR_DISABLE;
    switch (edge) {
        case Edge::RISING:  intr_type = GPIO_INTR_POSEDGE; break;
        case Edge::FALLING: intr_type = GPIO_INTR_NEGEDGE; break;
        case Edge::ANY:     intr_type = GPIO_INTR_ANYEDGE; break;
    }

    esp_err_t err = gpio_set_intr_type((gpio_num_t)pin, intr_type);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set interrupt type for pin %d: %s", pin, esp_err_to_name(err));
        return false;
    }

    s_interrupt_callbacks[pin] = std::move(callback);
    s_debounce_times[pin] = debounce_ms;
    s_last_trigger_times[pin] = 0;

    err = gpio_isr_handler_add((gpio_num_t)pin, gpio_isr_handler, (void*)(intptr_t)pin);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add ISR handler for pin %d: %s", pin, esp_err_to_name(err));
        return false;
    }

    return true;
}

void detach_interrupt(int pin) {
    if (pin < 0) return;

    gpio_isr_handler_remove((gpio_num_t)pin);
    gpio_set_intr_type((gpio_num_t)pin, GPIO_INTR_DISABLE);

    s_interrupt_callbacks.erase(pin);
    s_debounce_times.erase(pin);
    s_last_trigger_times.erase(pin);
}

} // namespace gpio
} // namespace bunny
