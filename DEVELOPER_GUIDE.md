# Bunny SDK — Guía de Desarrollo (COMPLETA)

Documento técnico para desarrolladores que extienden o mantienen el framework Bunny.

Última actualización: 21 de abril de 2026.

**← [Volver al README](README.md)**

Términos clave: [GLOSARIO.md](GLOSARIO.md)

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
10. [Patrones y Anti-patrones](#patrones-y-anti-patrones)
11. [Glosario de términos](#glosario-de-términos)

---

## Sistema de Tipos

**Archivo**: [`components/bunny/types/bunny_types.h`](components/bunny/types/bunny_types.h)

### ¿Por qué un sistema de tipos?

Bunny necesita saber qué tipo de datos espera y devuelve cada capacidad para:

1. **Validación en el motor de procesos**: Antes de enviar un comando, el motor de procesos valida que los parámetros coinciden con los tipos esperados.
2. **Documentación automática**: El LLM o herramientas de generación saben exactamente qué esperar.
3. **Serialización segura**: JSON permite cualquier tipo, pero nosotros queremos asegurarnos de conversiones correctas.
4. **Prevención de errores**: Si un sensor devuelve un NUMBER y el motor de procesos espera STRING, lo sabremos inmediatamente.

### Tipos definidos

Bunny define **5 tipos primitivos** que cubren el 99% de casos de uso en IoT:

```cpp
enum class Type : uint8_t {
    NUMBER  = 0,    // Números flotantes y enteros (double en C++)
    STRING  = 1,    // Cadenas de texto (const char* en C++)
    BOOLEAN = 2,    // Valores verdadero/falso
    OBJECT  = 3,    // Objetos JSON anidados (tratados como strings actualmente)
    ARRAY   = 4,    // Arreglos JSON (tratados como strings actualmente)
    VOID    = 5,    // Sin retorno (ej: comandos que no devuelven valor)
};
```

**Nota**: OBJECT y ARRAY se serializan como strings JSON en la metadata, pero el motor de procesos puede parsearlos según necesite.

### Alias globales para la Fluent API

Para que el código sea más legible, Bunny define alias globales (sin namespace) en el mismo archivo:

```cpp
static constexpr bunny::Type NUMBER  = bunny::Type::NUMBER;
static constexpr bunny::Type STRING  = bunny::Type::STRING;
static constexpr bunny::Type BOOLEAN = bunny::Type::BOOLEAN;
static constexpr bunny::Type OBJECT  = bunny::Type::OBJECT;
static constexpr bunny::Type ARRAY   = bunny::Type::ARRAY;
```

Esto permite escribir:

```cpp
// En lugar de:
Bunny.sensor("temp").returns(bunny::Type::NUMBER);

// Simplemente:
Bunny.sensor("temp").returns(NUMBER);
```

### Helper: type_name()

La función `type_name(Type t)` convierte el enum a string:

```cpp
inline const char* type_name(Type t) {
    switch (t) {
        case Type::NUMBER:  return "number";
        case Type::STRING:  return "string";
        case Type::BOOLEAN: return "boolean";
        case Type::OBJECT:  return "object";
        case Type::ARRAY:   return "array";
        case Type::VOID:    return "void";
        default:            return "unknown";
    }
}
```

Se usa internamente para serializar a JSON (ej: `"returns":"number"`).

### Ejemplo de uso

```cpp
// Sensor que devuelve un número (temperatura)
Bunny.sensor("temperature")
     .description("Room temperature in Celsius")
     .returns(NUMBER)  // <-- Tipo de retorno
     .build([]() -> double {
         return 23.5;
     });

// Comando que recibe un parámetro string
Bunny.command("setMode")
     .param("mode", STRING, "auto, manual, or standby")
     .execute([](const Params& p) {
         const char* mode = p.get_string("mode");
     });

// Comando sin retorno
Bunny.command("reset")
     .description("Soft reset device")
     .execute([](const Params& p) { /* no retorna nada */ });
```

---

## Metadata Semántica

**Archivo**: [`components/bunny/metadata/metadata.h`](components/bunny/metadata/metadata.h)

### ¿Qué es la metadata y por qué es crítica?

La **metadata es el contrato semántico** que describe qué hace cada capacidad, qué parámetros acepta, qué retorna, y cómo se relaciona con otras capacidades.

Es crítica porque:

1. **El motor de procesos no tiene el código**: El motor de procesos solo conoce lo que tú declaras en metadata.
2. **LLM puede razonar sobre capacidades**: Si documentas bien, un modelo de lenguaje puede usar las capacidades automáticamente.
3. **Validación automática**: Las herramientas pueden validar invocaciones contra la metadata.
4. **Generación de UI**: El motor de procesos puede generar interfaces automáticamente.
5. **Debugging**: Cuando algo falla, la metadata ayuda a entender qué salió mal.

### Estructura detallada

```cpp
struct ParamDef {
    const char* name;          // Nombre del parámetro: "state"
    Type        type;          // Tipo: STRING, NUMBER, etc.
    const char* description;   // Documentación: "ON or OFF"
    bool        required;      // ¿Es obligatorio? true = sí, false = opcional
};

struct Metadata {
    // Descripción de qué hace esta capacidad
    const char* description;
    // "Control the fan speed from 0 to 100%"

    // Tipo de datos que retorna (para sensores y comandos)
    Type        returns_type;
    // Type::NUMBER para sensores, Type::VOID para comandos sin retorno

    // Parámetros de entrada (si aplica)
    ParamDef    params[MAX_PARAMS];
    // Array de definiciones de parámetros
    size_t      param_count;
    // Cuántos parámetros realmente hay (0-8)

    // Etiquetas semánticas
    const char* tags[MAX_TAGS];
    // ["actuator", "climate", "fan"] - ayudan a categorizar
    size_t      tag_count;

    // Qué otras capacidades se ven afectadas
    const char* affects[MAX_AFFECTS];
    // ["fanSpeed", "fanState"] - cambiar esto afecta estas otras cosas
    size_t      affects_count;

    // Ejemplo práctico
    const char* example;
    // "{\"speed\": 75, \"rampTime\": 500}" - ayuda a entender uso
};
```

### Límites (todos configurables en el .h)

```cpp
static constexpr size_t MAX_PARAMS  = 8;      // Máximo 8 parámetros por capacidad
static constexpr size_t MAX_TAGS    = 8;      // Máximo 8 tags
static constexpr size_t MAX_AFFECTS = 8;      // Máximo 8 afectaciones
```

Estos límites existen para:
- **ESP32 es memoria limitada**: No queremos arrays dinámicos.
- **Simplicidad**: Si necesitas más, probablemente necesitas refactorizar.
- **Previsibilidad**: Tamaños conocidos en compile-time.

### Cómo rellenar metadata

#### Opción 1: Con Builders (RECOMENDADO)

Bunny maneja todo automáticamente:

```cpp
Bunny.sensor("humidity")
     .description("Relative humidity percentage")
     .returns(NUMBER)
     .tag("climate")
     .tag("environment")
     .affects("climate_control")
     .example("Read: 65.3%")
     .build([]() -> double { return 65.3; });
```

#### Opción 2: Manual (si lo necesitas)

```cpp
// Crear metadata manualmente
Metadata meta{};
meta.description = "My custom sensor";
meta.returns_type = Type::NUMBER;

// Agregar parámetros
meta.params[0] = {"threshold", Type::NUMBER, "Alert threshold", true};
meta.param_count = 1;

// Agregar tags
meta.tags[0] = "custom";
meta.tags[1] = "monitoring";
meta.tag_count = 2;

// Agregar afectaciones
meta.affects[0] = "alertState";
meta.affects_count = 1;

// Ejemplo
meta.example = "threshold: 25";

// Luego crear la capacidad
auto sensor = SensorCapability("my_sensor", meta, []() { return 10.0; });
Registry::instance().register_capability(&sensor);
```

---

## Contratos Base de Capacidades

**Archivo**: [`components/bunny/core/capability.h`](components/bunny/core/capability.h)

### ¿Qué es un contrato base?

Un contrato base (interfaz) define la forma en que **todas** las capacidades se comportan, sin importar su tipo específico.

Piénsalo como un "acuerdo":
- Toda capacidad tiene un nombre.
- Toda capacidad tiene metadata.
- Toda capacidad sabe qué tipo es (sensor, command, etc.).
- Toda capacidad puede serializarse a JSON.

### ICapability - La interfaz madre

```cpp
class ICapability {
public:
    virtual ~ICapability() = default;

    // ¿Qué tipo de capacidad soy? (SENSOR, COMMAND, EVENT, STATE)
    virtual CapabilityKind  kind()     const = 0;

    // ¿Cómo me llamo?
    virtual const char*     name()     const = 0;

    // ¿Cuál es mi metadata?
    virtual const Metadata& metadata() const = 0;

    // ¿Cómo te digo quién soy en JSON?
    virtual size_t          serialize(char* buf, size_t len) const = 0;
};
```

Cada método virtual es puro (`= 0`), significando que **cada subclase DEBE implementarla**.

### CapabilityKind - Tipos de capacidades

```cpp
enum class CapabilityKind : uint8_t {
    SENSOR  = 0,    // Produce datos (lectura)
    COMMAND = 1,    // Ejecuta acciones (escritura)
    EVENT   = 2,    // Notifica sucesos (push)
    STATE   = 3,    // Almacena estado (read/write)
};
```

---

## Fluent Builder API

**Archivos**: [`components/bunny/builder/`](components/bunny/builder/)

### ¿Qué es una Fluent API?

Es un **patrón de diseño** que permite encadenar llamadas de métodos para construir objetos de forma legible.

```cpp
// Con fluent API (muy legible):
Bunny.sensor("temperature")
     .description("Room temp")
     .returns(NUMBER)
     .tag("environment")
     .build([]() { return 25.0; });
```

### SensorBuilder — Declara sensores

Los sensores **producen datos**. Leen hardware y retornan un número.

```cpp
Bunny.sensor("battery_voltage")
     .description("Current battery voltage in volts")
     .returns(NUMBER)
     .tag("power")
     .example("Read: 3.7V")
     .build([]() -> double {
         int raw = adc_read(ADC_PIN_BATTERY);
         return (raw / 4095.0) * 4.2;
     });
```

### CommandBuilder — Declara comandos

Los comandos **ejecutan acciones**. Reciben parámetros del motor de procesos.

```cpp
Bunny.command("setFanSpeed")
     .description("Set fan speed 0-100%")
     .param("speed", NUMBER, "Target speed percentage (0-100)")
     .param("rampTime", NUMBER, "Ramp time in ms (optional)", false)
     .affects("fanSpeed")
     .tag("actuator")
     .example("{\"speed\": 75, \"rampTime\": 2000}")
     .execute([](const bunny::Params& p) {
         int speed = (int)p.get_number("speed");
         pwm_set(FAN_PIN, speed);
     });
```

### EventBuilder — Declara eventos

Los eventos **notifican al motor de procesos**. Se disparan cuando algo sucede.

```cpp
Bunny.event("motion_detected")
     .description("PIR sensor detected movement")
     .tag("sensor")
     .tag("security")
     .build([]() {
         gpio_set_level(LED_PIN, 1);  // Feedback local
         vTaskDelay(100 / portTICK_PERIOD_MS);
         gpio_set_level(LED_PIN, 0);
     });
```

### StateBuilder — Declara estado

El estado **almacena valores**. Motor de procesos puede leer Y escribir.

```cpp
static const char* s_fan_state = "OFF";

Bunny.state("fanState", STRING)
     .description("Current fan state: ON or OFF")
     .tag("actuator")
     .build(
         []() -> const char* { return s_fan_state; },
         [](const char* v)   { s_fan_state = v;    }
     );
```

---

## Registro Central (Registry)

**Archivo**: [`components/bunny/registry/registry.h`](components/bunny/registry/registry.h)

### ¿Por qué un Registry?

El Registry es un **índice centralizado** que permite:
- Almacenar todas las capacidades (sensores, comandos, eventos, states).
- Buscar una capacidad por nombre y tipo.
- Serializar todo a JSON para enviar al motor de procesos.
- Despachar comandos a sus hooks de ejecución.

Cuando el motor de procesos envía `setFanSpeed`, el Registry busca esa capacidad y la ejecuta.

### Singleton Pattern

```cpp
static Registry& instance() {
    static Registry s_instance;
    return s_instance;
}
```

Solo hay **una** instancia en toda la aplicación:

```cpp
Registry::instance().register_capability(cap);
Registry::instance().find("temperature", CapabilityKind::SENSOR);
```

---

## Crear un Módulo de Capacidades

Estructura recomendada:

```text
main/
├── sensors/
│   ├── temperature_sensor.h
│   └── temperature_sensor.cpp
├── commands/
├── events/
└── states/
```

### Header (.h)

```cpp
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void register_temperature_sensor();

#ifdef __cplusplus
}
#endif
```

### Implementación (.cpp)

```cpp
#include "temperature_sensor.h"
#include "bunny_sdk.h"

static double read_temperature_hw() {
    // TODO: hardware read aquí
    return 23.5;
}

void register_temperature_sensor() {
    Bunny.sensor("temperature")
         .description("Ambient temperature in Celsius")
         .returns(NUMBER)
         .tag("environment")
         .example("Read: 23.5")
         .build([]() -> double {
             return read_temperature_hw();
         });
}
```

### Integración en main

```c
#include "bunny.h"
#include "sensors/temperature_sensor.h"

void app_main(void) {
    bunny_begin();
    register_temperature_sensor();  // <-- Registra aquí
    bunny_load_modules();
    bunny_loop();
}
```

---

## Serialización JSON

Bunny **no usa bibliotecas externas** para JSON. Helper simple en [`components/bunny/utils/json_builder.h`](components/bunny/utils/json_builder.h).

Cada capacidad implementa `serialize()` que genera su JSON:

```json
{
  "name": "temperature",
  "kind": "sensor",
  "description": "Ambient temperature in Celsius",
  "returns": "number",
  "tags": ["environment"],
  "example": "Read: 23.5"
}
```

El Registry combina todos en un manifest:

```json
{
  "sensors": [...],
  "commands": [...],
  "events": [...],
  "states": [...]
}
```

---

## Protocolo de Comunicación

Mensajes JSON esperados entre motor de procesos y ESP32:

### Comando

**Motor de procesos → ESP32**:
```json
{"id": "req-001", "type": "command", "command": "setFanSpeed", "params": {"speed": 75}}
```

**ESP32 → Motor de procesos**:
```json
{"id": "req-001", "status": "ok", "result": null, "timestamp": 1234567890}
```

### Sensor

**Motor de procesos → ESP32**:
```json
{"id": "req-002", "type": "sensor", "sensor": "temperature"}
```

**ESP32 → Motor de procesos**:
```json
{"id": "req-002", "status": "ok", "value": 23.5, "timestamp": 1234567890}
```

### Evento (push)

**ESP32 → Motor de procesos**:
```json
{"event": "motion_detected", "timestamp": 1234567890, "data": {}}
```

---

## Ejemplos Completos

Ver archivos de ejemplo en [`main/`](main/):
- [Sensor](main/sensors/temperature_sensor.cpp)
- [Comando](main/commands/fan_command.cpp)
- [Evento](main/events/motion_event.cpp)
- [Estado](main/states/fan_state.cpp)

---

## Patrones y Anti-patrones

### ✅ CORRECTO

```cpp
// Buena documentación
Bunny.command("setFanSpeed")
     .description("Set fan speed 0-100%, supports ramping")
     .param("speed", NUMBER, "Target speed (0-100)")
     .param("rampTime", NUMBER, "Ramp time ms (optional)", false)
     .affects("fanSpeed")
     .execute([](const Params& p) {
         // Solo hardware
         int speed = (int)p.get_number("speed");
         pwm_set(FAN_PIN, speed);
     });
```

### ❌ INCORRECTO

```cpp
// NUNCA: lógica de negocio
Bunny.command("checkTemperature")
     .execute([](const Params& p) {
         if (read_temp() > 30) {  // ← REGLA DE NEGOCIO!
             activate_fan();
         }
     });

// Correcto: separar
Bunny.sensor("temperature").build([](){ return read_temp(); });
Bunny.command("activateFan").execute([](const Params& p){ activate_fan(); });
```

---

## Build y Debug

```bash
# Compilar
idf.py build

# Monitorear
idf.py -p /dev/ttyUSB0 monitor

# Limpiar
idf.py fullclean
```

---

## Referencias

- [Bunny README](README.md)
- [ESP-IDF Docs](https://docs.espressif.com/projects/esp-idf/)

---

## Glosario de términos

Para definiciones rápidas de términos como Fluent API, DSL, hooks, Registry, runtime y otros conceptos usados en esta guía, consulta [GLOSARIO.md](GLOSARIO.md).
