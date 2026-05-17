# Bunny Framework — Guía de `#include`

Qué incluir en cada tipo de archivo, por qué, y qué pasa si falta o sobra.

Última actualización: 22 de abril de 2026.

**← [Volver al README](README.md)**

---

## Índice

1. [Regla de oro](#regla-de-oro)
2. [Los dos headers públicos del framework](#los-dos-headers-públicos-del-framework)
3. [Archivos en `main/` — módulos de capacidades](#archivos-en-main--módulos-de-capacidades)
4. [El punto de entrada `bunny_framework.c`](#el-punto-de-entrada-bunny_frameworkc)
5. [Headers propios de cada módulo `.h`](#headers-propios-de-cada-módulo-h)
6. [Qué NO incluir en un módulo de capacidades](#qué-no-incluir-en-un-módulo-de-capacidades)
7. [Por qué cada módulo incluye su propio `.h`](#por-qué-cada-módulo-incluye-su-propio-h)
8. [Árbol de includes del framework (referencia)](#árbol-de-includes-del-framework-referencia)
9. [Tabla de referencia rápida](#tabla-de-referencia-rápida)

---

## Regla de oro

> **Un módulo de capacidades en `main/` solo necesita dos includes:**  
> 1. Su propio header (`"fan_command.h"`)  
> 2. El SDK del framework (`"bunny_sdk.h"`)

Todo lo demás (tipos, builders, Params, Registry) llega transitivamente desde `bunny_sdk.h`. No es necesario ni recomendable incluirlos por separado.

---

## Los dos headers públicos del framework

El framework ofrece exactamente **dos puntos de entrada** según el tipo de archivo:

### 1. `bunny_sdk.h` — para módulos de capacidades (`.cpp`)

```cpp
#include "bunny_sdk.h"
```

**Qué incluye internamente (transitivo):**

| Header transitivo | Qué aporta |
|---|---|
| `types/bunny_types.h` | `NUMBER`, `STRING`, `BOOLEAN`, `OBJECT`, `ARRAY`, `VOID` |
| `metadata/metadata.h` | `Metadata`, `ParamDef`, `Affects` — el contrato semántico |
| `builder/command_builder.h` | `.command("name").description(...).execute(...)` |
| `builder/sensor_builder.h` | `.sensor("name").returns(NUMBER).build(...)` |
| `builder/event_builder.h` | `.event("name").description(...).build(...)` |
| `builder/state_builder.h` | `.state("name", STRING).build(...)` |
| `capabilities/command_capability.h` | Clase `CommandCapability` + contenedor `Params` |
| `capabilities/sensor_capability.h` | Clase `SensorCapability` |
| `capabilities/event_capability.h` | Clase `EventCapability` |
| `capabilities/state_capability.h` | Clase `StateCapability` |
| `registry/registry.h` | `Registry::instance()` — acceso al registro central |

**Por qué no incluir cada uno por separado:** el orden de inclusión importa (algunos headers dependen de otros), y `bunny_sdk.h` ya gestiona ese orden correctamente. Incluirlos individualmente rompe esa garantía.

---

### 2. `bunny.h` — solo para el punto de entrada C (`bunny_framework.c`)

```c
#include "bunny.h"
```

**Qué expone:**

```c
void bunny_begin(void);
void bunny_load_modules(void);
void bunny_loop(void);
```

**Por qué es distinto de `bunny_sdk.h`:**  
`bunny_framework.c` es un archivo `.c` puro. No puede incluir C++ directamente. `bunny.h` es el wrapper con `extern "C"` que expone solo las tres funciones de ciclo de vida necesarias para `app_main`.

> Si lo que haces es registrar capacidades, **nunca uses `bunny.h` en un módulo C++**. Ese archivo no tiene builders ni `Bunny.command(...)`.

---

## Archivos en `main/` — módulos de capacidades

Cada módulo de capacidad tiene dos archivos: `.h` (declaración pública) y `.cpp` (implementación).

### `.cpp` — implementación del módulo

El patrón completo es siempre este:

```cpp
#include "fan_command.h"        // ← su propio header primero
#include "bunny_sdk.h"          // ← SDK del framework
#include <cstring>              // ← librería estándar si se necesita
```

**Por qué primero el propio header:**  
Incluirlo primero obliga al compilador a verificar que el header es autosuficiente (no depende silenciosamente de algo que ya estaba en memoria). Si hay un error ahí, lo detectas inmediatamente en ese módulo, no en quien lo importa.

**Ejemplo real:** [main/commands/fan_command.cpp](../main/commands/fan_command.cpp)

```cpp
#include "fan_command.h"
#include "bunny_sdk.h"
#include <cstring>

void register_fan_command() {
    Bunny.command("setFanState")
         .description("Turn the fan relay ON or OFF")
         .param("state", STRING, "Target state: ON or OFF")
         .affects("fanState")
         .tag("actuator")
         .example("{\"state\": \"ON\"}")
         .execute([](const bunny::Params& p) {
             const char* state = p.get_string("state");
             // hardware aquí, sin lógica de negocio
         });
}
```

**Nota sobre `<cstring>`:** Se necesita si usas `strcmp`, `strlen`, `strcpy`, etc. dentro del lambda o del módulo. `bunny_sdk.h` no lo incluye, ya que no es parte del SDK.

---

### `.h` — declaración pública del módulo

El patrón es minimalista:

```cpp
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void register_fan_command();

#ifdef __cplusplus
}
#endif
```

**Por qué `#pragma once`:** evita inclusión múltiple si varios archivos incluyen este header.

**Por qué `extern "C"`:** la función `register_*` es llamada desde `bunny_framework.c`, que es un archivo `.c` puro. Sin `extern "C"`, el nombre de la función queda decorado por el compilador C++ y el linker falla.

**Qué NO incluir en el `.h` del módulo:**  
- `bunny_sdk.h` (solo va en el `.cpp`)
- Headers de ESP-IDF como `driver/gpio.h`
- Tipos internos del framework como `Params`, `Metadata`

El header del módulo solo declara la firma de `register_*`. Nada más. Cualquier detail de implementación pertenece al `.cpp`.

**Ejemplo real:** [main/events/motion_event.h](../main/events/motion_event.h)

---

## El punto de entrada `bunny_framework.c`

```c
#include "bunny.h"                           // ← lifecycle C-compatible
#include "sensors/temperature_sensor.h"      // ← headers de módulos propios
#include "commands/fan_command.h"
#include "events/motion_event.h"
#include "states/fan_state.h"
```

**Por qué incluye los headers de módulos en vez de los `.cpp`:**  
El compilador nunca incluye `.cpp` directamente. Cada `.cpp` se compila como unidad de traducción independiente. El header declara la función `register_*`, el linker une todo después.

**Ejemplo real:** [main/bunny_framework.c](../main/bunny_framework.c)

---

## Headers propios de cada módulo `.h`

Cada módulo expone exactamente una función pública en su `.h`:

| Módulo | Función expuesta |
|--------|-----------------|
| [main/commands/fan_command.h](../main/commands/fan_command.h) | `void register_fan_command()` |
| [main/events/motion_event.h](../main/events/motion_event.h) | `void register_motion_event()` |
| [main/sensors/temperature_sensor.h](../main/sensors/temperature_sensor.h) | `void register_temperature_sensor()` |
| [main/states/fan_state.h](../main/states/fan_state.h) | `void register_fan_state()` |

Esa función es el único contrato entre el módulo y `bunny_framework.c`. Todo lo que hace internamente (builders, lambdas, hardware) es detalleprivado del `.cpp`.

---

## Qué NO incluir en un módulo de capacidades

| Header | Por qué no incluirlo directamente |
|--------|-----------------------------------|
| `builder/command_builder.h` | Ya lo trae `bunny_sdk.h` con el orden correcto |
| `capabilities/command_capability.h` | Ídem vía `bunny_sdk.h` |
| `registry/registry.h` | Solo necesario si accedes al Registry directamente (caso avanzado) |
| `metadata/metadata.h` | El builder lo gestiona internamente |
| `core/capability.h` | Es la interfaz base interna del framework, no para módulos de usuario |
| `esp_log.h`, `driver/gpio.h` | Los includes de ESP-IDF van en el `.cpp`, nunca en el `.h` del módulo |

---

## Por qué cada módulo incluye su propio `.h`

Esta es la regla que aplica a cualquier proyecto C/C++, no solo a Bunny:

1. **Auto-verificación:** Si el `.h` del módulo tiene un error (tipo incorrecto, falta un forward declaration), al incluirlo primero en su propio `.cpp` el error aparece en ese archivo, que es exactamente donde está el problema.
2. **Desacoplamiento:** Otros módulos o `bunny_framework.c` solo necesitan el `.h` — no ven el código de implementación.
3. **Compilación separada:** El compilador compila cada `.cpp` de forma independiente. Para saber qué funciones existen en otros módulos, necesita sus declaraciones en headers.
4. **Evitar ODR violations:** Sin `#pragma once` o include guards, incluir un header dos veces puede romper el build. Bunny usa `#pragma once` en todos sus headers.

---

## Árbol de includes del framework (referencia)

```
bunny_framework.c
└── bunny.h                        ← lifecycle C (begin/load/loop)

fan_command.cpp
├── fan_command.h                  ← declaración propia
└── bunny_sdk.h                    ← SDK completo
    ├── types/bunny_types.h        →  NUMBER, STRING, BOOLEAN...
    ├── metadata/metadata.h        →  Metadata, ParamDef
    ├── builder/command_builder.h  →  .command(...).execute(...)
    ├── builder/sensor_builder.h   →  .sensor(...).build(...)
    ├── builder/event_builder.h    →  .event(...).build(...)
    ├── builder/state_builder.h    →  .state(...).build(...)
    ├── capabilities/command_capability.h  → Params, CommandCapability
    ├── capabilities/sensor_capability.h   → SensorCapability
    ├── capabilities/event_capability.h    → EventCapability
    ├── capabilities/state_capability.h    → StateCapability
    └── registry/registry.h        →  Registry::instance()
```

---

## Tabla de referencia rápida

| Tipo de archivo | Include obligatorio | Include opcional |
|----------------|---------------------|------------------|
| `bunny_framework.c` (punto de entrada C) | `"bunny.h"` + headers de módulos | — |
| `my_command.cpp` (módulo C++) | `"my_command.h"`, `"bunny_sdk.h"` | `<cstring>`, `<cstdio>`, `driver/gpio.h` |
| `my_event.cpp` | `"my_event.h"`, `"bunny_sdk.h"` | headers de FreeRTOS si usas tareas |
| `my_sensor.cpp` | `"my_sensor.h"`, `"bunny_sdk.h"` | headers de ADC/I2C si lees hardware |
| `my_state.cpp` | `"my_state.h"`, `"bunny_sdk.h"` | — |
| `my_command.h` (header del módulo) | `#pragma once` + `extern "C"` | — |
