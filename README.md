# Bunny Framework (ESP32)

Framework declarativo para ESP32 orientado a **capacidades de hardware** (sensores, comandos, eventos y estados), donde la **lógica de negocio vive en motor de procesos**.

## 📚 Índice de documentación

- [BCP - Especificación del Bunny Communication Protocol](#bcp---especificación-del-bunny-communication-protocol)
- [COMPARISON.md](doc/COMPARISON.md) — **Comparativa: Bunny Framework vs PlatformIO y Arduino**
- [INSTALL_COMMAND.md](doc/INSTALL_COMMAND.md) — **Comando bunny install** (instalación automática de entorno de desarrollo completo)
- [MONITOR_COMMAND.md](doc/MONITOR_COMMAND.md) — **Comando bunny monitor** (ver output serial del ESP32 en tiempo real)
- [SETUP_ENVIRONMENT.md](doc/SETUP_ENVIRONMENT.md) — **Cómo agregar bunny al PATH** (instalación universal, pasos para cada plataforma, solución de problemas)
- [WINDOWS_SETUP.md](doc/WINDOWS_SETUP.md) — **Guía de instalación y uso en Windows** (requisitos, instalación, comandos CLI, troubleshooting)
- [DEVELOPER_GUIDE.md](doc/DEVELOPER_GUIDE.md) — **Guía técnica completa para desarrolladores** (Sistema de tipos, Metadata, Builders, Registry, ejemplos completos)
- [PROCESS_ENGINE_WEBSOCKET_GUIDE.md](doc/PROCESS_ENGINE_WEBSOCKET_GUIDE.md) — **Cómo debe manejar el motor la sesión WebSocket** (handshake, framing, health-check, reconexión y errores comunes)
- [GLOSARIO.md](doc/GLOSARIO.md) — **Definición de términos clave** (Fluent API, DSL, hooks, runtime, Registry, etc.)
- [FRAMEWORK_VS_LIBRARY.md](doc/FRAMEWORK_VS_LIBRARY.md) — **¿Por qué Bunny es un Framework? Arquitectura y Patrones** (Diferencias framework vs librería, arquitectura hexagonal, patrones de diseño, separación de responsabilidades)
- [GPIO_GUIDE.md](doc/GPIO_GUIDE.md) — **Guía de GPIOs en ESP32 y Ruta de Aprendizaje** (Digital I/O, PWM, ADC, interrupciones asíncronas con debounce y mapa de estudio)
- [BUNNY_PROGRAMMING_PHILOSOPHY.md](doc/BUNNY_PROGRAMMING_PHILOSOPHY.md) — **Cómo programar en Bunny** (filosofía capabilities-first y separación lógica vs firmware)
- [NETWORK_OVERVIEW.md](doc/NETWORK_OVERVIEW.md) — **Cómo funciona la red de Bunny (visión general)** (WiFi, discovery UDP, WebSocket, FLOW y operación)
- [Documentación de tesis](#documentación-de-tesis)
- [Ejemplos de Capacidades](#ejemplos-de-capacidades-módulos-de-ejemplo)
- [Estructura del Proyecto](#estructura-del-proyecto)
- [Archivos Clave del SDK](#archivos-clave-del-sdk)
- [Configuración del Dispositivo](#configuración-del-dispositivo)

## BCP - Especificación del Bunny Communication Protocol

- [BCP_SPECIFICATION.md](doc/BCP_SPECIFICATION.md) — **Especificación oficial de protocolo** (discovery, handshake, heartbeat, ejecución de capacidades, eventos, validación JSON, estado de implementación y brechas pendientes)

## Documentación de tesis

- [DOCUMENTACION_TESIS_PRINCIPAL.md](doc/DOCUMENTACION_TESIS_PRINCIPAL.md) — Versión condensada para el cuerpo principal de tesis (arquitectura, flujo y evidencia de implementación)
- [DOCUMENTACION_TESIS_ANEXOS.md](doc/DOCUMENTACION_TESIS_ANEXOS.md) — Versión extendida para anexos (detalles técnicos, decisiones y fragmentos adicionales de código)
- [DOCUMENTACION_TESIS_DIAPOSITIVA_UNICA.md](doc/DOCUMENTACION_TESIS_DIAPOSITIVA_UNICA.md) — Versión ultra resumida para una sola diapositiva (qué es, para qué sirve y código esencial)

## Objetivo

Bunny separa completamente responsabilidades:

- **ESP32 + Bunny**: define capacidades y ejecuta acciones de hardware.
- **Motor de procesos**: decide reglas, flujos, condiciones y procesos.
- **JSON (DSL)**: contrato de comunicación entre ambos.

> Regla principal: el firmware NO decide lógica de negocio.

## Principios de diseño

- Arquitectura declarativa (capabilities-first).
- Tipado fuerte en parámetros y retornos.
- Metadata semántica para consumo humano y LLM.
- Runtime liviano para edge (ESP32-friendly).
- Separación estricta entre ejecución de hardware y lógica de procesos.

## Tipos de capacidades

1. **Sensor**: produce datos.
2. **Command**: ejecuta acciones de hardware.
3. **Event**: notifica sucesos.
4. **State**: mantiene estado interno tipado.

## Ejemplos de Capacidades (Módulos de ejemplo)

El proyecto incluye 4 módulos de ejemplo que muestran cómo declarar cada tipo de capacidad:

| Tipo | Archivo | Código |
|------|---------|--------|
| **Sensor** | [main/sensors/temperature_sensor.cpp](main/sensors/temperature_sensor.cpp) | Lee temperatura (hardware mock) |
| **Command** | [main/commands/fan_command.cpp](main/commands/fan_command.cpp) | Enciende/apaga ventilador |
| **Event** | [main/events/motion_event.cpp](main/events/motion_event.cpp) | Notifica movimiento detectado |
| **State** | [main/states/fan_state.cpp](main/states/fan_state.cpp) | Mantiene estado del ventilador |

Para crear tus propios módulos, <u>sigue los patrones de estos ejemplos</u> y consulta [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md).

## Estructura del proyecto

```text
bunny_framework/
├── components/
│   └── bunny/
│       ├── include/
│       ├── types/
│       ├── metadata/
│       ├── core/
│       ├── capabilities/
│       ├── builder/
│       ├── registry/
│       ├── protocol/
│       ├── runtime/
│       ├── network/
│       ├── config/
│       └── utils/
├── config/
│   └── device.json
├── main/
│   ├── bunny_framework.cpp
│   ├── sensors/
│   ├── commands/
│   ├── events/
│   └── states/
├── CMakeLists.txt
└── sdkconfig
```

## Propósito de cada carpeta

### `components/bunny/`
Código del framework en sí (SDK + runtime base).

- `include/`: Headers públicos (`bunny.h` como Header Maestro, `bunny_gpio.h` para control de hardware).
- `types/`: sistema de tipos (`NUMBER`, `STRING`, `BOOLEAN`, `OBJECT`, `ARRAY`).
- `metadata/`: contrato semántico de capacidades (`description`, `params`, `returns`, `tags`, `affects`, `example`).
- `core/`: contratos base (`ICapability`, kind de capacidad).
- `capabilities/`: implementaciones de `SensorCapability`, `CommandCapability`, `EventCapability`, `StateCapability`.
- `builder/`: fluent API para declarar capacidades de forma ergonómica.
- `registry/`: registro central de capacidades y serialización a JSON.
- `protocol/`: parseo/serialización de mensajes JSON (motor de procesos <-> dispositivo).
- `runtime/`: ciclo de ejecución y despacho de hooks (sin lógica de negocio).
- `network/`: transporte (discovery UDP y conexión con motor de procesos/webhook).
- `config/`: carga y acceso de configuración del dispositivo (`device.json`).
- `utils/`: helpers ligeros compartidos (ej. construcción JSON sin dependencias).

### `config/`
Configuración declarativa editable por el usuario del dispositivo.

- `device.json`: identidad del dispositivo, discovery UDP y webhook.

### `main/`
Punto de entrada del firmware y módulos de capacidades auto-registrados.

- `bunny_framework.cpp`: punto de entrada minimalista (`app_main`).
- `sensors/`: declaraciones de sensores (solo `.cpp`, auto-registrados).
- `commands/`: declaraciones de comandos (solo `.cpp`, auto-registrados).
- `events/`: declaraciones de eventos (solo `.cpp`, auto-registrados).
- `states/`: declaraciones de estados (solo `.cpp`, auto-registrados).

## Archivos clave del SDK

Para trabajar con Bunny, estos son los archivos más importantes:

### API Pública (incluir en tus módulos)

- [components/bunny/include/bunny.h](components/bunny/include/bunny.h) — **Header Maestro Unificado** (exporta SDK C++, Fluent API, macros `BUNNY_*` y `bunny::gpio`)
- [components/bunny/include/bunny_gpio.h](components/bunny/include/bunny_gpio.h) — **Control simplificado de hardware** (`pin_mode`, `digital_write`, `analog_read`, `analog_write`, `attach_interrupt`)
- [components/bunny/bunny_sdk.h](components/bunny/bunny_sdk.h) — **Definición de clases SDK y macros de auto-registro**


### Core

- [components/bunny/types/bunny_types.h](components/bunny/types/bunny_types.h) — Sistema de tipos (NUMBER, STRING, BOOLEAN, OBJECT, ARRAY)
- [components/bunny/metadata/metadata.h](components/bunny/metadata/metadata.h) — Estructura de metadata semántica
- [components/bunny/core/capability.h](components/bunny/core/capability.h) — Interfaz base ICapability

### Builders (Fluent API)

- [components/bunny/builder/sensor_builder.h](components/bunny/builder/sensor_builder.h)
- [components/bunny/builder/command_builder.h](components/bunny/builder/command_builder.h)
- [components/bunny/builder/event_builder.h](components/bunny/builder/event_builder.h)
- [components/bunny/builder/state_builder.h](components/bunny/builder/state_builder.h)

### Capacidades

- [components/bunny/capabilities/sensor_capability.h](components/bunny/capabilities/sensor_capability.h)
- [components/bunny/capabilities/command_capability.h](components/bunny/capabilities/command_capability.h) — Incluye `Params` container
- [components/bunny/capabilities/event_capability.h](components/bunny/capabilities/event_capability.h)
- [components/bunny/capabilities/state_capability.h](components/bunny/capabilities/state_capability.h)

### Infraestructura

- [components/bunny/registry/registry.h](components/bunny/registry/registry.h) — Registro central y serialización JSON
- [components/bunny/config/config.h](components/bunny/config/config.h) — Carga de configuración del dispositivo
- [components/bunny/network/discovery.h](components/bunny/network/discovery.h) — Discovery UDP
- [NETWORK_OVERVIEW.md](NETWORK_OVERVIEW.md) — Visión general del flujo de red (WiFi + UDP + WebSocket)
- [components/bunny/utils/json_builder.h](components/bunny/utils/json_builder.h) — Helper JSON sin dependencias

## Flujo de arranque esperado

1. `app_main` llama `bunny_begin()`.
2. Se carga configuracion del dispositivo.
3. Se inicializa discovery de red.
4. Se registran capacidades declaradas en `main/`.
5. `bunny_load_modules()` deja listo el runtime.
6. `bunny_loop()` mantiene el ciclo de comunicacion/ejecucion.

## Metadata obligatoria (muy importante)

Cada capacidad debe definir, idealmente siempre:

- `description`
- `params` (cuando aplique)
- `returns`
- `tags`
- `affects` (opcional)
- `example` (opcional, recomendado)

Esto permite:

- Menos errores de uso desde motor de procesos/LLM.
- Validacion estructurada de invocaciones.
- Generacion de procesos y tooling automatico.

## Modelo Híbrido: Rápido vs Fluent API

Bunny Framework ofrece **dos formas** de declarar capacidades según la necesidad:

### 🚀 1. Modo Rápido Directo (Funciones C++ Tipadas, Cero Lambdas)
Ideal para el 90% de los casos donde solo quieres ejecutar una acción o leer un valor en pocas líneas:

```cpp
#include "bunny.h"

// Sensor rápido en 3 líneas:
BUNNY_READ(temperature) {
    return analog_read_mapped(34, -10.0, 60.0);
}

// Comando rápido con parámetro tipado directo:
BUNNY_ACTION(setFanState, const char* state) {
    pin_mode(15, OUTPUT, "fan_relay");
    digital_write(15, strcmp(state, "ON") == 0 ? HIGH : LOW);
}

// Comando con valor numérico o porcentaje:
BUNNY_ACTION(setDimmer, int level) {
    analog_write(18, level);
}

// Evento rápido de 1 línea:
BUNNY_TRIGGER(motionDetected);
```

### 🌟 2. Modo Fluent (Metadatos Ricos y Documentación Semántica)
Ideal cuando necesitas enriquecer el contrato con descripciones, tags semánticos para LLMs, validaciones y ejemplos JSON:

```cpp
#include "bunny.h"

BUNNY_COMMAND(drawText) {
    Bunny.command("drawText")
         .description("Dibuja texto en coordenadas específicas")
         .param("text", STRING, "Texto a mostrar")
         .param("x", NUMBER, "Coordenada horizontal X")
         .param("y", NUMBER, "Coordenada vertical Y")
         .tag("display")
         .tag("ui")
         .example("{\"text\": \"Hola Mundo\", \"x\": 10, \"y\": 20}")
         .execute([](const bunny::Params& p) {
             const char* text = p.get_string("text");
             int x = (int)p.get_number("x");
             int y = (int)p.get_number("y");
             // Acción de hardware...
         });
}
```


## Manejo Ultra-Simple de Hardware (GPIOs)

Bunny Framework incluye una capa de abstracción de hardware familiar y potente:

| Capacidad | Función Bunny | Descripción |
| :--- | :--- | :--- |
| **Modo de Pin** | `pin_mode(pin, OUTPUT)` | Configura `INPUT`, `OUTPUT`, `PULLUP`, `PULLDOWN`. |
| **Escritura Digital** | `digital_write(pin, HIGH)` | Escribe `HIGH` (3.3V) o `LOW` (0V). |
| **Lectura Digital** | `digital_read(pin)` | Lee estado del pin (`1` o `0`). |
| **Salida PWM** | `analog_write(pin, 128)`<br>`pwm_write_percent(pin, 75.0)` | PWM en escala 0-255 o en % (auto-gestión de canales LEDC). |
| **Lectura ADC** | `analog_read(pin)`<br>`analog_read_voltage(pin)`<br>`analog_read_mapped(pin, min, max)` | Auto-init ADC1, promediado de ruido y conversión a voltios/escala. |
| **Interrupciones** | `attach_interrupt(pin, FALLING, cb, 50)` | Interrupción segura asíncrona con filtro anti-rebote (*debounce*). |

Ver guía completa en [doc/GPIO_GUIDE.md](doc/GPIO_GUIDE.md).


## Reglas importantes a tener en cuenta

- No meter reglas de negocio en ESP32 (`if/else` de negocio, workflows, condiciones de proceso).
- No evaluar strings como codigo.
- No acoplar decisiones del motor de procesos en firmware.
- Si una capacidad no esta bien documentada en metadata, su uso por motor de procesos/LLM sera fragil.

## Build (ESP-IDF)

```bash
idf.py build
```

Para flashear:

```bash
idf.py -p <PORT> flash monitor
```

## Comandos rápidos (Bunny CLI)

El framework incluye comandos `bunny` para automatizar tareas comunes.

### Instalación Rápida (Recomendado)

Para usuarios nuevos, usa el comando `bunny install` para instalar todo automáticamente:

```bash
cd C:\proyects\bunny_framework    # Windows
cd /ruta/a/bunny_framework        # Linux/macOS

bunny install
```

Esto instala:
- ✅ Python (si es necesario)
- ✅ ESP-IDF (descarga e instala automáticamente)
- ✅ Variables de entorno (bunny disponible en cualquier ubicación)

Ver [doc/INSTALL_COMMAND.md](doc/INSTALL_COMMAND.md) para más detalles.

### Uso del comando `bunny`

#### Instalación de entorno (1 vez)
```bash
bunny install
bunny i           # Alias corto
```

#### Flasheo de ESP32
```bash
bunny flash       # Flashear
bunny flash clean # Limpiar y flashear
```

#### Monitor Serial
```bash
bunny monitor     # Ver output del ESP32
bunny m           # Alias corto
```

#### En Linux/macOS:
```bash
./bunny flash
./bunny flash clean
./bunny monitor
BUNNY_PORT=/dev/ttyUSB1 ./bunny monitor
```

#### En Windows (CMD):
```cmd
bunny flash
bunny flash clean
bunny monitor
set BUNNY_PORT=COM4 && bunny monitor
```

#### En Windows (PowerShell):
```powershell
.\bunny.ps1 flash
.\bunny.ps1 flash clean
.\bunny.ps1 monitor
$env:BUNNY_PORT='COM4'; .\bunny.ps1 monitor
```

#### O después de ejecutar setup (desde cualquier ubicación):
```powershell
bunny flash
```

**Nota:** El comando detecta automáticamente el puerto serial. Si tienes múltiples dispositivos, usa `BUNNY_PORT` para especificar cuál usar.

## Configuración del dispositivo

Ver [config/device.json](config/device.json) para configurar:

- **Identidad del dispositivo**: `id`, `name`, `description`, `version`
- **Discovery UDP**: puerto y intervalo de broadcast
- **Webhook**: puerto y path para conexión con motor de procesos

## Estado actual

- SDK declarativo base implementado.
- Registro y serializacion de capacidades listos.
- Ejemplos de `sensor`, `command`, `event`, `state` en `main/`.
- Discovery UDP y protocolo/runtime aun con partes marcadas como `TODO` para completar transporte y despacho final.
