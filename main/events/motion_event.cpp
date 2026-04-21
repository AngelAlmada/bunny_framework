#include "motion_event.h"
#include "bunny_sdk.h"

/**
 * Motion Event — example capability module.
 *
 * Declares an event that fires when the PIR sensor detects movement.
 * Triggering is done via Bunny.emit("motion_detected") from your ISR/task.
 *
 * The optional on_emit hook here blinks an indicator LED — hardware only.
 */

static void blink_indicator_hw() {
    // TODO: gpio_set_level(LED_PIN, 1); vTaskDelay(50ms); gpio_set_level(LED_PIN, 0);
}

void register_motion_event() {
    Bunny.event("motion_detected")
         .description("Triggered when the PIR sensor detects movement")
         .tag("sensor")
         .tag("security")
         .build([]() {
             blink_indicator_hw();
         });
}
