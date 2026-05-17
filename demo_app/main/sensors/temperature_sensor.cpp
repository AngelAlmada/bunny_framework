#include "temperature_sensor.h"
#include "bunny_sdk.h"

/**
 * Temperature Sensor — example capability module.
 *
 * Declares a sensor that reports temperature in Celsius.
 * The read hook reads the hardware ADC; no logic here.
 */

static double read_temperature_hw() {
    // TODO: replace with real ADC / I2C sensor read
    return 23.5;
}

void register_temperature_sensor() {
    Bunny.sensor("temperature")
         .description("Ambient temperature in degrees Celsius")
         .returns(NUMBER)
         .tag("environment")
         .tag("climate")
         .example("Read: 23.5")
         .build([]() -> double {
             return read_temperature_hw();
         });
}
