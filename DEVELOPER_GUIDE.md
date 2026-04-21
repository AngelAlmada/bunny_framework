# Bunny SDK — Guía de Desarrollo

Documento técnico para desarrolladores que extienden o mantienen el framework Bunny.

**← [Volver al README](README.md)**

## Índice

1. [Sistema de Tipos](#sistema-de-tipos)
2. [Metadata Semántica](#metadata-semántica)
3. [Contratos Base de Capacidades](#contratos-base-de-capacidades)
4. [Fluent Builder API](#fluent-builder-api)
5. [Registro Central (Registry)](#registro-central-registry)
6. [Crear un Módulo de Capacidades](#crear-un-módulo-de-capacidades)
7. [Serialización JSON](#serialización-json)
8. [Protocolo de Comunicación](#protocolo-de-comunicación)
9. [Ejemplos Completos](#ejemplos-completos)

---

## Sistema de Tipos

**Archivo**: [`components/bunny/types/bunny_types.h`](components/bunny/types/bunny_types.h)

Bunny define 5 tipos primitivos para parámetros y retornos:

```cpp
enum class Type : uint8_t {
    NUMBER  = 0,
    STRING  = 1,
    BOOLEAN = 2,
    OBJECT  = 3,
    ARRAY   = 4,
    VOID    = 5,
};
```

**Alias globales** (para usar sin namespace en fluent API):

```cpp
static constexpr bunny::Type NUMBER  = bunny::Type::NUMBER;
static constexpr bunny::Type STRING  = bunny::Type::STRING;
// ... etc
```

**Conversión a string**: función `type_name(Type t)` retorna `"number"`, `"string"`, etc.

### Uso

```cpp
Bunny.sensor("value")
     .returns(NUMBER)      // tipo de retorno: numérico
     .build([](){ return 42.0; });

Bunny.command("name")
     .param("input", STRING)  // parámetro de tipo string
     .execute([](const Params& p) { ... });
```

---

## Metadata Semántica

**Archivo**: [`components/bunny/metadata/metadata.h`](components/bunny/metadata/metadata.h)

Metadata es el contrato semántico que permite:
- Validación estructurada.
- Consumo por backend/LLM.
- Generación auto de procesos.

### Estructura

```cpp
struct ParamDef {
    const char* name;          // "state"
    Type        type;          // STRING
    const char* description;   // "ON or OFF"
    bool        required;      // true
};

struct Metadata {
    const char* description;               // "Turn fan ON/OFF"
    Type        returns_type;              // VOID
    ParamDef    params[MAX_PARAMS];        // array de parámetros
    size_t      param_count;               // número de parámetros
    const char* tags[MAX_TAGS];            // ["actuator", "climate"]
    size_t      tag_count;
    const char* affects[MAX_AFFECTS];      // ["fanState"]
    size_t      affects_count;
    const char* example;                   // "{\"state\": \"ON\"}"
};
```

### Constraints

- `MAX_PARAMS = 8` parámetros máximo por capacidad.
- `MAX_TAGS = 8` tags máximo.
- `MAX_AFFECTS = 8` capacidades afectadas máximo.

### Llenado manual (si no usas builders)

```cpp
Metadata meta{};
meta.description = "My sensor";
meta.returns_type = Type::NUMBER;
meta.tags[0] = "environment";
meta.tag_count = 1;
```

---

## Contratos Base de Capacidades

**Archivo**: [`components/bunny/core/capability.h`](components/bunny/core/capability.h)

### ICapability (interfaz)

```cpp
class ICapability {
public:
    virtual ~ICapability() = default;

    virtual CapabilityKind  kind()     const = 0;
    virtual const char*     name()     const = 0;
    virtual const Metadata& metadata() const = 0;
    virtual size_t          serialize(char* buf, size_t len) const = 0;
};
```

Cada capacidad instancia una clase derivada de `ICapability`.

### CapabilityKind

```cpp
enum class CapabilityKind : uint8_t {
    SENSOR  = 0,
    COMMAND = 1,
    EVENT   = 2,
    STATE   = 3,
};
```

Function helper: `const char* capability_kind_name(CapabilityKind k)` → `"sensor"`, `"command"`, etc.

---

## Fluent Builder API

**Archivos**: [`components/bunny/builder/`](components/bunny/builder/)

Bunny proporciona 4 builders correspondientes a los 4 tipos de capacidades.

### 1. SensorBuilder

**Uso**:

```cpp
Bunny.sensor("temperature")
     .description("Room temperature in Celsius")
     .returns(NUMBER)
     .tag("environment")
     .tag("climate")
     .affects("climate_control")
     .example("Read: 23.5°C")
     .build([]() -> double {
         return read_adc_temperature();
     });
```

**Métodos**:
- `description(const char*)` → `SensorBuilder&`
- `returns(Type)` → `SensorBuilder&`
- `tag(const char*)` → `SensorBuilder&` (repetible)
- `affects(const char*)` → `SensorBuilder&` (repetible)
- `example(const char*)` → `SensorBuilder&`
- `build(SensorReadFn)` → `SensorCapability*` (registra automaticamente)

### 2. CommandBuilder

**Uso**:

```cpp
Bunny.command("setFanSpeed")
     .description("Set fan speed 0-100")
     .param("speed", NUMBER, "0-100 percentage", true)
     .param("rampTime", NUMBER, "Ramp time in ms", false)
     .affects("fanSpeed")
     .tag("actuator")
     .example("{\"speed\": 75}")
     .execute([](const Params& p) {
         int speed = (int)p.get_number("speed");
         gpio_set_pwm(FAN_PIN, speed);
     });
```

**Métodos**:
- `description(const char*)` → `CommandBuilder&`
- `param(name, type, description="", required=true)` → `CommandBuilder&` (repetible)
- `tag(const char*)` → `CommandBuilder&` (repetible)
- `affects(const char*)` → `CommandBuilder&` (repetible)
- `returns(Type)` → `CommandBuilder&` (opcional)
- `example(const char*)` → `CommandBuilder&`
- `execute(CommandExecuteFn)` → `CommandCapability*` (registra automáticamente)

### 3. EventBuilder

**Uso**:

```cpp
Bunny.event("motion_detected")
     .description("PIR sensor detected movement")
     .param("intensity", NUMBER, "Movement intensity 0-100")
     .tag("sensor")
     .tag("security")
     .example("{\"intensity\": 85}")
     .build([]() {
         // Hook local opcional: lado-effects de hardware
         gpio_set_level(LED_PIN, 1);
         vTaskDelay(100 / portTICK_PERIOD_MS);
         gpio_set_level(LED_PIN, 0);
     });
```

**Métodos**:
- `description(const char*)` → `EventBuilder&`
- `param(name, type, description="")` → `EventBuilder&` (repetible)
- `tag(const char*)` → `EventBuilder&` (repetible)
- `example(const char*)` → `EventBuilder&`
- `build(EventEmitFn = nullptr)` → `EventCapability*` (registra automáticamente)

### 4. StateBuilder

**Uso**:

```cpp
static const char* s_fan_state = "OFF";

Bunny.state("fanState", STRING)
     .description("Current fan relay state")
     .tag("actuator")
     .example("OFF")
     .build(
         []() -> const char* { return s_fan_state; },
         [](const char* v)   { s_fan_state = v;    }
     );
```

**Métodos**:
- `description(const char*)` → `StateBuilder&`
- `tag(const char*)` → `StateBuilder&` (repetible)
- `example(const char*)` → `StateBuilder&`
- `build(StateGetFn get=nullptr, StateSetFn set=nullptr)` → `StateCapability*` (registra automáticamente)

---

## Registro Central (Registry)

**Archivo**: [`components/bunny/registry/registry.h`](components/bunny/registry/registry.h)

El Registry es un singleton que mantiene todas las capacidades declaradas.

### Interfaz pública

```cpp
class Registry {
public:
    static Registry& instance();

    // Registro (llamado automáticamente por builders)
    bool register_capability(ICapability* cap);

    // Búsqueda
    ICapability* find(const char* name, CapabilityKind kind) const;

    // Acceso directo
    size_t       count() const;
    ICapability* at(size_t i) const;

    // Serialización a JSON
    size_t serialize_capabilities(char* buf, size_t len) const;
};
```

### Uso típico

```cpp
// Usualmente transparente (builders registran automáticamente)
Bunny.sensor("temperature").build([]() { return 25.0; });

// Acceso directo si lo necesitas
auto* sensor = Registry::instance().find("temperature", CapabilityKind::SENSOR);
if (sensor) {
    double value = static_cast<SensorCapability*>(sensor)->read();
}
```

### JSON de salida

```json
{
  "sensors": [
    {
      "name": "temperature",
      "kind": "sensor",
      "description": "Room temperature in Celsius",
      "returns": "number",
      "tags": ["environment", "climate"],
      "affects": ["climate_control"],
      "example": "Read: 23.5°C"
    }
  ],
  "commands": [...],
  "events": [...],
  "states": [...]
}
```

---

## Crear un Módulo de Capacidades

Estructura estándar:

```text
main/
├── sensors/
│   ├── my_sensor.h
│   └── my_sensor.cpp
├── commands/
├── events/
└── states/
```

### Ejemplo: `main/sensors/my_sensor.h`

```cpp
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void register_my_sensor();

#ifdef __cplusplus
}
#endif
```

### Ejemplo: `main/sensors/my_sensor.cpp`

```cpp
#include "my_sensor.h"
#include "bunny_sdk.h"

// Hardware read function
static double read_hardware() {
    // TODO: leer ADC, I2C, SPI, etc.
    return 42.0;
}

void register_my_sensor() {
    Bunny.sensor("my_sensor")
         .description("My awesome sensor data")
         .returns(NUMBER)
         .tag("custom")
         .build([]() -> double {
             return read_hardware();
         });
}
```

### Integración en `main/bunny_framework.c`

```c
#include "bunny.h"
#include "sensors/my_sensor.h"

void app_main(void)
{
    bunny_begin();
    
    // Registrar capacidades
    register_my_sensor();
    // ... otros modulos
    
    bunny_load_modules();
    bunny_loop();
}
```

---

## Serialización JSON

**Archivo**: [`components/bunny/utils/json_builder.h`](components/bunny/utils/json_builder.h)

Helper ligero **sin dependencias externas** para generar JSON en buffers fixed-size (ESP32-friendly).

### Funciones públicas

```cpp
namespace bunny::json {

// Append strings safely within buffer boundaries
inline size_t append(char* buf, size_t len, size_t& pos, const char* str);

// Serialize full Metadata struct to JSON fragment
inline size_t serialize_metadata(char* buf, size_t len, const Metadata& m);

}
```

### Uso

```cpp
char buffer[512];
size_t pos = 0;

json::append(buffer, sizeof(buffer), pos, "{");
json::append(buffer, sizeof(buffer), pos, "\"name\":\"test\"");
json::append(buffer, sizeof(buffer), pos, "}");

// buffer contiene: {"name":"test"}
```

Cada capacidad usa `serialize_metadata()` en su método `serialize()`.

---

## Protocolo de Comunicación

**Archivo**: [`components/bunny/protocol/protocol.h`](components/bunny/protocol/protocol.h) (TODO: implementar)

### Formato esperado de mensajes

**Comando entrante** (backend → ESP32):

```json
{
  "id": "req-001",
  "type": "command",
  "command": "setFanState",
  "params": {
    "state": "ON"
  }
}
```

**Respuesta** (ESP32 → backend):

```json
{
  "id": "req-001",
  "status": "ok",
  "result": null
}
```

**Solicitud de lectura de sensor**:

```json
{
  "id": "req-002",
  "type": "sensor",
  "sensor": "temperature"
}
```

**Respuesta**:

```json
{
  "id": "req-002",
  "status": "ok",
  "value": 23.5
}
```

**Evento emitido por ESP32**:

```json
{
  "event": "motion_detected",
  "timestamp": 1234567890,
  "data": {}
}
```

### Implementación (TODO)

El módulo `protocol` debe:
1. Parsear JSON entrante.
2. Validar contra metadata en Registry.
3. Invocar hooks (execute, read, etc.) con parámetros tipados.
4. Serializar respuestas.

---

## Ejemplos Completos

### Sensor simple

**Archivo**: [`main/sensors/temperature_sensor.cpp`](main/sensors/temperature_sensor.cpp)

```cpp
#include "temperature_sensor.h"
#include "bunny_sdk.h"

static double read_temperature_hw() {
    return 23.5; // TODO: reemplazar con lectura real
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
```

### Comando con parámetros

**Archivo**: [`main/commands/fan_command.cpp`](main/commands/fan_command.cpp)

```cpp
#include "fan_command.h"
#include "bunny_sdk.h"
#include <cstring>

static constexpr int FAN_PIN = 5;

static void set_fan_hw(bool on) {
    // TODO: gpio_set_level(FAN_PIN, on ? 1 : 0);
}

void register_fan_command() {
    Bunny.command("setFanState")
         .description("Turn the fan relay ON or OFF")
         .param("state", STRING, "Target state: ON or OFF")
         .affects("fanState")
         .tag("actuator")
         .example("{\"state\": \"ON\"}")
         .execute([](const bunny::Params& p) {
             const char* state = p.get_string("state");
             set_fan_hw(strcmp(state, "ON") == 0);
         });
}
```

### Evento

**Archivo**: [`main/events/motion_event.cpp`](main/events/motion_event.cpp)

```cpp
#include "motion_event.h"
#include "bunny_sdk.h"

static void blink_indicator_hw() {
    // TODO: gpio_set_level(LED_PIN, 1); vTaskDelay(...); gpio_set_level(LED_PIN, 0);
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
```

### Estado

**Archivo**: [`main/states/fan_state.cpp`](main/states/fan_state.cpp)

```cpp
#include "fan_state.h"
#include "bunny_sdk.h"

static const char* s_fan_state = "OFF";

void register_fan_state() {
    Bunny.state("fanState", STRING)
         .description("Current fan relay state (ON or OFF)")
         .tag("actuator")
         .build(
             []() -> const char* { return s_fan_state; },
             [](const char* v)   { s_fan_state = v;    }
         );
}
```

---

## Patrones y Anti-patrones

### ✅ Correcto

```cpp
// Hardware actions sólo en los hooks
Bunny.command("setLight")
     .description("...")
     .execute([](const Params& p) {
         int brightness = (int)p.get_number("brightness");
         gpio_set_level(LED_PIN, brightness > 0 ? 1 : 0);
     });

// Metadata completa ayuda a backend/LLM
Bunny.sensor("humidity")
     .description("Relative humidity percentage")
     .returns(NUMBER)
     .tag("climate")
     .example("Read: 65.3")
     .build([](){ return read_humidity(); });
```

### ❌ Incorrecto

```cpp
// NO: lógica de negocio en firmware
Bunny.command("activateFan")
     .execute([](const Params& p) {
         if (temperature > 25 && humidity > 60) { // REGLA DE NEGOCIO!
             gpio_set_level(FAN_PIN, 1);
         }
     });

// NO: metadata incompleta
Bunny.state("data", STRING)
     .build([](){ return ""; });  // Qué es este estado? Nadie lo sabe.

// NO: parámetros sin documentación
Bunny.command("control")
     .param("x", STRING)  // ¿Qué es x?
     .execute([](const Params& p) { ... });
```

---

## Construcción y Debugging

### Build

```bash
idf.py build
```

### Monitoreo

```bash
idf.py -p /dev/ttyUSB0 monitor
```

### Limpiar build

```bash
idf.py fullclean
```

---

## Referencias

- [Bunny Framework README](README.md)
- [Configuración del dispositivo](config/device.json)
- [ESP-IDF Official Docs](https://docs.espressif.com/projects/esp-idf/)
